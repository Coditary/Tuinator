#include <tuinator/window/window_host.hpp>

#include <algorithm>

namespace tuinator {

namespace {

constexpr int kMinWindowWidth = 10;
constexpr int kMinWindowHeight = 5;

Point to_local_point(Point host_local, Rect frame) {
    return {host_local.x - frame.x, host_local.y - frame.y};
}

} // namespace

WindowHost::WindowHost() = default;

Window* WindowHost::create_window(std::string title, Rect local_bounds,
                                   std::unique_ptr<Widget> content, WindowOptions options) {
    auto window = std::make_unique<Window>(std::move(title), local_bounds, std::move(content), options);
    Window* raw = window.get();
    raw->set_z_index(next_z_index_++);
    raw->set_on_dirty(on_dirty_);
    raw->set_on_close([this, raw]() { close_window(raw); });
    windows_.push_back(std::move(window));
    focus_window(raw);
    sort_windows();
    mark_dirty();
    return raw;
}

void WindowHost::close_window(Window* window) {
    if (!window) {
        return;
    }

    const auto it = std::find_if(windows_.begin(), windows_.end(),
                                 [window](const std::unique_ptr<Window>& entry) {
                                     return entry.get() == window;
                                 });
    if (it == windows_.end()) {
        return;
    }

    if (active_window_ == window) {
        active_window_ = nullptr;
    }
    if (drag_window_ == window) {
        drag_mode_ = DragMode::None;
        drag_window_ = nullptr;
    }

    windows_.erase(it);

    if (!windows_.empty()) {
        focus_window(windows_.back().get());
    }

    sort_windows();
    mark_dirty();
}

Rect WindowHost::host_area() const {
    return {{0, 0}, bounds_.size()};
}

Point WindowHost::to_host_local(Point absolute) const {
    return {absolute.x - bounds_.x, absolute.y - bounds_.y};
}

MouseEvent WindowHost::to_host_event(const MouseEvent& event) const {
    MouseEvent local = event;
    local.position = to_host_local(event.position);
    return local;
}

MouseEvent WindowHost::to_window_event(const MouseEvent& event, const Window& window) const {
    MouseEvent local = event;
    local.position = to_local_point(event.position, window.frame_bounds());
    return local;
}

Size WindowHost::preferred_size() const {
    return {36, 14};
}

void WindowHost::layout(Rect bounds) {
    bounds_ = bounds;

    const Rect area = host_area();
    for (auto& window : windows_) {
        window->layout(window->frame_bounds().clamped_to(area));
    }
}

void WindowHost::paint(Canvas& canvas) const {
    canvas.with_clip(host_area(), [&](Canvas& clipped) {
        Style backdrop{};
        backdrop.dim = true;
        clipped.fill_rect(host_area(), '.', backdrop);

        std::vector<const Window*> draw_order;
        draw_order.reserve(windows_.size());
        for (const auto& window : windows_) {
            draw_order.push_back(window.get());
        }

        std::sort(draw_order.begin(), draw_order.end(),
                  [](const Window* a, const Window* b) { return a->z_index() < b->z_index(); });

        for (const Window* window : draw_order) {
            const Rect frame = window->frame_bounds();
            const Rect local_frame{
                frame.x - bounds_.x,
                frame.y - bounds_.y,
                frame.width,
                frame.height,
            };
            const Rect visible = local_frame.clamped_to(host_area());
            if (visible.width <= 0 || visible.height <= 0) {
                continue;
            }

            clipped.with_clip(visible, [&](Canvas& window_canvas) {
                window->paint(window_canvas);
            });
        }
    });
}

bool WindowHost::captures_pointer() const {
    return drag_mode_ != DragMode::None;
}

bool WindowHost::handle_event(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (drag_mode_ != DragMode::None || bounds_.contains(mouse->position)) {
            if (handle_mouse(to_host_event(*mouse))) {
                return true;
            }
        }

        return false;
    }

    if (const auto* key = std::get_if<KeyPress>(&event)) {
        if (key->key == Key::Escape && !windows_.empty()) {
            close_top_window();
            return true;
        }
    }

    if (route_keyboard(event)) {
        return true;
    }

    return false;
}

void WindowHost::collect_focusable(std::vector<Widget*>& out) {
    if (active_window_) {
        active_window_->collect_focusable(out);
    }
}

Widget* WindowHost::hit_test_focusable(Point point) {
    if (!bounds_.contains(point)) {
        return nullptr;
    }

    const Point host_local = to_host_local(point);
    if (!host_area().contains(host_local)) {
        return nullptr;
    }

    Window* window = top_window_at(host_local);
    if (!window) {
        return nullptr;
    }

    return window->hit_test_focusable(host_local);
}

Window* WindowHost::top_window_at(Point host_local) const {
    Window* result = nullptr;
    int best_z = -1;

    for (const auto& window : windows_) {
        if (!window->frame_bounds().contains(host_local)) {
            continue;
        }

        if (window->z_index() > best_z) {
            best_z = window->z_index();
            result = window.get();
        }
    }

    return result;
}

void WindowHost::focus_window(Window* window) {
    active_window_ = window;
    mark_dirty();
}

void WindowHost::bring_to_front(Window* window) {
    if (!window) {
        return;
    }

    window->set_z_index(next_z_index_++);
    focus_window(window);
    sort_windows();
    mark_dirty();
}

void WindowHost::sort_windows() {
    std::sort(windows_.begin(), windows_.end(),
              [](const std::unique_ptr<Window>& a, const std::unique_ptr<Window>& b) {
                  return a->z_index() < b->z_index();
              });
}

bool WindowHost::begin_drag_if_needed(const MouseEvent& host_event, Window* window) {
    if (!window || drag_mode_ != DragMode::None) {
        return false;
    }

    const bool can_start = host_event.action == MouseAction::Press
        || (host_event.action == MouseAction::Move && host_event.left_pressed);
    if (!can_start) {
        return false;
    }

    const Point local = to_local_point(host_event.position, window->frame_bounds());
    if (window->is_movable() && window->is_title_bar(local)) {
        drag_mode_ = DragMode::Move;
        drag_window_ = window;
        drag_anchor_ = host_event.position;
        drag_start_bounds_ = window->frame_bounds();
        return true;
    }

    if (window->is_resizable() && window->is_resize_handle(local)) {
        drag_mode_ = DragMode::Resize;
        drag_window_ = window;
        drag_anchor_ = host_event.position;
        drag_start_bounds_ = window->frame_bounds();
        return true;
    }

    return false;
}

bool WindowHost::update_drag(const MouseEvent& host_event) {
    if (drag_mode_ == DragMode::None || !drag_window_) {
        return false;
    }

    const bool motion_while_held =
        host_event.action == MouseAction::Move && host_event.left_pressed;
    const bool final_position = host_event.action == MouseAction::Release;

    if (!motion_while_held && !final_position) {
        return false;
    }

    const Rect area = host_area();

    if (drag_mode_ == DragMode::Move) {
        const int dx = host_event.position.x - drag_anchor_.x;
        const int dy = host_event.position.y - drag_anchor_.y;
        drag_window_->set_bounds(drag_start_bounds_.translated(dx, dy).clamped_to(area));
        mark_dirty();
        return true;
    }

    if (drag_mode_ == DragMode::Resize) {
        const int min_right = drag_start_bounds_.x + kMinWindowWidth - 1;
        const int min_bottom = drag_start_bounds_.y + kMinWindowHeight - 1;
        const int max_right = area.right() - 1;
        const int max_bottom = area.bottom() - 1;

        const int mouse_x = std::clamp(host_event.position.x, min_right, max_right);
        const int mouse_y = std::clamp(host_event.position.y, min_bottom, max_bottom);
        const int width = mouse_x - drag_start_bounds_.x + 1;
        const int height = mouse_y - drag_start_bounds_.y + 1;
        drag_window_->set_bounds({drag_start_bounds_.x, drag_start_bounds_.y, width, height});
        mark_dirty();
        return true;
    }

    return false;
}

void WindowHost::end_drag(const MouseEvent& host_event) {
    if (drag_mode_ == DragMode::None || !drag_window_) {
        return;
    }

    update_drag(host_event);
    drag_mode_ = DragMode::None;
    drag_window_ = nullptr;
}

bool WindowHost::handle_mouse(const MouseEvent& host_event) {
    if (drag_mode_ != DragMode::None && drag_window_) {
        if (host_event.action == MouseAction::Release || host_event.action == MouseAction::Click) {
            end_drag(host_event);
            if (host_event.action == MouseAction::Click) {
                return dispatch_window_click(host_event);
            }
            return true;
        }

        if (host_event.action == MouseAction::Press) {
            end_drag(host_event);
            return dispatch_window_click(host_event);
        }

        if (host_event.action == MouseAction::Move && host_event.left_pressed) {
            return update_drag(host_event);
        }

        if (host_event.action == MouseAction::Move && !host_event.left_pressed) {
            end_drag(host_event);
            return false;
        }

        return true;
    }

    if (host_event.action == MouseAction::Release || host_event.action == MouseAction::Click) {
        return dispatch_window_click(host_event);
    }

    if (host_event.action == MouseAction::Press
        || (host_event.action == MouseAction::Move && host_event.left_pressed)) {
        return dispatch_window_click(host_event);
    }

    return false;
}

bool WindowHost::dispatch_window_click(const MouseEvent& host_event) {
    Window* window = top_window_at(host_event.position);
    if (!window) {
        return false;
    }

    bring_to_front(window);

    if (begin_drag_if_needed(host_event, window)) {
        update_drag(host_event);
        return true;
    }

    const MouseEvent window_event = to_window_event(host_event, *window);
    if (window->handle_event(window_event)) {
        mark_dirty();
        return true;
    }

    return host_event.action != MouseAction::Press
        && !(host_event.action == MouseAction::Move && host_event.left_pressed);
}

bool WindowHost::route_keyboard(const Event& event) {
    if (!active_window_) {
        return false;
    }

    if (active_window_->handle_event(event)) {
        mark_dirty();
        return true;
    }

    return false;
}

void WindowHost::close_top_window() {
    if (!active_window_ || !active_window_->is_closable()) {
        return;
    }

    close_window(active_window_);
}

} // namespace tuinator
