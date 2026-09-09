#include <tuinator/core/event.hpp>
#include <tuinator/widgets/display/spinner.hpp>

#include <algorithm>
#include <string>
#include <variant>

namespace tuinator {

Spinner::Spinner(int min_value, int max_value, int value, int step, std::function<void(int)> on_change, Style style)
    : min_value_(min_value), max_value_(max_value), value_(std::clamp(value, min_value, max_value)),
      step_(std::max(1, step)), on_change_(std::move(on_change)), style_(style) {
    focused_style_ = style_;
    focused_style_.reverse = true;
}

void Spinner::set_value(int value) {
    const int clamped = std::clamp(value, min_value_, max_value_);
    if (value_ == clamped) {
        return;
    }

    value_ = clamped;
    mark_dirty();

    if (on_change_) {
        on_change_(value_);
    }
}

void Spinner::adjust(int delta) { set_value(value_ + delta * step_); }

Size Spinner::preferred_size() const { return {12, 1}; }

void Spinner::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    paint_bounds_background(ctx, style_);

    const std::string text = "< " + std::to_string(value_) + " >";
    const StyleResolver& styles = ctx.styles();
    const Style normal = styles.text(*this, style_);
    const Style focused = styles.focused(*this, focused_style_);
    const Style& active = is_focused() ? focused : normal;
    canvas.draw_text({0, 0}, text, active);
}

bool Spinner::handle_event(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (mouse->action != MouseAction::Click && mouse->action != MouseAction::Release) {
            return false;
        }

        if (!contains_point(mouse->position)) {
            return false;
        }

        const Point local{mouse->position.x - bounds_.x, mouse->position.y - bounds_.y};
        adjust(local.x < bounds_.width / 2 ? -1 : 1);
        return true;
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key || !is_focused()) {
        return false;
    }

    switch (key->key) {
    case Key::Up:
    case Key::Right: adjust(1); return true;
    case Key::Down:
    case Key::Left: adjust(-1); return true;
    case Key::PageUp: adjust(5); return true;
    case Key::PageDown: adjust(-5); return true;
    default: break;
    }

    return false;
}

} // namespace tuinator
