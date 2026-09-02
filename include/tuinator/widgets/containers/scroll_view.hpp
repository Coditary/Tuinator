#pragma once

#include <tuinator/render/scrollbar.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/widgets/widget.hpp>

#include <memory>

namespace tuinator {

struct ScrollViewOptions {
    int width = 40;
    int height = 10;
    ScrollbarOptions scrollbars{};
    Style background{};
};

class ScrollView : public Widget {
public:
    explicit ScrollView(std::unique_ptr<Widget> content, ScrollViewOptions options = {});

    Widget* content() const { return content_.get(); }
    int scroll_x() const { return scroll_x_; }
    int scroll_y() const { return scroll_y_; }
    int max_scroll_x() const;
    int max_scroll_y() const;

    void scroll_to(int x, int y);
    void scroll_by(int dx, int dy);
    void refresh_content();

    bool contains_widget(const Widget* widget) const;
    void ensure_visible(const Widget* widget);
    bool try_scroll(const Event& event);

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(Canvas& canvas) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return false; }
    bool pointer_active() const override;
    bool has_focused_descendant() const override;
    void collect_focusable(std::vector<Widget*>& out) override;
    Widget* hit_test(Point point) override;
    Widget* hit_test_focusable(Point point) override;

private:
    void clamp_scroll();
    void layout_content();
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
