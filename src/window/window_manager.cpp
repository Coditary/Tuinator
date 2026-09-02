#include <tuinator/window/window_manager.hpp>

#include <algorithm>

namespace tuinator {

namespace {

Point to_window_local_point(Point point, const Window& window) {
    const Rect frame = window.frame_bounds();
    return {point.x - frame.x, point.y - frame.y};
}

} // namespace

WindowManagerOptions desktop_window_manager_options() {
    WindowManagerOptions options;
    options.min_window_width = 12;
    options.min_window_height = 5;
    options.modal_aware = true;
    options.pointer_capture_aware = true;
    options.drag_before_content = false;
    options.local_coordinates = false;
    options.paint_shadows = true;
    return options;
}

WindowManagerOptions nested_window_manager_options() {
    WindowManagerOptions options;
    options.min_window_width = 10;
    options.min_window_height = 5;
    options.drag_before_content = true;
    options.local_coordinates = true;
    options.paint_backdrop = true;
    return options;
}

WindowManager::WindowManager(WindowManagerOptions options)
    : options_(options) {}

void WindowManager::set_bounds(Rect bounds) {
    host_bounds_ = bounds;
}

void WindowManager::set_on_dirty(std::function<void(Rect)> callback) {
    on_dirty_ = std::move(callback);
}

Window* WindowManager::create_window(
    std::string title,
    Rect bounds,
    std::unique_ptr<Widget> content,
    WindowOptions options) {
    auto window = std::make_unique<Window>(std::move(title), bounds, std::move(content), options);
    Window* raw = window.get();
    raw->set_z_index(next_z_index_++);
    raw->set_on_dirty(on_dirty_);
    raw->set_on_close([this, raw]() { close_window(raw); });
    windows_.push_back(std::move(window));
    focus_window(raw);
    sort_windows();
    if (on_dirty_) {
        on_dirty_(host_bounds_);
    }
    return raw;
}

void WindowManager::close_window(Window* window) {
    if (window == nullptr) {
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
    if (on_dirty_) {
        on_dirty_(host_bounds_);
    }
}

void WindowManager::layout_windows() {
    const Rect area = clamp_area();
    for (auto& window : windows_) {
        window->layout(window->frame_bounds().clamped_to(area));
    }
}

Rect WindowManager::clamp_area() const {
    if (options_.local_coordinates) {
        return {{0, 0}, host_bounds_.size()};
    }
    return host_bounds_;
}

Point WindowManager::paint_offset() const {
    return {host_bounds_.x, host_bounds_.y};
}

void WindowManager::paint_windows(PaintContext& ctx) const {
    const auto paint_stack = [&](PaintContext& clipped_ctx) {
        const Rect area = clamp_area();

        if (options_.paint_backdrop) {
            Style backdrop{};
            backdrop.dim = true;
            clipped_ctx.canvas.fill_rect(area, '.', backdrop);
        }

        std::vector<const Window*> draw_order;
        draw_order.reserve(windows_.size());
        for (const auto& window : windows_) {
            draw_order.push_back(window.get());
        }

        std::sort(draw_order.begin(), draw_order.end(),
                  [](const Window* a, const Window* b) { return a->z_index() < b->z_index(); });

        const Point offset = paint_offset();
        for (const Window* window : draw_order) {
            const Rect frame = window->frame_bounds();
            Rect local_frame{
                frame.x - offset.x,
                frame.y - offset.y,
                frame.width,
                frame.height,
            };

            if (options_.paint_shadows) {
                const Rect shadow = frame.translated(1, 1);
                const Rect local_shadow{
                    shadow.x - offset.x,
                    shadow.y - offset.y,
                    shadow.width,
                    shadow.height,
                };
                clipped_ctx.canvas.with_clip(local_shadow, [&](Canvas& shadow_canvas) {
                    Style dim{};
                    dim.dim = true;
                    shadow_canvas.fill_rect({0, 0, shadow.width, shadow.height}, ' ', dim);
                });
            }

            Rect visible = local_frame;
            if (options_.local_coordinates) {
                visible = local_frame.clamped_to(area);
                if (visible.width <= 0 || visible.height <= 0) {
                    continue;
                }
            }

            clipped_ctx.with_clip(visible, [&](PaintContext& window_ctx) { window->paint(window_ctx); });
        }
    };

    if (options_.local_coordinates) {
        ctx.with_clip(clamp_area(), paint_stack);
    } else {
        paint_stack(ctx);
    }
}

bool WindowManager::has_modal() const {
    return top_modal() != nullptr;
}

bool WindowManager::is_dragging() const {
    return drag_mode_ != DragMode::None;
}

Window* WindowManager::top_window_at(Point point) const {
    Window* result = nullptr;
    int best_z = -1;

    for (const auto& window : windows_) {
        if (!window->contains_point(point)) {
            continue;
        }

        if (window->z_index() > best_z) {
            best_z = window->z_index();
            result = window.get();
        }
    }

    return result;
}

Window* WindowManager::top_modal() const {
    Window* result = nullptr;
    int best_z = -1;

    for (const auto& window : windows_) {
        if (!window->is_modal()) {
            continue;
        }

        if (window->z_index() > best_z) {
            best_z = window->z_index();
            result = window.get();
        }
    }

    return result;
}

void WindowManager::focus_window(Window* window) {
    active_window_ = window;
    if (on_dirty_) {
        on_dirty_(host_bounds_);
    }
}

void WindowManager::bring_to_front(Window* window) {
    if (window == nullptr) {
        return;
    }

    window->set_z_index(next_z_index_++);
    focus_window(window);
    sort_windows();
    if (on_dirty_) {
        on_dirty_(host_bounds_);
    }
}

void WindowManager::sort_windows() {
    std::sort(windows_.begin(), windows_.end(),
              [](const std::unique_ptr<Window>& a, const std::unique_ptr<Window>& b) {
                  return a->z_index() < b->z_index();
              });
}

Point WindowManager::to_window_local(Point point, const Window& window) const {
    return to_window_local_point(point, window);
}

MouseEvent WindowManager::to_window_event(const MouseEvent& event, const Window& window) const {
    MouseEvent local = event;
    local.position = to_window_local(event.position, window);
    return local;
}

Window* WindowManager::window_capturing_pointer() const {
    for (const auto& window : windows_) {
        if (window->content() != nullptr && window->content()->captures_pointer()) {
            return window.get();
        }
    }

    return nullptr;
}

bool WindowManager::begin_drag_if_needed(const MouseEvent& event, Window* window) {
    if (window == nullptr || drag_mode_ != DragMode::None) {
        return false;
    }

    const bool can_start = event.action == MouseAction::Press
        || (event.action == MouseAction::Move && event.left_pressed);
    if (!can_start) {
        return false;
    }

    const Point local = to_window_local(event.position, *window);
    if (window->is_movable() && window->is_title_bar(local)) {
        drag_mode_ = DragMode::Move;
        drag_window_ = window;
        drag_anchor_ = event.position;
        drag_start_bounds_ = window->frame_bounds();
        return true;
    }

    if (window->is_resizable() && window->is_resize_handle(local)) {
        drag_mode_ = DragMode::Resize;
        drag_window_ = window;
        drag_anchor_ = event.position;
        drag_start_bounds_ = window->frame_bounds();
        return true;
    }

    return false;
}

bool WindowManager::update_drag(const MouseEvent& event) {
    if (drag_mode_ == DragMode::None || drag_window_ == nullptr) {
        return false;
    }

    const bool motion_while_held =
        event.action == MouseAction::Move && event.left_pressed;
    const bool final_position = event.action == MouseAction::Release;

    if (!motion_while_held && !final_position) {
        return false;
    }

    const Rect area = clamp_area();

    if (drag_mode_ == DragMode::Move) {
        const int dx = event.position.x - drag_anchor_.x;
        const int dy = event.position.y - drag_anchor_.y;
        drag_window_->set_bounds(drag_start_bounds_.translated(dx, dy).clamped_to(area));
        if (on_dirty_) {
            on_dirty_(host_bounds_);
        }
        return true;
    }

    if (drag_mode_ == DragMode::Resize) {
        const int min_right = drag_start_bounds_.x + options_.min_window_width - 1;
        const int min_bottom = drag_start_bounds_.y + options_.min_window_height - 1;
        const int max_right = area.right() - 1;
        const int max_bottom = area.bottom() - 1;

        const int mouse_x = std::clamp(event.position.x, min_right, max_right);
        const int mouse_y = std::clamp(event.position.y, min_bottom, max_bottom);
        const int width = mouse_x - drag_start_bounds_.x + 1;
        const int height = mouse_y - drag_start_bounds_.y + 1;
        drag_window_->set_bounds({drag_start_bounds_.x, drag_start_bounds_.y, width, height});
        if (on_dirty_) {
            on_dirty_(host_bounds_);
        }
        return true;
    }

    return false;
}

void WindowManager::end_drag(const MouseEvent& event) {
    if (drag_mode_ == DragMode::None || drag_window_ == nullptr) {
        return;
    }

    update_drag(event);
    drag_mode_ = DragMode::None;
    drag_window_ = nullptr;
}

bool WindowManager::handle_mouse(const MouseEvent& event) {
    if (drag_mode_ != DragMode::None && drag_window_ != nullptr) {
        if (event.action == MouseAction::Release || event.action == MouseAction::Click) {
            end_drag(event);
            if (event.action == MouseAction::Click) {
                return dispatch_window_click(event);
            }
            return true;
        }

        if (event.action == MouseAction::Press) {
            end_drag(event);
            return dispatch_window_click(event);
        }

        if (event.action == MouseAction::Move && event.left_pressed) {
            return update_drag(event);
        }

        if (event.action == MouseAction::Move && !event.left_pressed) {
            end_drag(event);
            return false;
        }

        return true;
    }

    if (event.action == MouseAction::Release || event.action == MouseAction::Click) {
        return dispatch_window_click(event);
    }

    if (event.action == MouseAction::Press
        || (event.action == MouseAction::Move && event.left_pressed)) {
        return dispatch_window_click(event);
    }

    return false;
}

bool WindowManager::dispatch_window_click(const MouseEvent& event) {
    if (options_.pointer_capture_aware) {
        if (Window* capturing = window_capturing_pointer()) {
            const MouseEvent window_event = to_window_event(event, *capturing);
            if (capturing->handle_event(window_event)) {
                if (on_dirty_) {
                    on_dirty_(host_bounds_);
                }
            }
            return true;
        }
    }

    Window* window = top_window_at(event.position);
    if (options_.modal_aware && has_modal()) {
        Window* modal = top_modal();
        if (modal == nullptr || !modal->contains_point(event.position)) {
            return true;
        }
        window = modal;
    }

    if (window == nullptr) {
        return false;
    }

    bring_to_front(window);

    if (options_.drag_before_content) {
        if (begin_drag_if_needed(event, window)) {
            update_drag(event);
            return true;
        }

        const MouseEvent window_event = to_window_event(event, *window);
        if (window->handle_event(window_event)) {
            if (on_dirty_) {
                on_dirty_(host_bounds_);
            }
            return true;
        }
    } else {
        const MouseEvent window_event = to_window_event(event, *window);
        if (window->handle_event(window_event)) {
            if (on_dirty_) {
                on_dirty_(host_bounds_);
            }
            return true;
        }

        if (begin_drag_if_needed(event, window)) {
            update_drag(event);
            return true;
        }
    }

    return event.action != MouseAction::Press
        && !(event.action == MouseAction::Move && event.left_pressed);
}

bool WindowManager::route_keyboard(const Event& event) {
    if (active_window_ == nullptr) {
        return false;
    }

    if (options_.modal_aware && has_modal() && active_window_ != top_modal()) {
        return true;
    }

    if (active_window_->handle_event(event)) {
        if (on_dirty_) {
            on_dirty_(host_bounds_);
        }
        return true;
    }

    return false;
}

void WindowManager::close_top_window() {
    Window* window = options_.modal_aware && has_modal() ? top_modal() : active_window_;
    if (window == nullptr || !window->is_closable()) {
        return;
    }

    close_window(window);
}

} // namespace tuinator
