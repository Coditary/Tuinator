#include <tuinator/window/desktop.hpp>

#include <tuinator/core/action_registry.hpp>
#include <tuinator/widgets/menu/command_palette.hpp>
#include <tuinator/widgets/menu/context_menu.hpp>
#include <tuinator/widgets/menu/menu_bar.hpp>

#include <algorithm>

namespace tuinator {

namespace {

constexpr int kMinWindowWidth = 12;
constexpr int kMinWindowHeight = 5;

Point to_local(Point absolute, Rect frame) {
    return {absolute.x - frame.x, absolute.y - frame.y};
}

MouseEvent to_window_event(const MouseEvent& event, const Window& window) {
    MouseEvent local = event;
    local.position = to_local(event.position, window.frame_bounds());
    return local;
}

} // namespace

Desktop::Desktop() {
    context_menu_ = std::make_unique<ContextMenu>();
    command_palette_ = std::make_unique<CommandPalette>();
}

Desktop::~Desktop() = default;

Window* Desktop::create_window(std::string title, Rect bounds, std::unique_ptr<Widget> content,
                               WindowOptions options) {
    auto window = std::make_unique<Window>(std::move(title), bounds, std::move(content), options);
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

Window* Desktop::show_modal(std::string title, Rect bounds, std::unique_ptr<Widget> content) {
    WindowOptions options{};
    options.modal = true;
    options.movable = false;
    options.resizable = false;
    return create_window(std::move(title), bounds, std::move(content), options);
}

void Desktop::close_window(Window* window) {
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

void Desktop::set_background(std::unique_ptr<Widget> background) {
    background_ = std::move(background);
    if (background_) {
        background_->set_on_dirty(on_dirty_);
    }
    mark_dirty();
}

void Desktop::set_context_menu_items(std::vector<MenuItem> items) {
    if (context_menu_) {
        context_menu_->set_items(std::move(items));
    }
}

void Desktop::show_context_menu(Point position) {
    if (!context_menu_) {
        return;
    }
    context_menu_->show(position);
    mark_dirty();
}

void Desktop::set_command_palette_entries(std::vector<CommandPaletteEntry> entries) {
    if (command_palette_) {
        command_palette_->set_entries(std::move(entries));
    }
}

void Desktop::show_command_palette() {
    if (!command_palette_) {
        return;
    }
    command_palette_->open();
    mark_dirty();
}

void Desktop::set_action_registry(std::shared_ptr<ActionRegistry> registry) {
    action_registry_ = std::move(registry);
}

Size Desktop::preferred_size() const {
    if (bounds_.width > 0 && bounds_.height > 0) {
        return bounds_.size();
    }
    return {80, 24};
}

void Desktop::layout(Rect bounds) {
    bounds_ = bounds;

    if (background_) {
        background_->layout(bounds_);
    }

    for (auto& window : windows_) {
        window->layout(window->frame_bounds().clamped_to(bounds_));
    }

    if (context_menu_) {
        context_menu_->layout(bounds_);
    }
    if (command_palette_) {
        command_palette_->layout(bounds_);
    }
}

void Desktop::paint(Canvas& canvas) const {
    if (background_) {
        background_->paint(canvas);
    }

    if (has_modal()) {
        Style dim{};
        dim.dim = true;
        canvas.fill_rect({{0, 0}, canvas.size()}, ' ', dim);
    }

    std::vector<const Window*> draw_order;
    draw_order.reserve(windows_.size());
    for (const auto& window : windows_) {
        draw_order.push_back(window.get());
    }

    std::sort(draw_order.begin(), draw_order.end(),
              [](const Window* a, const Window* b) { return a->z_index() < b->z_index(); });

    for (const Window* window : draw_order) {
        const Rect frame = window->frame_bounds();
        const Rect shadow = frame.translated(1, 1);

        canvas.with_clip({shadow.x - bounds_.x, shadow.y - bounds_.y, shadow.width, shadow.height},
                         [&](Canvas& shadow_canvas) {
                             Style dim{};
                             dim.dim = true;
                             shadow_canvas.fill_rect({0, 0, shadow.width, shadow.height}, ' ', dim);
                         });

        canvas.with_clip({frame.x - bounds_.x, frame.y - bounds_.y, frame.width, frame.height},
                         [&](Canvas& window_canvas) {
                             window->paint(window_canvas);
                         });
    }

    if (context_menu_ && context_menu_->is_open()) {
        context_menu_->paint(canvas);
    }
    if (command_palette_ && command_palette_->is_open()) {
        command_palette_->paint(canvas);
    }
}

bool Desktop::handle_event(const Event& event) {
    if (command_palette_ && command_palette_->is_open()) {
        if (command_palette_->handle_event(event)) {
            mark_dirty();
            return true;
        }
    }

    if (context_menu_ && context_menu_->is_open()) {
        if (context_menu_->handle_event(event)) {
            mark_dirty();
            return true;
        }
    }

    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (mouse->button == MouseButton::Right
            && (mouse->action == MouseAction::Click || mouse->action == MouseAction::Release)) {
            if (!has_modal() && !top_window_at(mouse->position) && context_menu_) {
                show_context_menu(mouse->position);
                return true;
            }
        }

        if (handle_mouse(*mouse)) {
            return true;
        }
    }

    if (const auto* key = std::get_if<KeyPress>(&event)) {
        if (action_registry_ && action_registry_->handle_key(*key)) {
            mark_dirty();
            return true;
        }

        if (key->ctrl && (key->character == 'p' || key->character == 'P')) {
            show_command_palette();
            return true;
        }

        if (route_keyboard(event)) {
            return true;
        }

        if (key->key == Key::Escape) {
            close_top_window();
            return true;
        }
    }

    if (!has_modal() && background_ && background_->handle_event(event)) {
        mark_dirty();
        return true;
    }

    return false;
}

void Desktop::collect_focusable(std::vector<Widget*>& out) {
    if (active_window_) {
        active_window_->collect_focusable(out);
    } else if (background_) {
        background_->collect_focusable(out);
    }
}

Widget* Desktop::hit_test_focusable(Point point) {
    if (!bounds_.contains(point)) {
        return nullptr;
    }

    if (has_modal()) {
        Window* modal = top_modal();
        if (!modal || !modal->contains_point(point)) {
            return nullptr;
        }
        return modal->hit_test_focusable(point);
    }

    if (Window* window = top_window_at(point)) {
        return window->hit_test_focusable(point);
    }

    if (background_) {
        return background_->hit_test_focusable(point);
    }

    return nullptr;
}

Window* Desktop::top_window_at(Point point) const {
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

Window* Desktop::top_modal() const {
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

bool Desktop::has_modal() const {
    return top_modal() != nullptr;
}

void Desktop::focus_window(Window* window) {
    active_window_ = window;
    mark_dirty();
}

void Desktop::bring_to_front(Window* window) {
    if (!window) {
        return;
    }

    window->set_z_index(next_z_index_++);
    focus_window(window);
    sort_windows();
    mark_dirty();
}

void Desktop::sort_windows() {
    std::sort(windows_.begin(), windows_.end(),
              [](const std::unique_ptr<Window>& a, const std::unique_ptr<Window>& b) {
                  return a->z_index() < b->z_index();
              });
}

bool Desktop::begin_drag_if_needed(const MouseEvent& event, Window* window) {
    if (!window || drag_mode_ != DragMode::None) {
        return false;
    }

    const bool can_start = event.action == MouseAction::Press
        || (event.action == MouseAction::Move && event.left_pressed);
    if (!can_start) {
        return false;
    }

    const Point local = to_local(event.position, window->frame_bounds());
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

bool Desktop::update_drag(const MouseEvent& event) {
    if (drag_mode_ == DragMode::None || !drag_window_) {
        return false;
    }

    const bool motion_while_held =
        event.action == MouseAction::Move && event.left_pressed;
    const bool final_position = event.action == MouseAction::Release;

    if (!motion_while_held && !final_position) {
        return false;
    }

    if (drag_mode_ == DragMode::Move) {
        const int dx = event.position.x - drag_anchor_.x;
        const int dy = event.position.y - drag_anchor_.y;
        drag_window_->set_bounds(drag_start_bounds_.translated(dx, dy).clamped_to(bounds_));
        mark_dirty();
        return true;
    }

    if (drag_mode_ == DragMode::Resize) {
        const int min_right = drag_start_bounds_.x + kMinWindowWidth - 1;
        const int min_bottom = drag_start_bounds_.y + kMinWindowHeight - 1;
        const int max_right = bounds_.right() - 1;
        const int max_bottom = bounds_.bottom() - 1;

        const int mouse_x = std::clamp(event.position.x, min_right, max_right);
        const int mouse_y = std::clamp(event.position.y, min_bottom, max_bottom);
        const int width = mouse_x - drag_start_bounds_.x + 1;
        const int height = mouse_y - drag_start_bounds_.y + 1;
        const Rect next{drag_start_bounds_.x, drag_start_bounds_.y, width, height};
        drag_window_->set_bounds(next);
        mark_dirty();
        return true;
    }

    return false;
}

void Desktop::end_drag(const MouseEvent& event) {
    if (drag_mode_ == DragMode::None || !drag_window_) {
        return;
    }

    update_drag(event);
    drag_mode_ = DragMode::None;
    drag_window_ = nullptr;
}

bool Desktop::handle_mouse(const MouseEvent& event) {
    if (drag_mode_ != DragMode::None && drag_window_) {
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

Window* Desktop::window_capturing_pointer() const {
    for (const auto& window : windows_) {
        if (window->content() && window->content()->captures_pointer()) {
            return window.get();
        }
    }

    return nullptr;
}

bool Desktop::dispatch_window_click(const MouseEvent& event) {
    if (Window* capturing = window_capturing_pointer()) {
        const MouseEvent window_event = to_window_event(event, *capturing);
        if (capturing->handle_event(window_event)) {
            mark_dirty();
        }
        return true;
    }

    Window* window = top_window_at(event.position);
    if (has_modal()) {
        Window* modal = top_modal();
        if (!modal || !modal->contains_point(event.position)) {
            return true;
        }
        window = modal;
    }

    if (!window) {
        return false;
    }

    bring_to_front(window);

    const MouseEvent window_event = to_window_event(event, *window);
    if (window->handle_event(window_event)) {
        mark_dirty();
        return true;
    }

    if (begin_drag_if_needed(event, window)) {
        update_drag(event);
        return true;
    }

    return event.action != MouseAction::Press
        && !(event.action == MouseAction::Move && event.left_pressed);
}

bool Desktop::route_keyboard(const Event& event) {
    if (!active_window_) {
        return false;
    }

    if (has_modal() && active_window_ != top_modal()) {
        return true;
    }

    if (active_window_->handle_event(event)) {
        mark_dirty();
        return true;
    }

    return false;
}

void Desktop::close_top_window() {
    Window* window = has_modal() ? top_modal() : active_window_;
    if (!window || !window->is_closable()) {
        return;
    }

    close_window(window);
}

} // namespace tuinator
