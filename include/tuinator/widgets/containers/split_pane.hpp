#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/widgets/widget.hpp>

#include <memory>

namespace tuinator {

enum class SplitOrientation {
    Horizontal,
    Vertical,
};

struct SplitPaneOptions {
    SplitOrientation orientation = SplitOrientation::Horizontal;
    int first_size = 24;
    Style divider_style{};
};

class SplitPane : public Widget {
public:
    SplitPane(std::unique_ptr<Widget> first,
              std::unique_ptr<Widget> second,
              SplitPaneOptions options = {});

    Widget* first() const { return first_.get(); }
    Widget* second() const { return second_.get(); }

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(Canvas& canvas) const override;
    bool handle_event(const Event& event) override;
    bool has_focused_descendant() const override;
    void collect_focusable(std::vector<Widget*>& out) override;

private:
    std::unique_ptr<Widget> first_;
    std::unique_ptr<Widget> second_;
    SplitPaneOptions options_;
};

} // namespace tuinator
