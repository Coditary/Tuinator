#include <tuinator/widgets/display/custom_paint.hpp>

namespace tuinator {

CustomPaint::CustomPaint() = default;

CustomPaint::CustomPaint(PaintCallback on_paint, Size preferred)
    : on_paint_(std::move(on_paint)), fixed_size_(preferred) {}

CustomPaint::CustomPaint(PaintCallback on_paint, SizeCallback preferred_size)
    : on_paint_(std::move(on_paint)), preferred_size_(std::move(preferred_size)) {}

void CustomPaint::set_on_paint(PaintCallback callback) {
    on_paint_ = std::move(callback);
    mark_dirty();
}

void CustomPaint::set_preferred_size(Size size) {
    fixed_size_ = size;
    preferred_size_ = nullptr;
    mark_layout_dirty();
}

void CustomPaint::set_preferred_size(SizeCallback callback) {
    preferred_size_ = std::move(callback);
    mark_layout_dirty();
}

void CustomPaint::set_background(Style background) {
    background_ = background;
    mark_dirty();
}

void CustomPaint::set_paint_children_first(bool children_first) {
    paint_children_first_ = children_first;
    mark_dirty();
}

void CustomPaint::set_needs_periodic_idle(bool enabled) {
    needs_periodic_idle_ = enabled;
}

void CustomPaint::apply_stylesheet(const StyleResolver& styles) {
    Widget::apply_stylesheet(styles);
    const WidgetOptions opts = styles.options(*this);
    if (opts.has("width")) {
        fixed_size_.width = opts.int_or("width", fixed_size_.width);
    }
    if (opts.has("height")) {
        fixed_size_.height = opts.int_or("height", fixed_size_.height);
    }
    mark_layout_dirty();
}

Size CustomPaint::preferred_size() const {
    if (preferred_size_) {
        return preferred_size_();
    }

    Size size = fixed_size_;
    for (const auto& child : children_) {
        const Size child_size = child->preferred_size();
        size.width = std::max(size.width, child_size.width);
        size.height = std::max(size.height, child_size.height);
    }
    return size;
}

void CustomPaint::layout(Rect bounds) {
    bounds_ = bounds;
    for (auto& child : children_) {
        child->layout(bounds_);
    }
}

void CustomPaint::paint_children(PaintContext& ctx) const {
    for (const auto& child : children_) {
        const Rect local{
            child->bounds().x - bounds_.x,
            child->bounds().y - bounds_.y,
            child->bounds().width,
            child->bounds().height,
        };
        ctx.with_clip(local, [&](PaintContext& child_ctx) { child->paint(child_ctx); });
    }
}

void CustomPaint::paint(PaintContext& ctx) const {
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    const Rect area{{0, 0}, bounds_.size()};
    paint_bounds_background(ctx, background_);

    if (paint_children_first_) {
        paint_children(ctx);
    }

    if (on_paint_) {
        on_paint_(ctx, area);
    }

    if (!paint_children_first_) {
        paint_children(ctx);
    }
}

} // namespace tuinator
