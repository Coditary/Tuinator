#include <tuinator/core/event.hpp>
#include <tuinator/platform/clipboard.hpp>
#include <tuinator/render/text.hpp>
#include <tuinator/render/text_edit.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/controls/text_area.hpp>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <tuple>
#include <variant>

namespace tuinator {

namespace {

std::vector<std::string> split_lines(const std::string& value) {
    std::vector<std::string> lines;
    std::size_t start = 0;
    while (true) {
        const std::size_t end = value.find('\n', start);
        if (end == std::string::npos) {
            lines.push_back(value.substr(start));
            break;
        }
        lines.push_back(value.substr(start, end - start));
        start = end + 1;
    }
    if (lines.empty()) {
        lines.emplace_back();
    }
    return lines;
}

int digit_count(int value) {
    int digits = 1;
    int remaining = std::max(1, value);
    while (remaining >= 10) {
        remaining /= 10;
        ++digits;
    }
    return digits;
}

bool is_printable(char ch) { return ch >= 32 && ch <= 126; }

bool style_customized(const Style& style) {
    return style.foreground != Color::Default || style.background != Color::Default ||
           style.foreground_rgb.has_value() || style.background_rgb.has_value() || style.bold || style.dim ||
           style.reverse;
}

std::string fit_gutter_text(std::string_view text, int columns) {
    if (columns <= 0) {
        return {};
    }

    const int width = text_display_width(text);
    if (width == columns) {
        return std::string(text);
    }
    if (width > columns) {
        return std::string(text.substr(0, text_byte_length_for_width(text, columns)));
    }
    return std::string(static_cast<std::size_t>(columns - width), ' ') + std::string(text);
}

} // namespace

GutterCell absolute_gutter(const GutterLine& line) { return {std::to_string(line.number), {}}; }

GutterCell relative_gutter(const GutterLine& line) {
    if (line.current) {
        return {"0", {}};
    }
    return {std::to_string(std::abs(line.index - line.cursor_index)), {}};
}

GutterCell hybrid_gutter(const GutterLine& line) {
    if (line.current) {
        Style style{};
        style.bold = true;
        return {std::to_string(line.number), style};
    }

    Style style{};
    style.dim = true;
    return {std::to_string(std::abs(line.index - line.cursor_index)), style};
}

TextArea::TextArea(TextAreaOptions options, Style style, Style focused_style)
    : title_(std::move(options.title)), placeholder_(std::move(options.placeholder)),
      min_width_(std::max(8, options.min_width)), min_height_(std::max(3, options.min_height)),
      line_numbers_(options.line_numbers), status_bar_(options.status_bar),
      gutter_width_(std::max(0, options.gutter_width)), scrollbars_(std::move(options.scrollbars)), style_(style),
      focused_style_(focused_style), gutter_renderer_(std::move(options.gutter)) {}

std::string TextArea::value() const {
    std::string out;
    for (std::size_t i = 0; i < lines_.size(); ++i) {
        if (i > 0) {
            out.push_back('\n');
        }
        out += lines_[i];
    }
    return out;
}

std::string_view TextArea::field_value() const {
    field_value_cache_ = value();
    return field_value_cache_;
}

void TextArea::set_value(std::string value) {
    lines_ = split_lines(value);
    cursor_row_ = 0;
    cursor_col_ = 0;
    selection_anchor_row_ = 0;
    selection_anchor_col_ = 0;
    scroll_x_ = 0;
    scroll_y_ = 0;
    clamp_cursor();
    mark_dirty();
}

void TextArea::set_title(std::string title) {
    title_ = std::move(title);
    mark_dirty();
}

void TextArea::set_placeholder(std::string placeholder) {
    placeholder_ = std::move(placeholder);
    mark_dirty();
}

void TextArea::apply_stylesheet(const StyleResolver& styles) {
    apply_multiline_text_input_stylesheet(*this, *this, styles);
}

void TextArea::set_line_numbers(bool enabled) {
    line_numbers_ = enabled;
    ensure_cursor_visible();
    mark_dirty();
}

void TextArea::set_status_bar(bool enabled) {
    status_bar_ = enabled;
    ensure_cursor_visible();
    mark_dirty();
}

void TextArea::set_gutter_renderer(GutterRenderer renderer) {
    gutter_renderer_ = std::move(renderer);
    ensure_cursor_visible();
    mark_dirty();
}

void TextArea::set_gutter_width(int width) {
    gutter_width_ = std::max(0, width);
    ensure_cursor_visible();
    mark_dirty();
}

void TextArea::set_min_width(int width) {
    min_width_ = std::max(8, width);
    mark_layout_dirty();
}

void TextArea::set_min_height(int height) {
    min_height_ = std::max(3, height);
    mark_layout_dirty();
}

void TextArea::set_on_change(std::function<void(const std::string&)> callback) { on_change_ = std::move(callback); }

Size TextArea::preferred_size() const { return {min_width_, min_height_}; }

void TextArea::layout(Rect bounds) {
    bounds_ = bounds;
    ensure_cursor_visible();
}

int TextArea::gutter_width() const {
    if (!line_numbers_) {
        return 0;
    }

    if (gutter_width_ > 0) {
        return gutter_width_;
    }

    int width = digit_count(line_count()) + 2;
    if (!gutter_renderer_ || lines_.empty()) {
        return width;
    }

    const int last = line_count() - 1;
    const int samples[] = {0, cursor_row_, last};
    for (int index : samples) {
        if (index < 0 || index > last) {
            continue;
        }
        const int cell_width = text_display_width(render_gutter_line(index).text) + 1;
        width = std::max(width, cell_width);
    }
    return width;
}

GutterLine TextArea::make_gutter_line(int index) const {
    GutterLine line{};
    line.index = index;
    line.number = index + 1;
    line.cursor_index = cursor_row_;
    line.line_count = line_count();
    line.current = index == cursor_row_;
    if (index >= 0 && index < line_count()) {
        line.text = lines_[static_cast<std::size_t>(index)];
    }
    return line;
}

GutterCell TextArea::render_gutter_line(int index) const {
    const GutterLine line = make_gutter_line(index);
    if (gutter_renderer_) {
        return gutter_renderer_(line);
    }
    return absolute_gutter(line);
}

int TextArea::max_line_width() const {
    int width = 0;
    for (const std::string& line : lines_) {
        width = std::max(width, static_cast<int>(line.size()));
    }
    return width;
}

ScrollbarMetrics TextArea::scrollbar_metrics() const {
    int area_width = bounds_.width;
    int area_height = bounds_.height;
    if (status_bar_ && area_height > 0) {
        --area_height;
    }

    return compute_scrollbar_metrics(area_width, area_height, max_line_width(), line_count(), scrollbars_.config);
}

int TextArea::content_width() const { return std::max(0, scrollbar_metrics().viewport_width - gutter_width()); }

int TextArea::content_height() const { return scrollbar_metrics().viewport_height; }

void TextArea::clamp_cursor() {
    if (lines_.empty()) {
        lines_.emplace_back();
    }
    cursor_row_ = std::clamp(cursor_row_, 0, line_count() - 1);
    const int line_len = static_cast<int>(lines_[static_cast<std::size_t>(cursor_row_)].size());
    cursor_col_ = std::clamp(cursor_col_, 0, line_len);
}

void TextArea::ensure_cursor_visible() {
    clamp_cursor();

    const int height = content_height();
    const int width = content_width();
    if (height <= 0 || width <= 0) {
        return;
    }

    if (cursor_row_ < scroll_y_) {
        scroll_y_ = cursor_row_;
    }
    if (cursor_row_ >= scroll_y_ + height) {
        scroll_y_ = cursor_row_ - height + 1;
    }

    if (cursor_col_ < scroll_x_) {
        scroll_x_ = cursor_col_;
    }
    if (cursor_col_ >= scroll_x_ + width) {
        scroll_x_ = cursor_col_ - width + 1;
    }

    const int max_scroll_y = std::max(0, line_count() - height);
    scroll_y_ = std::clamp(scroll_y_, 0, max_scroll_y);

    const int max_scroll_x = std::max(0, max_line_width() - width);
    scroll_x_ = std::clamp(scroll_x_, 0, max_scroll_x);
}

void TextArea::notify_change() {
    if (on_change_) {
        on_change_(value());
    }
}

void TextArea::insert_char(char ch) {
    if (has_selection()) {
        delete_selection();
    }

    clamp_cursor();
    auto& line = lines_[static_cast<std::size_t>(cursor_row_)];
    line.insert(static_cast<std::size_t>(cursor_col_), 1, ch);
    ++cursor_col_;
    clear_selection();
    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

void TextArea::insert_newline() {
    if (has_selection()) {
        delete_selection();
    }

    clamp_cursor();
    auto& line = lines_[static_cast<std::size_t>(cursor_row_)];
    std::string rest = line.substr(static_cast<std::size_t>(cursor_col_));
    line.erase(static_cast<std::size_t>(cursor_col_));
    lines_.insert(lines_.begin() + cursor_row_ + 1, std::move(rest));
    ++cursor_row_;
    cursor_col_ = 0;
    clear_selection();
    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

void TextArea::delete_before_cursor() {
    clamp_cursor();
    if (cursor_col_ > 0) {
        auto& line = lines_[static_cast<std::size_t>(cursor_row_)];
        line.erase(static_cast<std::size_t>(cursor_col_ - 1), 1);
        --cursor_col_;
    } else if (cursor_row_ > 0) {
        const std::string merged = std::move(lines_[static_cast<std::size_t>(cursor_row_)]);
        lines_.erase(lines_.begin() + cursor_row_);
        --cursor_row_;
        cursor_col_ = static_cast<int>(lines_[static_cast<std::size_t>(cursor_row_)].size());
        lines_[static_cast<std::size_t>(cursor_row_)] += merged;
    } else {
        return;
    }

    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

void TextArea::delete_at_cursor() {
    clamp_cursor();
    auto& line = lines_[static_cast<std::size_t>(cursor_row_)];
    if (cursor_col_ < static_cast<int>(line.size())) {
        line.erase(static_cast<std::size_t>(cursor_col_), 1);
    } else if (cursor_row_ + 1 < line_count()) {
        line += lines_[static_cast<std::size_t>(cursor_row_ + 1)];
        lines_.erase(lines_.begin() + cursor_row_ + 1);
    } else {
        return;
    }

    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

void TextArea::set_cursor(int row, int col, bool extend_selection) {
    if (row < 0) {
        cursor_row_ = 0;
        cursor_col_ = 0;
    } else if (row >= line_count()) {
        cursor_row_ = line_count() - 1;
        cursor_col_ = static_cast<int>(lines_[static_cast<std::size_t>(cursor_row_)].size());
    } else {
        cursor_row_ = row;
        cursor_col_ = col;
        clamp_cursor();
    }

    if (!extend_selection) {
        selection_anchor_row_ = cursor_row_;
        selection_anchor_col_ = cursor_col_;
    }

    ensure_cursor_visible();
    mark_dirty();
}

namespace {

int compare_positions(int row_a, int col_a, int row_b, int col_b) {
    if (row_a != row_b) {
        return row_a < row_b ? -1 : 1;
    }
    if (col_a != col_b) {
        return col_a < col_b ? -1 : 1;
    }
    return 0;
}

} // namespace

bool TextArea::has_selection() const {
    return selection_anchor_row_ != cursor_row_ || selection_anchor_col_ != cursor_col_;
}

bool TextArea::is_selected(int row, int col) const {
    if (!has_selection()) {
        return false;
    }

    const int start_cmp = compare_positions(selection_anchor_row_, selection_anchor_col_, cursor_row_, cursor_col_);
    const int row_cmp_anchor = compare_positions(row, col, selection_anchor_row_, selection_anchor_col_);
    const int row_cmp_cursor = compare_positions(row, col, cursor_row_, cursor_col_);
    if (start_cmp < 0) {
        return row_cmp_anchor >= 0 && row_cmp_cursor < 0;
    }
    return row_cmp_cursor >= 0 && row_cmp_anchor < 0;
}

void TextArea::clear_selection() {
    selection_anchor_row_ = cursor_row_;
    selection_anchor_col_ = cursor_col_;
}

void TextArea::select_all() {
    selection_anchor_row_ = 0;
    selection_anchor_col_ = 0;
    cursor_row_ = line_count() - 1;
    cursor_col_ = static_cast<int>(lines_.back().size());
    ensure_cursor_visible();
    mark_dirty();
}

std::string TextArea::selected_text() const {
    if (!has_selection()) {
        return {};
    }

    const bool forward = compare_positions(selection_anchor_row_, selection_anchor_col_, cursor_row_, cursor_col_) <= 0;
    const int start_row = forward ? selection_anchor_row_ : cursor_row_;
    const int start_col = forward ? selection_anchor_col_ : cursor_col_;
    const int end_row = forward ? cursor_row_ : selection_anchor_row_;
    const int end_col = forward ? cursor_col_ : selection_anchor_col_;

    std::string out;
    for (int row = start_row; row <= end_row; ++row) {
        if (row > start_row) {
            out.push_back('\n');
        }
        const std::string& line = lines_[static_cast<std::size_t>(row)];
        const int from = row == start_row ? start_col : 0;
        const int to = row == end_row ? end_col : static_cast<int>(line.size());
        if (to > from) {
            out.append(line.substr(static_cast<std::size_t>(from), static_cast<std::size_t>(to - from)));
        }
    }
    return out;
}

void TextArea::delete_selection() {
    if (!has_selection()) {
        return;
    }

    const bool forward = compare_positions(selection_anchor_row_, selection_anchor_col_, cursor_row_, cursor_col_) <= 0;
    const int start_row = forward ? selection_anchor_row_ : cursor_row_;
    const int start_col = forward ? selection_anchor_col_ : cursor_col_;
    const int end_row = forward ? cursor_row_ : selection_anchor_row_;
    const int end_col = forward ? cursor_col_ : selection_anchor_col_;

    if (start_row == end_row) {
        auto& line = lines_[static_cast<std::size_t>(start_row)];
        line.erase(static_cast<std::size_t>(start_col), static_cast<std::size_t>(end_col - start_col));
    } else {
        std::string merged = lines_[static_cast<std::size_t>(start_row)].substr(0, static_cast<std::size_t>(start_col));
        merged += lines_[static_cast<std::size_t>(end_row)].substr(static_cast<std::size_t>(end_col));
        lines_.erase(lines_.begin() + start_row + 1, lines_.begin() + end_row + 1);
        lines_[static_cast<std::size_t>(start_row)] = std::move(merged);
    }

    cursor_row_ = start_row;
    cursor_col_ = start_col;
    clear_selection();
    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

void TextArea::delete_word_before_cursor() {
    if (has_selection()) {
        delete_selection();
        return;
    }

    clamp_cursor();
    auto& line = lines_[static_cast<std::size_t>(cursor_row_)];
    if (cursor_col_ > 0) {
        const std::size_t start = text_edit::previous_word_boundary(line, static_cast<std::size_t>(cursor_col_));
        line.erase(start, static_cast<std::size_t>(cursor_col_) - start);
        cursor_col_ = static_cast<int>(start);
    } else if (cursor_row_ > 0) {
        delete_before_cursor();
        return;
    } else {
        return;
    }

    clear_selection();
    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

void TextArea::delete_word_after_cursor() {
    if (has_selection()) {
        delete_selection();
        return;
    }

    clamp_cursor();
    auto& line = lines_[static_cast<std::size_t>(cursor_row_)];
    if (cursor_col_ < static_cast<int>(line.size())) {
        const std::size_t end = text_edit::next_word_boundary(line, static_cast<std::size_t>(cursor_col_));
        line.erase(static_cast<std::size_t>(cursor_col_), end - static_cast<std::size_t>(cursor_col_));
    } else if (cursor_row_ + 1 < line_count()) {
        delete_at_cursor();
        return;
    } else {
        return;
    }

    clear_selection();
    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

void TextArea::delete_line_before_cursor() {
    if (cursor_col_ == 0 && cursor_row_ == 0) {
        return;
    }

    if (cursor_col_ > 0) {
        auto& line = lines_[static_cast<std::size_t>(cursor_row_)];
        line.erase(0, static_cast<std::size_t>(cursor_col_));
        cursor_col_ = 0;
    } else if (cursor_row_ > 0) {
        const std::string merged = std::move(lines_[static_cast<std::size_t>(cursor_row_)]);
        lines_.erase(lines_.begin() + cursor_row_);
        --cursor_row_;
        cursor_col_ = static_cast<int>(lines_[static_cast<std::size_t>(cursor_row_)].size());
        lines_[static_cast<std::size_t>(cursor_row_)] += merged;
    }

    clear_selection();
    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

void TextArea::delete_line_after_cursor() {
    clamp_cursor();
    auto& line = lines_[static_cast<std::size_t>(cursor_row_)];
    if (cursor_col_ < static_cast<int>(line.size())) {
        line.erase(static_cast<std::size_t>(cursor_col_));
    } else if (cursor_row_ + 1 < line_count()) {
        line += lines_[static_cast<std::size_t>(cursor_row_ + 1)];
        lines_.erase(lines_.begin() + cursor_row_ + 1);
    } else {
        return;
    }

    clear_selection();
    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

void TextArea::delete_current_line() {
    if (lines_.size() == 1) {
        lines_[0].clear();
        cursor_col_ = 0;
    } else {
        lines_.erase(lines_.begin() + cursor_row_);
        if (cursor_row_ >= line_count()) {
            cursor_row_ = line_count() - 1;
        }
        cursor_col_ = std::min(cursor_col_, static_cast<int>(lines_[static_cast<std::size_t>(cursor_row_)].size()));
    }

    clear_selection();
    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

void TextArea::insert_text(std::string_view text) {
    if (text.empty()) {
        return;
    }

    if (has_selection()) {
        delete_selection();
    }

    for (std::size_t index = 0; index < text.size();) {
        const unsigned char byte = static_cast<unsigned char>(text[index]);
        if (byte == '\r') {
            ++index;
            continue;
        }
        if (byte == '\n') {
            insert_newline();
            ++index;
            continue;
        }
        if (byte == '\t') {
            insert_char('\t');
            ++index;
            continue;
        }
        if (byte < 32) {
            ++index;
            continue;
        }

        const std::size_t length = utf8_char_length(text, index);
        std::string& line = lines_[static_cast<std::size_t>(cursor_row_)];
        line.insert(static_cast<std::size_t>(cursor_col_), text.substr(index, length));
        cursor_col_ += static_cast<int>(length);
        index += length;
    }

    selection_anchor_row_ = cursor_row_;
    selection_anchor_col_ = cursor_col_;
    ensure_cursor_visible();
    mark_dirty();
    notify_change();
}

void TextArea::copy_selection() {
    if (!has_selection()) {
        return;
    }
    clipboard::set(selected_text());
}

void TextArea::cut_selection() {
    if (!has_selection()) {
        return;
    }
    clipboard::set(selected_text());
    delete_selection();
}

void TextArea::paste_from_clipboard() { insert_text(clipboard::get()); }

bool TextArea::handle_shortcut(const KeyPress& key) {
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
    case 'c': copy_selection(); return true;
    case 'x': cut_selection(); return true;
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

Point TextArea::to_local(Point terminal) const { return {terminal.x - bounds_.x, terminal.y - bounds_.y}; }

int TextArea::byte_col_at_display(int row, int display_col, CaretAffinity affinity) const {
    if (row < 0 || row >= line_count()) {
        return 0;
    }
    return static_cast<int>(text_caret_index_at_column(lines_[static_cast<std::size_t>(row)], display_col, affinity));
}

bool TextArea::handle_mouse(const MouseEvent& mouse) {
    if (mouse.action == MouseAction::WheelUp) {
        if (!contains_point(mouse.position)) {
            return false;
        }
        scroll_y_ = std::max(0, scroll_y_ - 3);
        mark_dirty();
        return true;
    }
    if (mouse.action == MouseAction::WheelDown) {
        if (!contains_point(mouse.position)) {
            return false;
        }
        const int max_scroll = std::max(0, line_count() - content_height());
        scroll_y_ = std::min(max_scroll, scroll_y_ + 3);
        mark_dirty();
        return true;
    }

    if (mouse.action == MouseAction::Release) {
        if (selecting_with_mouse_) {
            const Point local = to_local(mouse.position);
            if (!(status_bar_ && local.y >= content_height())) {
                const int row = scroll_y_ + local.y;
                const int display_col = scroll_x_ + (local.x - gutter_width());
                if (row != mouse_press_row_ || display_col != mouse_press_display_col_) {
                    set_cursor(row, byte_col_at_display(row, display_col, CaretAffinity::After), true);
                } else {
                    set_cursor(row, mouse_press_col_, false);
                }
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

        const Point local = to_local(mouse.position);
        if (status_bar_ && local.y >= content_height()) {
            return true;
        }

        const int row = scroll_y_ + local.y;
        const int display_col = scroll_x_ + (local.x - gutter_width());
        set_cursor(row, byte_col_at_display(row, display_col, CaretAffinity::After), true);
        selecting_with_mouse_ = true;
        return true;
    }

    if (mouse.action == MouseAction::Press || mouse.action == MouseAction::Click) {
        if (!contains_point(mouse.position)) {
            return false;
        }

        const Point local = to_local(mouse.position);
        if (status_bar_ && local.y >= content_height()) {
            return true;
        }

        const int row = scroll_y_ + local.y;
        const int display_col = scroll_x_ + (local.x - gutter_width());
        mouse_press_row_ = row;
        mouse_press_display_col_ = display_col;
        mouse_press_col_ = byte_col_at_display(row, display_col, CaretAffinity::Before);
        set_cursor(row, mouse_press_col_, false);
        selecting_with_mouse_ = true;
        return true;
    }

    return false;
}

void TextArea::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    const bool focused = is_focused();
    const StyleResolver& styles = ctx.styles();
    const Style normal_style = styles.text(*this, style_);
    const Style focused_style = styles.focused(*this, focused_style_);
    const Style& text_style = focused ? focused_style : normal_style;

    Style number_style = text_style;
    number_style.dim = true;
    number_style.bold = false;
    number_style.reverse = false;

    Style placeholder_style = text_style;
    placeholder_style.dim = true;

    const int gutter = gutter_width();
    const int width = content_width();
    const int height = content_height();

    if (height > 0 && bounds_.width > 0) {
        canvas.fill_rect({0, 0, bounds_.width, height}, ' ', text_style);
    }

    const bool empty = lines_.size() == 1 && lines_[0].empty();
    if (empty && !focused && !placeholder_.empty() && height > 0 && width > 0) {
        if (gutter > 0) {
            const GutterCell cell = render_gutter_line(0);
            Style gutter_style = style_customized(cell.style) ? cell.style : number_style;
            canvas.draw_text({0, 0}, fit_gutter_text(cell.text, gutter), gutter_style);
        }
        const std::size_t bytes = text_byte_length_for_width(placeholder_, width);
        canvas.draw_text({gutter, 0}, placeholder_.substr(0, bytes), placeholder_style);
    } else {
        for (int row = 0; row < height; ++row) {
            const int index = scroll_y_ + row;
            if (index < 0 || index >= line_count()) {
                continue;
            }

            if (gutter > 0) {
                const GutterCell cell = render_gutter_line(index);
                Style gutter_style = style_customized(cell.style) ? cell.style : number_style;
                canvas.draw_text({0, row}, fit_gutter_text(cell.text, gutter), gutter_style);
            }

            const std::string& line = lines_[static_cast<std::size_t>(index)];
            for (int col = 0; col < width; ++col) {
                const int char_index = scroll_x_ + col;
                const char ch = char_index >= 0 && char_index < static_cast<int>(line.size())
                                    ? line[static_cast<std::size_t>(char_index)]
                                    : ' ';
                const bool at_cursor = focused && index == cursor_row_ && char_index == cursor_col_;
                const bool selected = is_selected(index, char_index);

                Style glyph_style = text_style;
                if (selected) {
                    glyph_style.reverse = !glyph_style.reverse;
                }
                if (at_cursor && !has_selection()) {
                    glyph_style.reverse = !glyph_style.reverse;
                }
                canvas.draw_char({gutter + col, row}, ch, glyph_style);
            }
        }
    }

    const auto metrics = scrollbar_metrics();
    const int editor_height = metrics.viewport_height + (metrics.show_horizontal ? 1 : 0);
    canvas.with_clip({{0, 0}, {bounds_.width, editor_height}}, [&](Canvas& editor) {
        paint_scrollbars(editor, scrollbars_, scroll_x_, scroll_y_, max_line_width(), line_count());
    });

    if (!status_bar_ || bounds_.height <= 0) {
        return;
    }

    Style bar_style{};
    bar_style.foreground = Color::Black;
    bar_style.background = focused ? Color::Cyan : Color::White;

    const int bar_y = std::max(0, bounds_.height - 1);
    canvas.fill_rect({0, bar_y, bounds_.width, 1}, ' ', bar_style);

    char left[64];
    if (!title_.empty()) {
        std::snprintf(left, sizeof(left), "[%d] %s", line_count(), title_.c_str());
    } else {
        std::snprintf(left, sizeof(left), "[%d]", line_count());
    }

    char right[32];
    std::snprintf(right, sizeof(right), "(%d,%d)", cursor_row_ + 1, cursor_col_ + 1);

    const int left_width = std::min(text_display_width(left), bounds_.width);
    canvas.draw_text({0, bar_y}, std::string(left).substr(0, static_cast<std::size_t>(left_width)), bar_style);

    const int right_width = text_display_width(right);
    const int right_x = bounds_.width - right_width;
    if (right_x > left_width) {
        canvas.draw_text({right_x, bar_y}, right, bar_style);
    }
}

bool TextArea::handle_event(const Event& event) {
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
        if (key->ctrl) {
            delete_current_line();
        } else if (key->alt) {
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
    case Key::Left:
        if (cursor_col_ > 0) {
            set_cursor(cursor_row_, cursor_col_ - 1, key->shift);
        } else if (cursor_row_ > 0) {
            const int prev_len = static_cast<int>(lines_[static_cast<std::size_t>(cursor_row_ - 1)].size());
            set_cursor(cursor_row_ - 1, prev_len, key->shift);
        }
        return true;
    case Key::Right: {
        const int line_len = static_cast<int>(lines_[static_cast<std::size_t>(cursor_row_)].size());
        if (cursor_col_ < line_len) {
            set_cursor(cursor_row_, cursor_col_ + 1, key->shift);
        } else if (cursor_row_ + 1 < line_count()) {
            set_cursor(cursor_row_ + 1, 0, key->shift);
        }
        return true;
    }
    case Key::Up:
        if (cursor_row_ > 0) {
            set_cursor(cursor_row_ - 1, cursor_col_, key->shift);
        }
        return true;
    case Key::Down:
        if (cursor_row_ + 1 < line_count()) {
            set_cursor(cursor_row_ + 1, cursor_col_, key->shift);
        }
        return true;
    case Key::Home: set_cursor(cursor_row_, 0, key->shift); return true;
    case Key::End:
        set_cursor(cursor_row_, static_cast<int>(lines_[static_cast<std::size_t>(cursor_row_)].size()), key->shift);
        return true;
    case Key::PageUp: set_cursor(cursor_row_ - std::max(1, content_height()), cursor_col_, key->shift); return true;
    case Key::PageDown: set_cursor(cursor_row_ + std::max(1, content_height()), cursor_col_, key->shift); return true;
    case Key::Escape:
        if (has_selection()) {
            clear_selection();
            mark_dirty();
            return true;
        }
        return false;
    case Key::Enter: insert_newline(); return true;
    case Key::Tab: insert_char('\t'); return true;
    default: break;
    }

    if (key->character == '\n' || key->character == '\r') {
        insert_newline();
        return true;
    }

    if (key->character == '\t') {
        insert_char('\t');
        return true;
    }

    const std::string insert_text_value = keypress_insert_text(*key);
    if (!insert_text_value.empty() && allows_text_insert_modifiers(*key)) {
        insert_text(insert_text_value);
        return true;
    }

    return false;
}

} // namespace tuinator
