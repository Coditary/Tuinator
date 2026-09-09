#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/widgets/widget.hpp>

namespace tuinator {

class Separator : public Widget {
  public:
    explicit Separator(Style style = {});

    std::string_view widget_type_name() const override { return "Separator"; }

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;

  private:
    Style style_;
};

} // namespace tuinator
