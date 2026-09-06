#include <tuinator/core/event.hpp>
#include <tuinator/render/text.hpp>
#include <tuinator/widgets/controls/text_input.hpp>

#include <algorithm>
#include <variant>

namespace tuinator {

TextInput::TextInput(TextInputOptions options, Style style, Style focused_style)
    : placeholder_(std::move(options.placeholder)), min_width_(std::max(1, options.min_width)), style_(style),
      focused_style_(focused_style) {}

void TextInput::set_value(std::string value) {
    value_ = std::move(value);
    cursor_ = std::min(cursor_, value_.size());
    clear_selection();
    mark_dirty();
}

void TextInput::set_placeholder(std::string placeholder) {
    placeholder_ = std::move(placeholder);
    mark_dirty();
}

void TextInput::set_on_change(std::function<void(const std::string&)> callback) { on_change_ = std::move(callback); }

void TextInput::set_on_submit(std::function<void(const std::string&)> callback) { on_submit_ = std::move(callback); }

Size TextInput::preferred_size() const { return {min_width_ + 2, 1}; }

void TextInput::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    const bool focused = is_focused();
    const Style& active_style = focused ? focused_style_ : style_;
    Style placeholder_style{};
    placeholder_style.foreground = active_style.foreground;
    placeholder_style.background = active_style.background;
    placeholder_style.dim = true;

    Style selection_style = active_style;
    selection_style.reverse = true;

    const int inner_width = std::max(0, bounds_.width - 2);
    canvas.draw_text({0, 0}, "[", active_style);

    if (value_.empty() && !focused && !placeholder_.empty()) {
        const std::string visible = placeholder_.substr(0, static_cast<std::size_t>(inner_width));
        canvas.draw_text({1, 0}, visible, placeholder_style);
    } else {
        const auto [sel_start, sel_end] = selection_range();
        const int visible_chars = std::min(inner_width, static_cast<int>(value_.size()));

        for (int i = 0; i < visible_chars; ++i) {
            const std::size_t index = static_cast<std::size_t>(i);
            const char ch = value_[index];
            const bool selected = index >= sel_start && index < sel_end;
            const bool at_cursor = focused && index == cursor_ && !has_selection();

            Style glyph_style = selected ? selection_style : active_style;
            if (at_cursor) {
                glyph_style.reverse = !glyph_style.reverse;
            }

            canvas.draw_char({1 + i, 0}, ch, glyph_style);
        }

        if (focused && cursor_ == value_.size() && static_cast<int>(value_.size()) < inner_width) {
            canvas.draw_char({1 + static_cast<int>(value_.size()), 0}, '_', active_style);
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
    if (static_cast<int>(value_.size()) >= min_width_) {
        return;
    }

    value_.insert(cursor_, 1, ch);
    ++cursor_;
    selection_anchor_ = cursor_;
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
    mark_dirty();
    notify_change();
}

void TextInput::delete_at_cursor() {
    if (cursor_ >= value_.size()) {
        return;
    }

    value_.erase(cursor_, 1);
    selection_anchor_ = cursor_;
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
    mark_dirty();
}

void TextInput::select_all() {
    selection_anchor_ = 0;
    cursor_ = value_.size();
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
