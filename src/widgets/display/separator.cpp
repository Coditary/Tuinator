#include <tuinator/widgets/display/separator.hpp>

#include <string>

namespace tuinator {

Separator::Separator(Style style) : style_(style) {}

Size Separator::preferred_size() const { return {1, 1}; }

void Separator::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    paint_bounds_background(ctx, style_);
    canvas.draw_hline(0, 0, bounds_.width, ctx.styles().border(*this, style_));
}

} // namespace tuinator
