#include <tuinator/core/event.hpp>
#include <tuinator/platform/clipboard.hpp>
#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/text.hpp>
#include <tuinator/render/text_edit.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/controls/text_input.hpp>

#include <algorithm>
#include <variant>

namespace tuinator {

namespace {

int display_width_before(std::string_view text, std::size_t byte_index) {
    return text_display_width(text.substr(0, byte_index));
}

Style with_field_background(const Style& base, const Style& field) {
    Style style = base;
    style.background = field.background;
    style.background_rgb = field.background_rgb;
    return style;
}

Style make_selection_style(const Style& base, const Style& field) {
    Style style = with_field_background(base, field);
    style.reverse = true;
    if (field.foreground_rgb.has_value() && field.background_rgb.has_value()) {
        style.reverse = false;
        style.foreground_rgb = field.background_rgb;
        style.background_rgb = field.foreground_rgb;
    }
    return style;
}

Style make_cursor_style(const Style& base, const Style& field) {
    Style style = with_field_background(base, field);
    style.reverse = false;
    style.bold = false;
    style.dim = false;
    if (field.foreground_rgb.has_value() && field.background_rgb.has_value()) {
        style.foreground_rgb = field.background_rgb;
        style.background_rgb = field.foreground_rgb;
    } else {
        style.reverse = true;
    }
    return style;
}

char cursor_block_char(bool password) { return password ? '*' : ' '; }

int count_lines(std::string_view text) {
    if (text.empty()) {
        return 1;
    }
    int lines = 1;
    for (char ch : text) {
        if (ch == '\n') {
            ++lines;
        }
    }
    return lines;
}

std::size_t line_start(std::string_view text, int line) {
    std::size_t pos = 0;
    int current = 0;
    while (pos < text.size() && current < line) {
        if (text[pos] == '\n') {
            ++current;
        }
        ++pos;
    }
    return pos;
}

std::size_t line_end(std::string_view text, int line) {
    const std::size_t start = line_start(text, line);
    std::size_t pos = start;
    while (pos < text.size() && text[pos] != '\n') {
        ++pos;
    }
    return pos;
}

int cursor_line(std::string_view text, std::size_t cursor) {
    int line = 0;
    for (std::size_t i = 0; i < cursor && i < text.size(); ++i) {
        if (text[i] == '\n') {
            ++line;
        }
    }
    return line;
}

std::size_t byte_index_at_line_col(std::string_view text, int line, int col) {
    const std::size_t start = line_start(text, line);
    const std::size_t end = line_end(text, line);
    const std::string line_text(text.substr(start, end - start));
    const std::size_t offset = text_caret_index_at_column(line_text, col, CaretAffinity::Before);
    return start + std::min(offset, end - start);
}

std::size_t extend_through_trailing_spaces(std::string_view text, std::size_t line_begin, std::size_t line_end,
                                            std::size_t limit, int width) {
    if (width <= 0) {
        return line_end;
    }
    while (line_end < limit && text[line_end] == ' ') {
        const std::size_t next = line_end + 1;
        if (text_display_width(text.substr(line_begin, next - line_begin)) > width) {
            break;
        }
        line_end = next;
    }
    return line_end;
}

void wrap_plain_segment(std::string_view text, std::size_t seg_start, std::size_t seg_end, int width,
                        std::vector<WrappedLine>& out) {
    if (seg_start >= seg_end) {
        out.push_back({seg_start, seg_start});
        return;
    }
    if (width <= 0) {
        out.push_back({seg_start, seg_end});
        return;
    }

    std::size_t line_begin = seg_start;
    std::size_t pos = seg_start;
    while (pos < seg_end) {
        while (pos < seg_end && text[pos] == ' ') {
            ++pos;
        }
        if (pos >= seg_end) {
            out.push_back({line_begin, seg_end});
            return;
        }

        int used = 0;
        std::size_t line_end = pos;
        while (pos < seg_end) {
            while (pos < seg_end && text[pos] == ' ') {
                ++pos;
            }
            if (pos >= seg_end) {
                break;
            }

            const std::size_t word_start = pos;
            while (pos < seg_end && text[pos] != ' ') {
                ++pos;
            }
            const std::string_view word = text.substr(word_start, pos - word_start);
            const int word_w = text_display_width(word);
            const int gap = line_end == line_begin ? 0 : 1;

            if (word_w > width) {
                if (used > 0) {
                    out.push_back({line_begin, extend_through_trailing_spaces(text, line_begin, line_end, seg_end, width)});
                    line_begin = word_start;
                    used = 0;
                    pos = word_start;
                    break;
                }
                const std::size_t chunk = text_byte_length_for_width(word, width);
                const std::size_t chunk_end = word_start + std::max<std::size_t>(1, chunk);
                out.push_back({word_start, std::min(chunk_end, seg_end)});
                line_begin = std::min(chunk_end, seg_end);
                pos = line_begin;
                used = 0;
                break;
            }

            if (used > 0 && used + gap + word_w > width) {
                out.push_back({line_begin, extend_through_trailing_spaces(text, line_begin, line_end, seg_end, width)});
                line_begin = word_start;
                used = word_w;
                line_end = pos;
                continue;
            }

            used += (used > 0 ? gap : 0) + word_w;
            line_end = pos;
        }

        if (line_end > line_begin) {
            const std::size_t push_end = extend_through_trailing_spaces(text, line_begin, line_end, seg_end, width);
            out.push_back({line_begin, push_end});
            line_begin = push_end;
            pos = line_begin;
            continue;
        }
        break;
    }
}

std::vector<WrappedLine> build_wrapped_lines(std::string_view value, int width) {
    std::vector<WrappedLine> out;
    if (value.empty()) {
        out.push_back({0, 0});
        return out;
    }

    std::size_t at = 0;
    while (at <= value.size()) {
        std::size_t para_end = value.find('\n', at);
        if (para_end == std::string::npos) {
            para_end = value.size();
        }
        wrap_plain_segment(value, at, para_end, width, out);
        if (para_end >= value.size()) {
            break;
        }
        at = para_end + 1;
    }
    if (out.empty()) {
        out.push_back({0, 0});
    }
    return out;
}

} // namespace

TextInput::TextInput(TextInputOptions options, Style style, Style focused_style)
    : placeholder_(std::move(options.placeholder)), min_width_(std::max(1, options.min_width)),
      password_(options.password), chrome_(options.chrome), min_content_lines_(std::max(1, options.min_content_lines)),
      max_content_lines_(std::max(min_content_lines_, options.max_content_lines)), prompt_(std::move(options.prompt)),
      border_label_(std::move(options.border_label)),
      framed_border_style_(options.framed_border_style),
      framed_border_glyphs_(options.framed_border_glyphs),
      framed_show_border_(options.framed_show_border), soft_wrap_(options.soft_wrap),
      content_pad_left_(std::max(0, options.content_pad_left)),
      content_pad_right_(std::max(0, options.content_pad_right)), style_(style), focused_style_(focused_style) {}

void TextInput::set_options(TextInputOptions options) {
    min_width_ = std::max(1, options.min_width);
    placeholder_ = std::move(options.placeholder);
    password_ = options.password;
    chrome_ = options.chrome;
    min_content_lines_ = std::max(1, options.min_content_lines);
    max_content_lines_ = std::max(min_content_lines_, options.max_content_lines);
    prompt_ = std::move(options.prompt);
    border_label_ = std::move(options.border_label);
    framed_border_style_ = options.framed_border_style;
    framed_border_glyphs_ = options.framed_border_glyphs;
    framed_show_border_ = options.framed_show_border;
    soft_wrap_ = options.soft_wrap;
    content_pad_left_ = std::max(0, options.content_pad_left);
    content_pad_right_ = std::max(0, options.content_pad_right);
    mark_dirty();
    update_layout_for_lines();
}

void TextInput::set_field_styles(Style normal, Style focused) {
    style_ = std::move(normal);
    focused_style_ = std::move(focused);
    mark_dirty();
}

void TextInput::apply_stylesheet(const StyleResolver& styles) { apply_text_input_stylesheet(*this, *this, styles); }

void TextInput::set_value(std::string value) {
    value_ = std::move(value);
    cursor_ = value_.size();
    selection_anchor_ = cursor_;
    ensure_cursor_visible();
    mark_dirty();
}

void TextInput::set_placeholder(std::string placeholder) {
    placeholder_ = std::move(placeholder);
    mark_dirty();
}

void TextInput::set_on_change(std::function<void(const std::string&)> callback) { on_change_ = std::move(callback); }

void TextInput::set_on_submit(std::function<void(const std::string&)> callback) { on_submit_ = std::move(callback); }

bool TextInput::use_soft_wrap() const {
    return soft_wrap_ && max_content_lines_ > 1 && chrome_ == TextInputChrome::Framed;
}

int TextInput::wrap_width_for_layout() const {
    const int w = inner_width();
    if (w > 0) {
        return w;
    }
    if (last_wrap_width_ > 0) {
        return last_wrap_width_;
    }
    return std::max(1, min_width_);
}

std::vector<WrappedLine> TextInput::wrapped_lines() const {
    if (!use_soft_wrap()) {
        return {};
    }
    return build_wrapped_lines(value_, wrap_width_for_layout());
}

int TextInput::wrapped_line_count() const {
    if (!use_soft_wrap()) {
        return count_lines(value_);
    }
    return static_cast<int>(wrapped_lines().size());
}

int TextInput::cursor_wrapped_row() const {
    const auto lines = wrapped_lines();
    if (lines.empty()) {
        return 0;
    }
    // Segments are ordered [start, end) byte ranges; the caret at a boundary belongs
    // to the later visual row (e.g. after a space that starts the next wrapped line).
    int row = 0;
    for (int i = 0; i < static_cast<int>(lines.size()); ++i) {
        if (cursor_ >= lines[static_cast<std::size_t>(i)].start) {
            row = i;
        } else {
            break;
        }
    }
    return row;
}

int TextInput::content_line_count() const {
    const int lines = use_soft_wrap() ? wrapped_line_count() : count_lines(value_);
    return std::clamp(lines, min_content_lines_, max_content_lines_);
}

int TextInput::framed_height_for_lines(int lines) const { return std::max(3, lines + 2); }

Size TextInput::preferred_size() const {
    if (chrome_ == TextInputChrome::Framed) {
        const int prompt_cols = text_display_width(prompt_);
        int width = min_width_ + 4 + prompt_cols;
        if (!border_label_.empty()) {
            // "─ " + label + " ─" plus both corners.
            width = std::max(width, text_display_width(border_label_) + 6);
        }
        return {width, framed_height_for_lines(content_line_count())};
    }
    return {min_width_ + 2, 1};
}

int TextInput::inner_height() const {
    if (chrome_ == TextInputChrome::Framed) {
        return std::max(1, bounds_.height - 2);
    }
    return 1;
}

void TextInput::update_layout_for_lines() {
    if (chrome_ == TextInputChrome::Framed && max_content_lines_ > 1) {
        mark_layout_dirty();
    }
}

void TextInput::layout(Rect bounds) {
    bounds_ = bounds;
    const int w = inner_width();
    if (w > 0) {
        last_wrap_width_ = w;
    }
    ensure_cursor_visible();
}

int TextInput::text_row() const { return chrome_ == TextInputChrome::Framed ? 1 : 0; }

int TextInput::text_col_start() const {
    if (chrome_ == TextInputChrome::Framed) {
        return 2 + text_display_width(prompt_) + content_pad_left_;
    }
    return 1;
}

int TextInput::inner_width() const {
    if (chrome_ == TextInputChrome::Framed) {
        return std::max(0, bounds_.width - 4 - text_display_width(prompt_) - content_pad_left_ - content_pad_right_);
    }
    return std::max(0, bounds_.width - 2);
}

int TextInput::display_column_at_local(int local_x) const {
    const int inner = inner_width();
    const int start_col = text_col_start();
    if (use_soft_wrap()) {
        if (local_x <= start_col) {
            return 0;
        }
        if (local_x >= start_col + inner) {
            return inner;
        }
        return local_x - start_col;
    }
    if (local_x <= start_col) {
        return scroll_x_;
    }
    if (local_x >= start_col + inner) {
        return scroll_x_ + inner;
    }
    return scroll_x_ + (local_x - start_col);
}

std::size_t TextInput::byte_index_at_local(int local_x, int local_y, CaretAffinity affinity) const {
    const int display_col = display_column_at_local(local_x);
    if (max_content_lines_ <= 1) {
        const std::size_t line_start_pos = line_start(value_, 0);
        const std::size_t line_end_pos = line_end(value_, 0);
        const std::string line_text = value_.substr(line_start_pos, line_end_pos - line_start_pos);
        if (display_col >= text_display_width(line_text)) {
            return line_end_pos;
        }
        return line_start_pos + text_caret_index_at_column(line_text, display_col, affinity);
    }

    if (use_soft_wrap()) {
        const std::vector<WrappedLine> lines = wrapped_lines();
        if (lines.empty()) {
            return 0;
        }
        const int row = std::max(0, local_y - text_row());
        int wrap_row = scroll_y_ + row;
        wrap_row = std::clamp(wrap_row, 0, static_cast<int>(lines.size()) - 1);
        const WrappedLine& wrapped = lines[static_cast<std::size_t>(wrap_row)];
        const std::string line_text = value_.substr(wrapped.start, wrapped.end - wrapped.start);
        if (display_col >= text_display_width(line_text)) {
            return wrapped.end;
        }
        return wrapped.start + text_caret_index_at_column(line_text, display_col, affinity);
    }

    const int row = std::max(0, local_y - text_row());
    int line = scroll_y_ + row;
    const int lines = count_lines(value_);
    line = std::clamp(line, 0, lines - 1);
    return byte_index_at_line_col(value_, line, display_col);
}

bool TextInput::handle_mouse(const MouseEvent& mouse) {
    if (mouse.action == MouseAction::WheelUp || mouse.action == MouseAction::WheelDown) {
        if (!contains_point(mouse.position) || max_content_lines_ <= 1) {
            return false;
        }

        const int visible_lines = inner_height();
        const int total_lines = use_soft_wrap() ? wrapped_line_count() : count_lines(value_);
        const int max_scroll_y = std::max(0, total_lines - visible_lines);
        if (max_scroll_y <= 0) {
            return false;
        }

        if (mouse.action == MouseAction::WheelUp) {
            scroll_y_ = std::max(0, scroll_y_ - 1);
        } else {
            scroll_y_ = std::min(max_scroll_y, scroll_y_ + 1);
        }
        mark_dirty();
        return true;
    }

    if (mouse.action == MouseAction::Release) {
        if (selecting_with_mouse_) {
            const Point local{mouse.position.x - bounds_.x, mouse.position.y - bounds_.y};
            const int release_display_col = display_column_at_local(local.x);
            if (release_display_col != mouse_press_display_col_) {
                set_cursor(byte_index_at_local(local.x, local.y, CaretAffinity::After), true);
            } else {
                set_cursor(mouse_press_index_, false);
            }
            selecting_with_mouse_ = false;
            mark_dirty();
            return true;
        }
        return contains_point(mouse.position);
    }

    if (mouse.action == MouseAction::Move) {
        if (!selecting_with_mouse_ || !mouse.left_pressed) {
            return false;
        }

        const Point local{mouse.position.x - bounds_.x, mouse.position.y - bounds_.y};
        set_cursor(byte_index_at_local(local.x, local.y, CaretAffinity::After), true);
        selecting_with_mouse_ = true;
        return true;
    }

    if (mouse.action == MouseAction::Press || mouse.action == MouseAction::Click) {
        if (!contains_point(mouse.position)) {
            return false;
        }

        const Point local{mouse.position.x - bounds_.x, mouse.position.y - bounds_.y};
        mouse_press_display_col_ = display_column_at_local(local.x);
        mouse_press_index_ = byte_index_at_local(local.x, local.y, CaretAffinity::Before);
        set_cursor(mouse_press_index_, false);
        selecting_with_mouse_ = true;
        return true;
    }

    return false;
}

void TextInput::ensure_cursor_visible() {
    const int width = inner_width();
    if (width <= 0) {
        scroll_x_ = 0;
        scroll_y_ = 0;
        return;
    }

    if (use_soft_wrap()) {
        const int line = cursor_wrapped_row();
        scroll_x_ = 0;
        if (max_content_lines_ > 1) {
            const int visible_lines = inner_height();
            if (line < scroll_y_) {
                scroll_y_ = line;
            }
            if (line >= scroll_y_ + visible_lines) {
                scroll_y_ = line - visible_lines + 1;
            }
            const int max_scroll_y = std::max(0, wrapped_line_count() - visible_lines);
            scroll_y_ = std::clamp(scroll_y_, 0, max_scroll_y);
        } else {
            scroll_y_ = 0;
        }
        return;
    }

    const int line = cursor_line(value_, cursor_);
    if (max_content_lines_ > 1) {
        const int visible_lines = inner_height();
        if (line < scroll_y_) {
            scroll_y_ = line;
        }
        if (line >= scroll_y_ + visible_lines) {
            scroll_y_ = line - visible_lines + 1;
        }
        const int max_scroll_y = std::max(0, count_lines(value_) - visible_lines);
        scroll_y_ = std::clamp(scroll_y_, 0, max_scroll_y);
    } else {
        scroll_y_ = 0;
    }

    const std::size_t line_start_pos = line_start(value_, line);
    const std::size_t line_end_pos = line_end(value_, line);
    const std::string line_text = value_.substr(line_start_pos, line_end_pos - line_start_pos);
    const int cursor_col = display_width_before(line_text, cursor_ - line_start_pos);
    const int right_edge =
        cursor_ == line_end_pos ? text_display_width(line_text) + (cursor_ == value_.size() ? 1 : 0) : cursor_col;

    if (cursor_col < scroll_x_) {
        scroll_x_ = cursor_col;
    }
    if (right_edge >= scroll_x_ + width) {
        scroll_x_ = right_edge - width + 1;
    }

    const int content_width = text_display_width(line_text) + (cursor_ == line_end_pos ? 1 : 0);
    const int max_scroll = std::max(0, content_width - width);
    scroll_x_ = std::clamp(scroll_x_, 0, max_scroll);
}

std::optional<Point> TextInput::caret_local_cell() const {
    if (!is_focused() || has_selection()) {
        return std::nullopt;
    }

    const int row_base = text_row();
    const int col_start = text_col_start();
    const int width = inner_width();

    if (value_.empty()) {
        return Point{col_start, row_base};
    }

    int cursor_line_idx = cursor_line(value_, cursor_);
    std::size_t line_start_pos = line_start(value_, cursor_line_idx);
    std::size_t line_end_pos = line_end(value_, cursor_line_idx);
    int line_scroll = scroll_x_;
    if (use_soft_wrap()) {
        cursor_line_idx = cursor_wrapped_row();
        const std::vector<WrappedLine> lines = wrapped_lines();
        if (lines.empty()) {
            return Point{col_start, row_base};
        }
        const WrappedLine& wrapped = lines[static_cast<std::size_t>(cursor_line_idx)];
        line_start_pos = wrapped.start;
        line_end_pos = wrapped.end;
        line_scroll = 0;
    }
    const int paint_row = row_base + (cursor_line_idx - scroll_y_);
    if (paint_row < row_base || paint_row >= bounds_.height) {
        return std::nullopt;
    }

    const std::string line_text = value_.substr(line_start_pos, line_end_pos - line_start_pos);
    const int col = col_start + display_width_before(line_text, cursor_ - line_start_pos) - line_scroll;
    if (col < col_start || col >= col_start + width) {
        return std::nullopt;
    }
    return Point{col, paint_row};
}

void TextInput::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    const bool focused = is_focused();
    const StyleResolver& styles = ctx.styles();
    const Style normal_style = styles.text(*this, style_);
    const Style focused_style = styles.focused(*this, focused_style_);
    Style text_style = focused ? focused_style : normal_style;
    Style placeholder_style{};
    placeholder_style.foreground = text_style.foreground;
    placeholder_style.background = text_style.background;
    placeholder_style.background_rgb = text_style.background_rgb;
    placeholder_style.dim = true;

    paint_bounds_background(ctx, text_style);

    const int width = inner_width();
    const int row = text_row();
    const int col_start = text_col_start();
    Style fill_style = text_style;
    if (chrome_ == TextInputChrome::Framed) {
        if (!fill_style.background_rgb.has_value()) {
            const Style field = styles.text(*this, style_);
            fill_style.background_rgb = field.background_rgb;
        }
        placeholder_style.background = fill_style.background;
        placeholder_style.background_rgb = fill_style.background_rgb;
        text_style.background = fill_style.background;
        text_style.background_rgb = fill_style.background_rgb;
        placeholder_style.foreground_rgb = text_style.foreground_rgb;
        placeholder_style.dim = true;
    }

    if (chrome_ == TextInputChrome::Framed) {
        Style border_style = text_style;
        border_style.dim = false;
        border_style.background = fill_style.background;
        border_style.background_rgb = fill_style.background_rgb;
        const Style resolved_border = styles.border(*this, text_style);
        if (resolved_border.foreground_rgb.has_value()) {
            border_style.foreground_rgb = resolved_border.foreground_rgb;
        } else if (text_style.foreground_rgb.has_value()) {
            border_style.foreground_rgb = text_style.foreground_rgb;
        }

        const BorderGlyphs glyphs =
            framed_border_glyphs_.has_value() ? *framed_border_glyphs_ : border_glyphs_for(framed_border_style_);
        if (bounds_.width >= 2 && bounds_.height >= 2) {
            if (framed_show_border_) {
                canvas.fill_rect({1, 1, bounds_.width - 2, bounds_.height - 2}, ' ', fill_style);
                canvas.draw_box({0, 0, bounds_.width, bounds_.height}, border_style, glyphs);
            } else {
                canvas.fill_rect({0, 0, bounds_.width, bounds_.height}, ' ', fill_style);
            }
        }

        if (!border_label_.empty() && bounds_.width > 4 && framed_show_border_) {
            // Embed the label into the top border; border glyphs use the same style as the frame.
            const std::string prefix = glyphs.horizontal + " ";
            const std::string suffix = " " + glyphs.horizontal;
            const int inner_width = bounds_.width - 2;
            const int prefix_width = text_display_width(prefix);
            const int suffix_width = text_display_width(suffix);
            const int label_max = std::max(0, inner_width - prefix_width - suffix_width);
            const std::size_t label_bytes = text_byte_length_for_width(border_label_, label_max);
            const std::string label_text = border_label_.substr(0, label_bytes);

            canvas.draw_text({1, 0}, prefix, border_style);
            Style label_text_style = border_style;
            label_text_style.bold = true;
            canvas.draw_text({1 + prefix_width, 0}, label_text, label_text_style);
            if (!label_text.empty() || suffix_width <= inner_width - prefix_width) {
                canvas.draw_text({1 + prefix_width + text_display_width(label_text), 0}, suffix, border_style);
            }
        }

        if (!prompt_.empty() && row < bounds_.height) {
            Style prompt_style = text_style;
            prompt_style.bold = true;
            prompt_style.foreground_rgb = border_style.foreground_rgb;
            prompt_style.background = fill_style.background;
            prompt_style.background_rgb = fill_style.background_rgb;
            canvas.draw_text({2, row}, prompt_, prompt_style);
        }
    } else {
        canvas.draw_text({0, row}, "[", text_style);
    }

    const int visible_lines = max_content_lines_ > 1 ? inner_height() : 1;
    const auto [sel_start, sel_end] = selection_range();
    const int cursor_line_idx = cursor_line(value_, cursor_);

    if (value_.empty() && !focused && !placeholder_.empty()) {
        const std::size_t bytes = text_byte_length_for_width(placeholder_, width);
        canvas.draw_text({col_start, row}, placeholder_.substr(0, bytes), placeholder_style);
    } else if (value_.empty() && focused) {
        Style cursor_style = make_cursor_style(text_style, text_style);
        canvas.draw_char({col_start, row}, cursor_block_char(password_), cursor_style);
    } else {
        const std::vector<WrappedLine> wrapped = use_soft_wrap() ? wrapped_lines() : std::vector<WrappedLine>{};
        const int active_cursor_row = use_soft_wrap() ? cursor_wrapped_row() : cursor_line_idx;
        for (int visible_row = 0; visible_row < visible_lines; ++visible_row) {
            const int line_idx = scroll_y_ + visible_row;
            const int paint_row = row + visible_row;
            if (paint_row >= bounds_.height) {
                break;
            }
            std::size_t line_start_pos = 0;
            std::size_t line_end_pos = 0;
            int line_scroll = 0;
            if (use_soft_wrap()) {
                if (line_idx >= static_cast<int>(wrapped.size())) {
                    break;
                }
                line_start_pos = wrapped[static_cast<std::size_t>(line_idx)].start;
                line_end_pos = wrapped[static_cast<std::size_t>(line_idx)].end;
            } else {
                if (line_idx >= count_lines(value_)) {
                    break;
                }
                line_start_pos = line_start(value_, line_idx);
                line_end_pos = line_end(value_, line_idx);
                line_scroll = (line_idx == active_cursor_row) ? scroll_x_ : 0;
            }
            const std::string line_text = value_.substr(line_start_pos, line_end_pos - line_start_pos);
            const std::size_t start_byte =
                line_start_pos + text_caret_index_at_column(line_text, line_scroll, CaretAffinity::Before);
            const std::size_t visible_bytes =
                start_byte + text_byte_length_for_width(line_text.substr(start_byte - line_start_pos), width);
            const std::size_t paint_end = std::min(visible_bytes, line_end_pos);

            for (std::size_t index = start_byte; index < paint_end;) {
                if (index >= line_end_pos) {
                    break;
                }
                const std::size_t char_len = utf8_char_length(value_, index);
                const int col = col_start + display_width_before(line_text, index - line_start_pos) - line_scroll;
                if (col >= col_start + width) {
                    break;
                }

                const bool selected = index >= sel_start && index < sel_end;
                const bool at_cursor = focused && index == cursor_ && !has_selection();

                Style glyph_style = text_style;
                if (chrome_ == TextInputChrome::Framed) {
                    glyph_style = with_field_background(glyph_style, text_style);
                }
                if (selected) {
                    glyph_style = make_selection_style(glyph_style, text_style);
                } else if (at_cursor) {
                    glyph_style = make_cursor_style(glyph_style, text_style);
                }

                const std::string glyph =
                    password_ ? std::string(1, '*') : value_.substr(index, static_cast<std::size_t>(char_len));
                canvas.draw_text({col, paint_row}, glyph, glyph_style);
                index += char_len;
            }

            if (focused && cursor_ == line_end_pos && !has_selection()) {
                const bool draw_end_cursor =
                    !use_soft_wrap() || cursor_wrapped_row() == line_idx;
                if (draw_end_cursor) {
                    const int col =
                        col_start + display_width_before(line_text, cursor_ - line_start_pos) - line_scroll;
                    if (col >= col_start && col < col_start + width) {
                        Style cursor_style = make_cursor_style(text_style, text_style);
                        canvas.draw_char({col, paint_row}, cursor_block_char(password_), cursor_style);
                    }
                }
            }
        }
    }

    if (chrome_ == TextInputChrome::Brackets) {
        canvas.draw_text({bounds_.width - 1, row}, "]", text_style);
    }
}

bool TextInput::handle_event(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        return handle_mouse(*mouse);
    }

    if (const auto* paste = std::get_if<ClipboardPaste>(&event)) {
        if (!is_focused()) {
            return false;
        }
        insert_text(paste->text);
        return true;
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key || !is_focused()) {
        return false;
    }

    if (handle_shortcut(*key)) {
        return true;
    }

    switch (key->key) {
    case Key::Backspace:
        if (key->ctrl || key->alt) {
            delete_word_before_cursor();
        } else if (has_selection()) {
            delete_selection();
        } else {
            delete_before_cursor();
        }
        return true;
    case Key::Delete:
        if (key->ctrl || key->alt) {
            delete_word_after_cursor();
        } else if (has_selection()) {
            delete_selection();
        } else {
            delete_at_cursor();
        }
        return true;
    case Key::Left: move_cursor(-1, key->shift); return true;
    case Key::Right: move_cursor(1, key->shift); return true;
    case Key::Up:
        if (max_content_lines_ > 1) {
            move_cursor_vertical(-1, key->shift);
            return true;
        }
        return false;
    case Key::Down:
        if (max_content_lines_ > 1) {
            move_cursor_vertical(1, key->shift);
            return true;
        }
        return false;
    case Key::Home:
        if (max_content_lines_ > 1 && !key->ctrl) {
            const int line = cursor_line(value_, cursor_);
            set_cursor(line_start(value_, line), key->shift);
            return true;
        }
        set_cursor(0, key->shift);
        return true;
    case Key::End:
        if (max_content_lines_ > 1 && !key->ctrl) {
            const int line = cursor_line(value_, cursor_);
            set_cursor(line_end(value_, line), key->shift);
            return true;
        }
        set_cursor(value_.size(), key->shift);
        return true;
    case Key::Escape:
        if (has_selection()) {
            clear_selection();
            mark_dirty();
            return true;
        }
        return false;
    case Key::Enter:
        if (max_content_lines_ > 1 && (key->shift || key->text == "\n")) {
            insert_char('\n');
            return true;
        }
        if (on_submit_) {
            on_submit_(value_);
        }
        return true;
    default: break;
    }

    const std::string insert_text_value = keypress_insert_text(*key);
    if (!insert_text_value.empty() && allows_text_insert_modifiers(*key)) {
        if (has_selection()) {
            delete_selection();
        }
        if (insert_text(insert_text_value)) {
            return true;
        }
    }

    return false;
}

bool TextInput::handle_shortcut(const KeyPress& key) {
    if (is_altgr(key)) {
        return false;
    }

    char shortcut = '\0';
    if (key.ctrl && key.character >= 'a' && key.character <= 'z') {
        shortcut = key.character;
    } else if (key.character >= 1 && key.character <= 26) {
        shortcut = static_cast<char>('a' + key.character - 1);
    }

    if (shortcut == '\0') {
        return false;
    }

    switch (shortcut) {
    case 'a': select_all(); return true;
    case 'c':
        if (!has_selection()) {
            return false;
        }
        copy_selection();
        return true;
    case 'x':
        if (!has_selection()) {
            return false;
        }
        cut_selection();
        return true;
    case 'v':
        // Paste is delivered as ClipboardPaste when bracketed-paste mode is active.
        return true;
    case 'w': delete_word_before_cursor(); return true;
    case 'u': delete_line_before_cursor(); return true;
    case 'k': delete_line_after_cursor(); return true;
    default: break;
    }

    return false;
}

void TextInput::insert_char(char ch) {
    value_.insert(cursor_, 1, ch);
    ++cursor_;
    selection_anchor_ = cursor_;
    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

void TextInput::delete_before_cursor() {
    if (cursor_ == 0) {
        return;
    }

    value_.erase(cursor_ - 1, 1);
    --cursor_;
    selection_anchor_ = cursor_;
    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

void TextInput::delete_at_cursor() {
    if (cursor_ >= value_.size()) {
        return;
    }

    value_.erase(cursor_, 1);
    selection_anchor_ = cursor_;
    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

void TextInput::delete_selection() {
    if (!has_selection()) {
        return;
    }

    const auto [start, end] = selection_range();
    value_.erase(start, end - start);
    cursor_ = start;
    clear_selection();
    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

void TextInput::delete_word_before_cursor() {
    if (has_selection()) {
        delete_selection();
        return;
    }

    const std::size_t start = text_edit::previous_word_boundary(value_, cursor_);
    if (start == cursor_) {
        return;
    }

    value_.erase(start, cursor_ - start);
    cursor_ = start;
    selection_anchor_ = cursor_;
    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

void TextInput::delete_word_after_cursor() {
    if (has_selection()) {
        delete_selection();
        return;
    }

    const std::size_t end = text_edit::next_word_boundary(value_, cursor_);
    if (end == cursor_) {
        return;
    }

    value_.erase(cursor_, end - cursor_);
    selection_anchor_ = cursor_;
    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

void TextInput::delete_line_before_cursor() {
    if (cursor_ == 0) {
        return;
    }

    value_.erase(0, cursor_);
    cursor_ = 0;
    selection_anchor_ = 0;
    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

void TextInput::delete_line_after_cursor() {
    if (cursor_ >= value_.size()) {
        return;
    }

    value_.erase(cursor_);
    selection_anchor_ = cursor_;
    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

void TextInput::delete_entire_field() {
    if (value_.empty()) {
        return;
    }

    value_.clear();
    cursor_ = 0;
    selection_anchor_ = 0;
    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

bool TextInput::insert_text(std::string_view text) {
    if (text.empty()) {
        return false;
    }

    if (has_selection()) {
        delete_selection();
    }

    std::string cleaned;
    for (std::size_t index = 0; index < text.size();) {
        const unsigned char byte = static_cast<unsigned char>(text[index]);
        if (byte == '\r') {
            ++index;
            continue;
        }
        if (byte == '\n') {
            if (max_content_lines_ <= 1) {
                ++index;
                continue;
            }
            cleaned.push_back('\n');
            ++index;
            continue;
        }
        if (byte == '\t') {
            ++index;
            continue;
        }
        if (byte < 32) {
            ++index;
            continue;
        }

        const std::size_t length = utf8_char_length(text, index);
        cleaned.append(text.data() + index, length);
        index += length;
    }

    if (cleaned.empty()) {
        return false;
    }

    value_.insert(cursor_, cleaned);
    cursor_ += cleaned.size();
    selection_anchor_ = cursor_;
    ensure_cursor_visible();
    mark_dirty();
    notify_change();
    return true;
}

void TextInput::copy_selection() {
    if (!has_selection()) {
        return;
    }

    clipboard::set(selected_text());
}

void TextInput::cut_selection() {
    if (!has_selection()) {
        return;
    }

    clipboard::set(selected_text());
    delete_selection();
}

void TextInput::paste_from_clipboard() { insert_text(clipboard::get()); }

std::string TextInput::selected_text() const {
    if (!has_selection()) {
        return {};
    }

    const auto [start, end] = selection_range();
    return value_.substr(start, end - start);
}

void TextInput::move_cursor_vertical(int delta, bool extend_selection) {
    if (delta == 0) {
        return;
    }

    if (use_soft_wrap()) {
        const std::vector<WrappedLine> lines = wrapped_lines();
        if (lines.empty()) {
            return;
        }
        const int current_row = cursor_wrapped_row();
        const WrappedLine& current = lines[static_cast<std::size_t>(current_row)];
        const int col = text_display_width(value_.substr(current.start, cursor_ - current.start));
        const int new_row = std::clamp(current_row + delta, 0, static_cast<int>(lines.size()) - 1);
        const WrappedLine& target = lines[static_cast<std::size_t>(new_row)];
        const std::string target_text = value_.substr(target.start, target.end - target.start);
        const int target_col = std::min(col, text_display_width(target_text));
        set_cursor(target.start + text_caret_index_at_column(target_text, target_col, CaretAffinity::Before),
                   extend_selection);
        return;
    }

    const int line = cursor_line(value_, cursor_);
    const std::size_t line_start_pos = line_start(value_, line);
    const int col = text_display_width(value_.substr(line_start_pos, cursor_ - line_start_pos));
    const int lines = count_lines(value_);
    const int new_line = std::clamp(line + delta, 0, lines - 1);
    const std::size_t new_line_start = line_start(value_, new_line);
    const std::size_t new_line_end = line_end(value_, new_line);
    const std::string new_line_text = value_.substr(new_line_start, new_line_end - new_line_start);
    const int target_col = std::min(col, text_display_width(new_line_text));
    set_cursor(byte_index_at_line_col(value_, new_line, target_col), extend_selection);
}

void TextInput::move_cursor(int delta, bool extend_selection) {
    if (delta == 0) {
        return;
    }

    std::size_t next = cursor_;
    if (delta < 0) {
        for (int steps = 0; steps < -delta && next > 0; ++steps) {
            const std::size_t step = utf8_char_length(value_, next - 1);
            next -= step > 0 ? step : 1;
        }
    } else {
        for (int steps = 0; steps < delta && next < value_.size(); ++steps) {
            const std::size_t step = utf8_char_length(value_, next);
            next += step > 0 ? step : 1;
        }
    }

    set_cursor(next, extend_selection);
}

void TextInput::set_cursor(std::size_t pos, bool extend_selection) {
    const std::size_t next = std::min(pos, value_.size());
    if (!extend_selection) {
        selection_anchor_ = next;
    }

    if (cursor_ == next && (!extend_selection || selection_anchor_ == next)) {
        return;
    }

    cursor_ = next;
    ensure_cursor_visible();
    mark_dirty();
}

void TextInput::select_all() {
    selection_anchor_ = 0;
    cursor_ = value_.size();
    ensure_cursor_visible();
    mark_dirty();
}

void TextInput::clear_selection() { selection_anchor_ = cursor_; }

bool TextInput::has_selection() const { return selection_anchor_ != cursor_; }

std::pair<std::size_t, std::size_t> TextInput::selection_range() const {
    const std::size_t start = std::min(selection_anchor_, cursor_);
    const std::size_t end = std::max(selection_anchor_, cursor_);
    return {start, end};
}

void TextInput::notify_change() {
    update_layout_for_lines();
    if (on_change_) {
        on_change_(value_);
    }
}

} // namespace tuinator
