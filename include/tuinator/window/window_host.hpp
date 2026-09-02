#pragma once

#include <tuinator/window/window.hpp>

#include <memory>
#include <vector>

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
    void paint(Canvas& canvas) const override;
    bool handle_event(const Event& event) override;
    bool captures_pointer() const override;
    void collect_focusable(std::vector<Widget*>& out) override;
    Widget* hit_test_focusable(Point point) override;

    Window* active_window() const { return active_window_; }

private:
    enum class DragMode {
        None,
        Move,
        Resize,
    };

    Rect host_area() const;
    Point to_host_local(Point absolute) const;
    MouseEvent to_host_event(const MouseEvent& event) const;
    MouseEvent to_window_event(const MouseEvent& event, const Window& window) const;

    Window* top_window_at(Point host_local) const;
    void focus_window(Window* window);
    void bring_to_front(Window* window);
    void sort_windows();
    bool handle_mouse(const MouseEvent& event);
    bool dispatch_window_click(const MouseEvent& host_event);
    bool begin_drag_if_needed(const MouseEvent& host_event, Window* window);
    bool update_drag(const MouseEvent& host_event);
    void end_drag(const MouseEvent& host_event);
    bool route_keyboard(const Event& event);
    void close_top_window();

    std::vector<std::unique_ptr<Window>> windows_;
    Window* active_window_ = nullptr;
    DragMode drag_mode_ = DragMode::None;
    Window* drag_window_ = nullptr;
    Point drag_anchor_{};
    Rect drag_start_bounds_{};
    int next_z_index_ = 1;
};

} // namespace tuinator
