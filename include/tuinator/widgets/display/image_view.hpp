#pragma once

#include <tuinator/render/terminal_image.hpp>
#include <tuinator/widgets/widget.hpp>

namespace tuinator {

class ImageView : public Widget {
  public:
    ImageView(TerminalImage image, Size display_cells = {});

    const TerminalImage& image() const { return image_; }
    Size display_cells() const { return display_cells_; }

    void set_image(TerminalImage image);
    void set_display_cells(Size cells);

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;

  private:
    TerminalImage image_;
    Size display_cells_{};
};

} // namespace tuinator
