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
    style.reverse = false;
    style.foreground_rgb = rgb(255, 255, 255);
    style.background_rgb = rgb(88, 78, 128);
    return style;
}

Style make_cursor_style(const Style& base, const Style& field) {
    Style style = with_field_background(base, field);
    style.reverse = false;
    style.bold = false;
    style.dim = false;
    style.foreground_rgb = rgb(28, 24, 40);
    style.background_rgb = rgb(196, 178, 255);
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

} // namespace

TextInput::TextInput(TextInputOptions options, Style style, Style focused_style)
    : placeholder_(std::move(options.placeholder)), min_width_(std::max(1, options.min_width)),
      password_(options.password), chrome_(options.chrome), min_content_lines_(std::max(1, options.min_content_lines)),
      max_content_lines_(std::max(min_content_lines_, options.max_content_lines)), prompt_(std::move(options.prompt)),
      style_(style), focused_style_(focused_style) {}

void TextInput::set_options(TextInputOptions options) {
    min_width_ = std::max(1, options.min_width);
    placeholder_ = std::move(options.placeholder);
    password_ = options.password;
    chrome_ = options.chrome;
    min_content_lines_ = std::max(1, options.min_content_lines);
    max_content_lines_ = std::max(min_content_lines_, options.max_content_lines);
    prompt_ = std::move(options.prompt);
    mark_dirty();
    update_layout_for_lines();
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

int TextInput::content_line_count() const {
    return std::clamp(count_lines(value_), min_content_lines_, max_content_lines_);
}

int TextInput::framed_height_for_lines(int lines) const { return std::max(3, lines + 2); }

Size TextInput::preferred_size() const {
    if (chrome_ == TextInputChrome::Framed) {
        const int prompt_cols = text_display_width(prompt_);
        return {min_width_ + 4 + prompt_cols, framed_height_for_lines(content_line_count())};
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
    ensure_cursor_visible();
}

int TextInput::text_row() const { return chrome_ == TextInputChrome::Framed ? 1 : 0; }

int TextInput::text_col_start() const {
    if (chrome_ == TextInputChrome::Framed) {
        return 2 + text_display_width(prompt_);
    }
    return 1;
}

int TextInput::inner_width() const {
    if (chrome_ == TextInputChrome::Framed) {
        return std::max(0, bounds_.width - 4 - text_display_width(prompt_));
    }
    return std::max(0, bounds_.width - 2);
}

int TextInput::display_column_at_local(int local_x) const {
    const int inner = inner_width();
    const int start_col = text_col_start();
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

    const int row = std::max(0, local_y - text_row());
    int line = scroll_y_ + row;
    const int lines = count_lines(value_);
    line = std::clamp(line, 0, lines - 1);
    return byte_index_at_line_col(value_, line, display_col);
}

bool TextInput::handle_mouse(const MouseEvent& mouse) {
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
        if (fill_style.background == Color::Default) {
            fill_style.background_rgb = rgb(42, 38, 58);
        }
        placeholder_style.background = fill_style.background;
        placeholder_style.background_rgb = fill_style.background_rgb;
        text_style.background = fill_style.background;
        text_style.background_rgb = fill_style.background_rgb;
        placeholder_style.foreground_rgb = rgb(120, 115, 140);
    }

    if (chrome_ == TextInputChrome::Framed) {
        Style border_style = text_style;
        if (focused) {
            border_style.foreground_rgb = rgb(150, 130, 220);
        } else {
            border_style.foreground_rgb = rgb(90, 82, 118);
            border_style.dim = true;
        }
        border_style.background = fill_style.background;
        border_style.background_rgb = fill_style.background_rgb;

        if (bounds_.width >= 2 && bounds_.height >= 2) {
            canvas.fill_rect({1, 1, bounds_.width - 2, bounds_.height - 2}, ' ', fill_style);
            canvas.draw_box({0, 0, bounds_.width, bounds_.height}, border_style, unicode_rounded_border_glyphs());
        }

        if (!prompt_.empty() && row < bounds_.height) {
            Style prompt_style = text_style;
            prompt_style.bold = true;
            prompt_style.foreground_rgb = focused ? rgb(180, 160, 255) : rgb(130, 120, 170);
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
        for (int visible_row = 0; visible_row < visible_lines; ++visible_row) {
            const int line_idx = scroll_y_ + visible_row;
            const int paint_row = row + visible_row;
            if (paint_row >= bounds_.height) {
                break;
            }
            if (line_idx >= count_lines(value_)) {
                break;
            }

            const std::size_t line_start_pos = line_start(value_, line_idx);
            const std::size_t line_end_pos = line_end(value_, line_idx);
            const std::string line_text = value_.substr(line_start_pos, line_end_pos - line_start_pos);
            const int line_scroll = (line_idx == cursor_line_idx) ? scroll_x_ : 0;
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
                const int col = col_start + display_width_before(line_text, cursor_ - line_start_pos) - line_scroll;
                if (col >= col_start && col < col_start + width) {
                    Style cursor_style = make_cursor_style(text_style, text_style);
                    canvas.draw_char({col, paint_row}, cursor_block_char(password_), cursor_style);
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
