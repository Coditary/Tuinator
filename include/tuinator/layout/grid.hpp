#pragma once

#include <tuinator/render/style_resolver.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/widget.hpp>

namespace tuinator {

struct GridOptions {
    int columns = 2;
    int gap = 1;
    int padding = 0;
};

class Grid : public Widget, public LayoutBox {
  public:
    explicit Grid(GridOptions options = {});

    int columns() const { return columns_; }
    void set_columns(int columns);
    int layout_gap() const override { return gap_; }
    int layout_padding() const override { return padding_; }
    int layout_column_count() const override { return columns_; }
    void set_layout_gap(int gap) override;
    void set_layout_padding(int padding) override;
    void set_layout_column_count(int columns) override { set_columns(columns); }

    std::string_view widget_type_name() const override { return "Grid"; }
    void apply_stylesheet(const StyleResolver& styles) override;

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;

  private:
    int columns_;
    int gap_;
    int padding_;
};

} // namespace tuinator
