#include <tuinator/core/event.hpp>
#include <tuinator/render/text.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/controls/text_input.hpp>

#include <algorithm>
#include <variant>

namespace tuinator {

namespace {

std::size_t utf8_char_length(std::string_view text, std::size_t index) {
    if (index >= text.size()) {
        return 0;
    }

    const unsigned char lead = static_cast<unsigned char>(text[index]);
    if (lead < 0x80) {
        return 1;
    }
    if ((lead & 0xE0) == 0xC0 && index + 1 < text.size()) {
        return 2;
    }
    if ((lead & 0xF0) == 0xE0 && index + 2 < text.size()) {
        return 3;
    }
    if ((lead & 0xF8) == 0xF0 && index + 3 < text.size()) {
        return 4;
    }
    return 1;
}

int display_width_before(std::string_view text, std::size_t byte_index) {
    return text_display_width(text.substr(0, byte_index));
}

std::size_t byte_index_at_display_column(std::string_view text, int column) {
    if (column <= 0) {
        return 0;
    }
    return text_byte_length_for_width(text, column);
}

} // namespace

TextInput::TextInput(TextInputOptions options, Style style, Style focused_style)
    : placeholder_(std::move(options.placeholder)), min_width_(std::max(1, options.min_width)), style_(style),
      focused_style_(focused_style) {}

void TextInput::set_options(TextInputOptions options) {
    min_width_ = std::max(1, options.min_width);
    placeholder_ = std::move(options.placeholder);
    mark_dirty();
}

void TextInput::apply_stylesheet(const StyleResolver& styles) {
    apply_text_input_stylesheet(*this, *this, styles);
}

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

Size TextInput::preferred_size() const { return {min_width_ + 2, 1}; }

void TextInput::layout(Rect bounds) {
    bounds_ = bounds;
    ensure_cursor_visible();
}

int TextInput::inner_width() const { return std::max(0, bounds_.width - 2); }

void TextInput::ensure_cursor_visible() {
    const int width = inner_width();
    if (width <= 0) {
        scroll_x_ = 0;
        return;
    }

    const int cursor_col = display_width_before(value_, cursor_);
    const int right_edge = cursor_ == value_.size() ? text_display_width(value_) : cursor_col;

    if (cursor_col < scroll_x_) {
        scroll_x_ = cursor_col;
    }
    if (right_edge >= scroll_x_ + width) {
        scroll_x_ = right_edge - width + 1;
    }

    const int content_width = text_display_width(value_) + (cursor_ == value_.size() ? 1 : 0);
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
    const Style& active_style = focused ? focused_style : normal_style;
    Style placeholder_style{};
    placeholder_style.foreground = active_style.foreground;
    placeholder_style.background = active_style.background;
    placeholder_style.dim = true;

    Style selection_style = active_style;
    selection_style.reverse = true;

    paint_bounds_background(ctx, active_style);

    const int width = inner_width();
    canvas.draw_text({0, 0}, "[", active_style);

    if (value_.empty() && !focused && !placeholder_.empty()) {
        const std::size_t bytes = text_byte_length_for_width(placeholder_, width);
        canvas.draw_text({1, 0}, placeholder_.substr(0, bytes), placeholder_style);
    } else {
        const auto [sel_start, sel_end] = selection_range();
        const std::size_t start_byte = byte_index_at_display_column(value_, scroll_x_);
        const std::size_t visible_bytes = text_byte_length_for_width(value_.substr(start_byte), width) + start_byte;
        const std::size_t paint_end = std::min(visible_bytes, value_.size());

        for (std::size_t index = start_byte; index < paint_end;) {
            const std::size_t char_len = utf8_char_length(value_, index);
            const int col = 1 + display_width_before(value_, index) - scroll_x_;
            if (col >= 1 + width) {
                break;
            }

            const bool selected = index >= sel_start && index < sel_end;
            const bool at_cursor = focused && index == cursor_ && !has_selection();

            Style glyph_style = selected ? selection_style : active_style;
            if (at_cursor) {
                glyph_style.reverse = !glyph_style.reverse;
            }

            canvas.draw_text({col, 0}, value_.substr(index, char_len), glyph_style);
            index += char_len;
        }

        if (focused && cursor_ == value_.size() && !has_selection()) {
            const int col = 1 + display_width_before(value_, cursor_) - scroll_x_;
            if (col >= 1 && col < 1 + width) {
                canvas.draw_char({col, 0}, '_', active_style);
            }
        }
    }

    canvas.draw_text({bounds_.width - 1, 0}, "]", active_style);
}

bool TextInput::handle_event(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (mouse->action != MouseAction::Click && mouse->action != MouseAction::Release) {
            return false;
        }

        return contains_point(mouse->position);
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key || !is_focused()) {
        return false;
    }

    switch (key->key) {
    case Key::Backspace:
        if (has_selection()) {
            delete_selection();
        } else {
            delete_before_cursor();
        }
        return true;
    case Key::Delete:
        if (has_selection()) {
            delete_selection();
        } else {
            delete_at_cursor();
        }
        return true;
    case Key::Left: move_cursor(-1, false); return true;
    case Key::Right: move_cursor(1, false); return true;
    case Key::Home: set_cursor(0, false); return true;
    case Key::End: set_cursor(value_.size(), false); return true;
    case Key::Escape:
        if (has_selection()) {
            clear_selection();
            mark_dirty();
            return true;
        }
        return false;
    case Key::Enter:
        if (on_submit_) {
            on_submit_(value_);
        }
        return true;
    default: break;
    }

    if (key->character == 1) {
        select_all();
        return true;
    }

    if (key->character >= 32 && key->character <= 126) {
        if (has_selection()) {
            delete_selection();
        }
        insert_char(key->character);
        return true;
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

void TextInput::move_cursor(int delta, bool extend_selection) {
    if (delta < 0) {
        set_cursor(cursor_ > static_cast<std::size_t>(-delta) ? cursor_ - static_cast<std::size_t>(-delta) : 0,
                   extend_selection);
    } else {
        set_cursor(cursor_ + static_cast<std::size_t>(delta), extend_selection);
    }
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
    if (on_change_) {
        on_change_(value_);
    }
}

} // namespace tuinator
