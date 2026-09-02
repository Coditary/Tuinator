#include <tuinator/widgets/controls/toggle.hpp>

#include <tuinator/core/event.hpp>

#include <string>
#include <variant>

namespace tuinator {

Toggle::Toggle(std::string label,
               bool checked,
               std::function<void(bool)> on_change,
               Style style,
               Style checked_style)
    : label_(std::move(label)),
      checked_(checked),
      on_change_(std::move(on_change)),
      style_(style) {
    if (checked_style.foreground == Color::Default
        && !checked_style.foreground_rgb.has_value()
        && checked_style.background == Color::Default
        && !checked_style.background_rgb.has_value()
        && !checked_style.bold
        && !checked_style.dim
        && !checked_style.reverse) {
        checked_style_ = style_;
        checked_style_.foreground = Color::Green;
    } else {
        checked_style_ = checked_style;
    }
    focused_style_ = style_;
    focused_style_.foreground = Color::Black;
    focused_style_.background = Color::Cyan;
    focused_style_.bold = true;
    focused_style_.reverse = false;
}

void Toggle::set_label(std::string label) {
    label_ = std::move(label);
    mark_dirty();
}

void Toggle::set_checked(bool checked) {
    if (checked_ == checked) {
        return;
    }

    checked_ = checked;
    mark_dirty();
}

Size Toggle::preferred_size() const {
    const std::string marker = checked_ ? "[X]" : "[ ]";
    return {
        static_cast<int>(marker.size() + 1 + label_.size()),
        1,
    };
}

void Toggle::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    const std::string marker = checked_ ? "[X]" : "[ ]";
    const std::string text = marker + " " + label_;

    Style active = style_;
    if (checked_) {
        active = checked_style_;
    }
    if (is_focused()) {
        active = focused_style_;
    }

    canvas.draw_text({0, 0}, text, active);
}

bool Toggle::handle_event(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (mouse->action != MouseAction::Click && mouse->action != MouseAction::Release) {
            return false;
        }

        if (!contains_point(mouse->position)) {
            return false;
        }

        toggle();
        return true;
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key || !is_focused()) {
        return false;
    }

    if (key->key == Key::Enter || key->character == ' ' || key->character == '\r') {
        toggle();
        return true;
    }

    return false;
}

void Toggle::toggle() {
    checked_ = !checked_;
    if (on_change_) {
        on_change_(checked_);
    }
    mark_dirty();
}

} // namespace tuinator
