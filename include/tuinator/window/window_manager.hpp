#pragma once

#include <tuinator/render/paint_context.hpp>
#include <tuinator/window/window.hpp>

#include <functional>
#include <memory>
#include <vector>

namespace tuinator {

struct WindowManagerOptions {
    int min_window_width = 10;
    int min_window_height = 5;
    bool modal_aware = false;
    bool pointer_capture_aware = false;
    bool drag_before_content = false;
    bool local_coordinates = false;
    bool paint_backdrop = false;
    bool paint_shadows = false;
};

WindowManagerOptions desktop_window_manager_options();
WindowManagerOptions nested_window_manager_options();

class WindowManager {
  public:
    explicit WindowManager(WindowManagerOptions options = {});

    void set_bounds(Rect bounds);
    void set_on_dirty(std::function<void(Rect)> callback);

    Window* create_window(std::string title, Rect bounds, std::unique_ptr<Widget> content, WindowOptions options = {});
    void close_window(Window* window);

    void layout_windows();
    void paint_windows(PaintContext& ctx) const;

    bool handle_mouse(const MouseEvent& event);
    bool route_keyboard(const Event& event);
    void close_top_window();

    Window* active_window() const { return active_window_; }
    Window* top_window_at(Point point) const;
    Window* top_modal() const;
    bool has_modal() const;
    bool is_dragging() const;

    const std::vector<std::unique_ptr<Window>>& windows() const { return windows_; }

  private:
    enum class DragMode {
        None,
        Move,
        Resize,
    };

    Rect clamp_area() const;
    Point paint_offset() const;
    Point to_window_local(Point point, const Window& window) const;
    MouseEvent to_window_event(const MouseEvent& event, const Window& window) const;

    void focus_window(Window* window);
    void bring_to_front(Window* window);
    void sort_windows();
    Window* window_capturing_pointer() const;

    bool begin_drag_if_needed(const MouseEvent& event, Window* window);
    bool update_drag(const MouseEvent& event);
    void end_drag(const MouseEvent& event);
    bool dispatch_window_click(const MouseEvent& event);

    WindowManagerOptions options_;
    Rect host_bounds_{};
    std::function<void(Rect)> on_dirty_;
    std::vector<std::unique_ptr<Window>> windows_;
    Window* active_window_ = nullptr;
    DragMode drag_mode_ = DragMode::None;
    Window* drag_window_ = nullptr;
    Point drag_anchor_{};
    Rect drag_start_bounds_{};
    int next_z_index_ = 1;
};

} // namespace tuinator
