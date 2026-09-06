#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/widgets/widget.hpp>

namespace tuinator {

class Separator : public Widget {
  public:
    explicit Separator(Style style = {});

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;

  private:
    Style style_;
};

} // namespace tuinator
