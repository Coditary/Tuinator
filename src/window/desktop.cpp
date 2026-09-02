#include <tuinator/window/desktop.hpp>

#include <tuinator/core/action_registry.hpp>
#include <tuinator/widgets/menu/command_palette.hpp>
#include <tuinator/widgets/menu/context_menu.hpp>
#include <tuinator/widgets/menu/menu_bar.hpp>

namespace tuinator {

Desktop::Desktop()
    : windows_(desktop_window_manager_options()) {
    context_menu_ = std::make_unique<ContextMenu>();
    command_palette_ = std::make_unique<CommandPalette>();
    windows_.set_on_dirty([this](Rect region) {
        (void)region;
        mark_dirty();
    });
}

Desktop::~Desktop() = default;

Window* Desktop::create_window(std::string title, Rect bounds, std::unique_ptr<Widget> content,
                               WindowOptions options) {
    return windows_.create_window(std::move(title), bounds, std::move(content), options);
}

Window* Desktop::show_modal(std::string title, Rect bounds, std::unique_ptr<Widget> content) {
    WindowOptions options{};
    options.modal = true;
    options.movable = false;
    options.resizable = false;
    return create_window(std::move(title), bounds, std::move(content), options);
}

void Desktop::close_window(Window* window) {
    windows_.close_window(window);
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
    windows_.set_bounds(bounds_);

    if (background_) {
        background_->layout(bounds_);
    }

    windows_.layout_windows();

    if (context_menu_) {
        context_menu_->layout(bounds_);
    }
    if (command_palette_) {
        command_palette_->layout(bounds_);
    }
}

void Desktop::paint(PaintContext& ctx) const {
    if (background_) {
        background_->paint(ctx);
    }

    if (windows_.has_modal()) {
        Style dim{};
        dim.dim = true;
        ctx.canvas.fill_rect({{0, 0}, ctx.canvas.size()}, ' ', dim);
    }

    windows_.paint_windows(ctx);

    if (context_menu_ && context_menu_->is_open()) {
        context_menu_->paint(ctx);
    }
    if (command_palette_ && command_palette_->is_open()) {
        command_palette_->paint(ctx);
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
            if (!windows_.has_modal() && windows_.top_window_at(mouse->position) == nullptr && context_menu_) {
                show_context_menu(mouse->position);
                return true;
            }
        }

        if (windows_.handle_mouse(*mouse)) {
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

        if (windows_.route_keyboard(event)) {
            return true;
        }

        if (key->key == Key::Escape) {
            windows_.close_top_window();
            return true;
        }
    }

    if (!windows_.has_modal() && background_ && background_->handle_event(event)) {
        mark_dirty();
        return true;
    }

    return false;
}

void Desktop::collect_focusable(std::vector<Widget*>& out) {
    if (windows_.active_window() != nullptr) {
        windows_.active_window()->collect_focusable(out);
    } else if (background_) {
        background_->collect_focusable(out);
    }
}

Widget* Desktop::hit_test_focusable(Point point) {
    if (!bounds_.contains(point)) {
        return nullptr;
    }

    if (windows_.has_modal()) {
        Window* modal = windows_.top_modal();
        if (modal == nullptr || !modal->contains_point(point)) {
            return nullptr;
        }
        return modal->hit_test_focusable(point);
    }

    if (Window* window = windows_.top_window_at(point)) {
        return window->hit_test_focusable(point);
    }

    if (background_) {
        return background_->hit_test_focusable(point);
    }

    return nullptr;
}

} // namespace tuinator
