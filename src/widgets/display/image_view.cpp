#include <tuinator/widgets/display/image_view.hpp>

#include <tuinator/render/graphics_protocol.hpp>

#include <algorithm>

namespace tuinator {

ImageView::ImageView(TerminalImage image, Size display_cells)
    : image_(std::move(image)), display_cells_(display_cells) {}

void ImageView::set_image(TerminalImage image) {
    image_ = std::move(image);
    mark_dirty();
}

void ImageView::set_display_cells(Size cells) {
    display_cells_ = cells;
    mark_dirty();
}

Size ImageView::preferred_size() const {
    if (display_cells_.width > 0 && display_cells_.height > 0) {
        return display_cells_;
    }

    if (image_.empty()) {
        return {20, 10};
    }

    const int width = std::max(8, std::min(40, image_.width() / 8));
    const int height = std::max(4, std::min(20, image_.height() / 16));
    return {width, height};
}

void ImageView::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (image_.empty() || bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    if (active_graphics_protocol() == GraphicsProtocol::None) {
        canvas.draw_text({0, 0}, "(terminal graphics not available)", Style{});
        return;
    }

    // Leave cells empty; Kitty/Sixel overlay is flushed after ncurses refresh.
    canvas.draw_image({0, 0}, {bounds_.width, bounds_.height}, image_);
}

} // namespace tuinator
