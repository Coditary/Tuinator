#pragma once

#include <tuinator/render/paint_context.hpp>
#include <tuinator/render/style_resolver.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/containers/split_pane_options.hpp>
#include <tuinator/widgets/widget.hpp>

#include <memory>

namespace tuinator {

struct SplitDividerLine {
    bool vertical = false;
    int fixed = 0;
    int start = 0;
    int end = 0;
};

class SplitPane : public Widget, public BinaryContainer, public Splittable {
  public:
    SplitPane(std::unique_ptr<Widget> first, std::unique_ptr<Widget> second, SplitPaneOptions options = {});

    Widget* first() const { return first_.get(); }
    Widget* second() const { return second_.get(); }
    Widget* first_child() const override { return first_.get(); }
    Widget* second_child() const override { return second_.get(); }
    int first_size() const { return options_.first_size; }
    const SplitPaneOptions& options() const { return options_; }
    const SplitPaneOptions& split_options() const override { return options_; }
    void set_options(SplitPaneOptions options);
    void set_split_options(SplitPaneOptions options) override { set_options(options); }
    bool wants_outer_border() const { return options_.outer_border; }

    std::string_view widget_type_name() const override { return "SplitPane"; }
    void apply_stylesheet(const StyleResolver& styles) override;

    void set_on_dirty(std::function<void(Rect)> callback) override;
    void set_on_layout(std::function<void()> callback) override;

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    Widget* hit_test(Point point) override;
    bool pointer_active() const override;
    bool has_focused_descendant() const override;
    void collect_focusable(std::vector<Widget*>& out) override;
    void for_each_child(const std::function<void(Widget*)>& visitor) override;
    void for_each_descendant(const std::function<void(Widget*)>& visitor) override;

    void append_divider_line(std::vector<SplitDividerLine>& lines) const;

  private:
    int divider_position() const;
    Rect divider_bounds() const;
    Rect divider_hit_bounds() const;
    bool contains_divider(Point point) const;
    void update_first_size_from_mouse(Point global_position);
    void begin_drag();
    void end_drag();

    std::unique_ptr<Widget> first_;
    std::unique_ptr<Widget> second_;
    SplitPaneOptions options_;
    bool dragging_ = false;
};

void join_split_divider_lines(std::vector<SplitDividerLine>& lines);
void paint_split_dividers(Widget& root, PaintContext& ctx);

} // namespace tuinator
