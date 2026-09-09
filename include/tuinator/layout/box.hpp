#pragma once

#include <tuinator/render/style_resolver.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/widget.hpp>

namespace tuinator {

struct BoxOptions {
    int gap = 1;
    int padding = 0;
};

class VBox : public Widget, public LayoutBox {
  public:
    explicit VBox(BoxOptions options = {});

    int gap() const { return gap_; }
    int padding() const { return padding_; }
    void set_gap(int gap);
    void set_padding(int padding);
    int layout_gap() const override { return gap_; }
    int layout_padding() const override { return padding_; }
    void set_layout_gap(int gap) override { set_gap(gap); }
    void set_layout_padding(int padding) override { set_padding(padding); }

    std::string_view widget_type_name() const override { return "VBox"; }
    void apply_stylesheet(const StyleResolver& styles) override;

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;

  private:
    int gap_;
    int padding_;
};

class HBox : public Widget, public LayoutBox {
  public:
    explicit HBox(BoxOptions options = {});

    int gap() const { return gap_; }
    int padding() const { return padding_; }
    void set_gap(int gap);
    void set_padding(int padding);
    int layout_gap() const override { return gap_; }
    int layout_padding() const override { return padding_; }
    void set_layout_gap(int gap) override { set_gap(gap); }
    void set_layout_padding(int padding) override { set_padding(padding); }

    std::string_view widget_type_name() const override { return "HBox"; }
    void apply_stylesheet(const StyleResolver& styles) override;

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;
    Widget* hit_test(Point point) override;
    Widget* hit_test_focusable(Point point) override;
    bool handle_event(const Event& event) override;

  private:
    Widget* focusable_child_at(Point point) const;
    int gap_;
    int padding_;
};

} // namespace tuinator
