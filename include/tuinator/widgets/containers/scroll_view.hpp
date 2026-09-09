#pragma once

#include <tuinator/render/scrollbar.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/render/style_resolver.hpp>
#include <tuinator/widgets/capabilities.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/widget.hpp>

#include <memory>

namespace tuinator {

struct ScrollViewOptions {
    int width = 40;
    int height = 10;
    ScrollbarOptions scrollbars{};
    Style background{};
};

class ScrollView : public Widget, public Scrollable, public SingleChildContainer {
  public:
    explicit ScrollView(std::unique_ptr<Widget> content, ScrollViewOptions options = {});

    Widget* content() const override { return content_.get(); }
    void set_content(std::unique_ptr<Widget> content) override;
    int scroll_x() const override { return scroll_x_; }
    int scroll_y() const override { return scroll_y_; }
    int max_scroll_x() const;
    int max_scroll_y() const;

    void scroll_to(int x, int y);
    void scroll_by(int dx, int dy) override;
    void refresh_content();

    bool contains_widget(const Widget* widget) const override;
    void ensure_visible(const Widget* widget) override;
    bool try_scroll(const Event& event) override;
    Widget* scroll_content() const override { return content(); }

    void set_on_dirty(std::function<void(Rect)> callback) override;

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return false; }
    bool pointer_active() const override;
    bool has_focused_descendant() const override;
    void collect_focusable(std::vector<Widget*>& out) override;
    Widget* hit_test(Point point) override;
    Widget* hit_test_focusable(Point point) override;
    void for_each_child(const std::function<void(Widget*)>& visitor) override;

    const ScrollViewOptions& options() const { return options_; }
    void set_options(ScrollViewOptions options);

    std::string_view widget_type_name() const override { return "ScrollView"; }
    void apply_stylesheet(const StyleResolver& styles) override;

  private:
    void clamp_scroll();
    void layout_content();
    void bind_content_dirty_callback();
    ScrollbarMetrics scrollbar_metrics() const;
    ScrollbarLayout scrollbar_layout() const;
    Point to_local(Point terminal) const;
    Point to_content_local(Point terminal) const;
    bool handle_scrollbar_mouse(const MouseEvent& mouse);
    ScrollbarScrollActions scrollbar_actions();

    std::unique_ptr<Widget> content_;
    ScrollViewOptions options_;
    int scroll_x_ = 0;
    int scroll_y_ = 0;
    int content_width_ = 0;
    int content_height_ = 0;
    ScrollbarInteractionState scrollbar_state_{};
};

} // namespace tuinator
