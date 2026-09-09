#include <tuinator/core/event.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/controls/slider.hpp>

#include <algorithm>
#include <string>
#include <variant>

namespace tuinator {

Slider::Slider(int min_value, int max_value, int value, std::function<void(int)> on_change, Style style, int min_width)
    : min_value_(min_value), max_value_(max_value), value_(value), min_width_(min_width),
      on_change_(std::move(on_change)), style_(style) {
    focused_style_ = style_;
    focused_style_.reverse = true;
    set_value_internal(value, false);
}

void Slider::set_value(int value) { set_value_internal(value, true); }

void Slider::set_min_width(int min_width) {
    min_width_ = std::max(1, min_width);
    mark_layout_dirty();
}

void Slider::apply_stylesheet(const StyleResolver& styles) {
    apply_value_control_stylesheet(*this, *this, styles);
}

void Slider::set_value_internal(int value, bool notify) {
    const int clamped = std::clamp(value, min_value_, max_value_);
    if (value_ == clamped) {
        return;
    }

    value_ = clamped;
    mark_dirty();

    if (notify && on_change_) {
        on_change_(value_);
    }
}

Size Slider::preferred_size() const { return {min_width_, 1}; }

void Slider::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    paint_bounds_background(ctx, style_);

    const int width = std::max(3, bounds_.width);
    const int range = std::max(1, max_value_ - min_value_);
    const int thumb = std::clamp((value_ - min_value_) * (width - 1) / range, 0, width - 1);

    std::string track(static_cast<std::size_t>(width), '-');
    track[static_cast<std::size_t>(thumb)] = 'O';

    const StyleResolver& styles = ctx.styles();
    const Style style = styles.text(*this, style_);
    const Style focused = styles.focused(*this, focused_style_);
    const Style& active = is_focused() ? focused : style;
    canvas.draw_text({0, 0}, track, active);
}

bool Slider::handle_event(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (mouse->action != MouseAction::Click && mouse->action != MouseAction::Release) {
            return false;
        }

        if (!contains_point(mouse->position)) {
            return false;
        }

        const Point local{mouse->position.x - bounds_.x, mouse->position.y - bounds_.y};
        const int width = std::max(1, bounds_.width);
        const int range = std::max(1, max_value_ - min_value_);
        const int thumb = std::clamp(local.x * range / std::max(1, width - 1), 0, range);
        set_value(min_value_ + thumb);
        return true;
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key || !is_focused()) {
        return false;
    }

    switch (key->key) {
    case Key::Left: set_value(value_ - 1); return true;
    case Key::Right: set_value(value_ + 1); return true;
    case Key::Home: set_value(min_value_); return true;
    case Key::End: set_value(max_value_); return true;
    default: break;
    }

    return false;
}

} // namespace tuinator
