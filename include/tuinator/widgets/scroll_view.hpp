#pragma once

#include <tuinator/widgets/widget.hpp>

#include <memory>

namespace tuinator {

struct ScrollViewOptions {
    int width = 40;
    int height = 10;
};

class ScrollView : public Widget {
public:
    explicit ScrollView(std::unique_ptr<Widget> content, ScrollViewOptions options = {});

    Widget* content() const { return content_.get(); }
    int scroll_y() const { return scroll_y_; }
    int max_scroll_y() const;

    void scroll_to(int y);
    void scroll_by(int delta);
    void refresh_content();

    bool contains_widget(const Widget* widget) const;
    void ensure_visible(const Widget* widget);
    bool try_scroll(const Event& event);

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(Canvas& canvas) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return false; }
    bool has_focused_descendant() const override;
    void collect_focusable(std::vector<Widget*>& out) override;
    Widget* hit_test(Point point) override;
    Widget* hit_test_focusable(Point point) override;

private:
    void clamp_scroll();
    void layout_content();

    std::unique_ptr<Widget> content_;
    ScrollViewOptions options_;
    int scroll_y_ = 0;
    int content_height_ = 0;
};

} // namespace tuinator
