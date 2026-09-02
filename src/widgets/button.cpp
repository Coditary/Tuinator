#include <tuinator/widgets/button.hpp>

#include <tuinator/core/event.hpp>

#include <string>
#include <variant>

namespace tuinator {

Button::Button(std::string label, std::function<void()> on_click, Style style)
    : label_(std::move(label)),
      on_click_(std::move(on_click)),
      style_(style) {
    focused_style_ = style_;
    focused_style_.foreground = Color::Black;
    focused_style_.background = Color::Cyan;
    focused_style_.bold = true;
    focused_style_.reverse = false;
}

void Button::set_label(std::string label) {
    label_ = std::move(label);
    mark_dirty();
}

Size Button::preferred_size() const {
    return {
        static_cast<int>(label_.size()) + 2,
        1,
    };
}

void Button::paint(Canvas& canvas) const {
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    const std::string text = "[" + label_ + "]";
    const Style& active_style = is_focused() ? focused_style_ : style_;
    canvas.draw_text({0, 0}, text, active_style);
}

bool Button::handle_event(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (mouse->action != MouseAction::Click && mouse->action != MouseAction::Release) {
            return false;
        }

        if (!contains_point(mouse->position)) {
            return false;
        }

        if (on_click_) {
            on_click_();
        }
        mark_dirty();
        return true;
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key) {
        return false;
    }

    if (!is_focused()) {
        return false;
    }

    if (key->key == Key::Enter || key->character == ' ' || key->character == '\r') {
        if (on_click_) {
            on_click_();
        }
        mark_dirty();
        return true;
    }

    return false;
}

} // namespace tuinator
