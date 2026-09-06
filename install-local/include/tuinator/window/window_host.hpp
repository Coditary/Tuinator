#pragma once

#include <tuinator/window/window.hpp>
#include <tuinator/window/window_manager.hpp>

#include <memory>

namespace tuinator {

// Manages floating windows inside a bounded area (e.g. inside another Window).
// Inner windows use coordinates local to this host and cannot leave its bounds.
class WindowHost : public Widget {
public:
    WindowHost();

    Window* create_window(std::string title, Rect local_bounds, std::unique_ptr<Widget> content,
                          WindowOptions options = {});
    void close_window(Window* window);

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    bool captures_pointer() const override;
    void collect_focusable(std::vector<Widget*>& out) override;
    Widget* hit_test_focusable(Point point) override;

    Window* active_window() const { return windows_.active_window(); }

private:
    Point to_host_local(Point absolute) const;
    MouseEvent to_host_event(const MouseEvent& event) const;

    WindowManager windows_;
};

} // namespace tuinator
