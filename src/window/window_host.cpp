#include <tuinator/window/window_host.hpp>

namespace tuinator {

WindowHost::WindowHost() : windows_(nested_window_manager_options()) {
    windows_.set_on_dirty([this](Rect region) {
        (void)region;
        mark_dirty();
    });
}

Window* WindowHost::create_window(std::string title, Rect local_bounds, std::unique_ptr<Widget> content,
                                  WindowOptions options) {
    return windows_.create_window(std::move(title), local_bounds, std::move(content), options);
}

void WindowHost::close_window(Window* window) { windows_.close_window(window); }

Point WindowHost::to_host_local(Point absolute) const { return {absolute.x - bounds_.x, absolute.y - bounds_.y}; }

MouseEvent WindowHost::to_host_event(const MouseEvent& event) const {
    MouseEvent local = event;
    local.position = to_host_local(event.position);
    return local;
}

Size WindowHost::preferred_size() const { return {36, 14}; }

void WindowHost::layout(Rect bounds) {
    bounds_ = bounds;
    windows_.set_bounds(bounds_);
    windows_.layout_windows();
}

void WindowHost::paint(PaintContext& ctx) const { windows_.paint_windows(ctx); }

bool WindowHost::captures_pointer() const { return windows_.is_dragging(); }

bool WindowHost::handle_event(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (windows_.is_dragging() || bounds_.contains(mouse->position)) {
            if (windows_.handle_mouse(to_host_event(*mouse))) {
                return true;
            }
        }

        return false;
    }

    if (const auto* key = std::get_if<KeyPress>(&event)) {
        if (key->key == Key::Escape && !windows_.windows().empty()) {
            windows_.close_top_window();
            return true;
        }
    }

    if (windows_.route_keyboard(event)) {
        return true;
    }

    return false;
}

void WindowHost::collect_focusable(std::vector<Widget*>& out) {
    if (Window* active = windows_.active_window()) {
        active->collect_focusable(out);
    }
}

Widget* WindowHost::hit_test_focusable(Point point) {
    if (!bounds_.contains(point)) {
        return nullptr;
    }

    const Point host_local = to_host_local(point);
    const Rect host_area{{0, 0}, bounds_.size()};
    if (!host_area.contains(host_local)) {
        return nullptr;
    }

    Window* window = windows_.top_window_at(host_local);
    if (window == nullptr) {
        return nullptr;
    }

    return window->hit_test_focusable(host_local);
}

} // namespace tuinator
