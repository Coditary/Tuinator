#include <tuinator/core/event.hpp>
#include <tuinator/widgets/menu/context_menu.hpp>
#include <tuinator/widgets/menu/menu_common.hpp>

#include <algorithm>
#include <variant>

namespace tuinator {

ContextMenu::ContextMenu() = default;

void ContextMenu::set_items(std::vector<MenuItem> items) {
    items_ = std::move(items);
    close();
}

void ContextMenu::set_look(const MenuBarLook& look) {
    look_ = look;
    custom_look_ = true;
    mark_dirty();
}

void ContextMenu::set_on_action(std::function<void(const std::string& item)> callback) {
    on_action_ = std::move(callback);
}

void ContextMenu::set_on_hint(std::function<void(const std::string& hint)> callback) { on_hint_ = std::move(callback); }

void ContextMenu::show(Point position) {
    anchor_ = position;
    open_ = true;
    active_item_ = 0;
    submenu_path_.clear();
    if (!item_selectable(active_item_)) {
        move_selection(1);
    } else {
        notify_hint();
    }
    mark_layout_dirty();
}

void ContextMenu::close() {
    if (!open_) {
        return;
    }
    open_ = false;
    submenu_path_.clear();
    active_item_ = 0;
    notify_hint();
    mark_layout_dirty();
}

const std::vector<MenuItem>* ContextMenu::current_items() const {
    if (!open_) {
        return nullptr;
    }
    return menu_follow_path(items_, submenu_path_);
}

Size ContextMenu::preferred_size() const {
    if (!open_) {
        return {0, 0};
    }

    int width = menu_panel_width(items_);
    int height = 0;
    for (const MenuPanelLayout& panel : open_panels()) {
        width = std::max(width, panel.x + panel.width);
        height = std::max(height, panel.y + panel.height);
    }
    return {width, height};
}

std::vector<MenuPanelLayout> ContextMenu::open_panels() const {
    std::vector<MenuPanelLayout> panels;
    if (!open_) {
        return panels;
    }

    MenuPanelLayout root;
    root.x = anchor_.x;
    root.y = anchor_.y;
    root.width = menu_panel_width(items_);
    root.height = static_cast<int>(items_.size()) + 2;
    panels.push_back(root);

    const std::vector<MenuItem>* items = &items_;
    for (std::size_t depth = 0; depth < submenu_path_.size(); ++depth) {
        const int index = submenu_path_[depth];
        if (index < 0 || index >= static_cast<int>(items->size())) {
            break;
        }

        const MenuItem& parent = (*items)[static_cast<std::size_t>(index)];
        items = &parent.children;

        MenuPanelLayout panel;
        panel.width = menu_panel_width(*items);
        panel.height = static_cast<int>(items->size()) + 2;
        const MenuPanelLayout& parent_panel = panels.back();
        panel.x = parent_panel.x + parent_panel.width - 1;
        panel.y = parent_panel.y + 1 + index;
        if (bounds_.width > 0 && panel.x + panel.width > bounds_.width) {
            panel.x = std::max(0, parent_panel.x - panel.width + 1);
        }
        if (bounds_.height > 0 && panel.y + panel.height > bounds_.height) {
            panel.y = std::max(0, bounds_.height - panel.height);
        }
        panels.push_back(panel);
    }

    return panels;
}

void ContextMenu::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (!open_) {
        return;
    }

    const MenuBarLook look = custom_look_ ? look_ : menu_bar_look_classic(ctx.theme);

    const std::vector<MenuPanelLayout> panels = open_panels();
    const std::vector<MenuItem>* items = &items_;
    for (std::size_t depth = 0; depth < panels.size(); ++depth) {
        const int active =
            depth + 1 < panels.size() && depth < submenu_path_.size() ? submenu_path_[depth] : active_item_;
        paint_menu_panel(canvas, panels[depth], *items, active, look);
        if (depth < submenu_path_.size()) {
            const int index = submenu_path_[depth];
            if (index >= 0 && index < static_cast<int>(items->size())) {
                items = &(*items)[static_cast<std::size_t>(index)].children;
            }
        }
    }
}

bool ContextMenu::item_selectable(int index) const {
    const std::vector<MenuItem>* items = current_items();
    if (items == nullptr || index < 0 || index >= static_cast<int>(items->size())) {
        return false;
    }
    return menu_item_selectable((*items)[static_cast<std::size_t>(index)]);
}

void ContextMenu::notify_hint() {
    if (!on_hint_) {
        return;
    }

    const std::vector<MenuItem>* items = current_items();
    if (items == nullptr || active_item_ < 0 || active_item_ >= static_cast<int>(items->size())) {
        on_hint_({});
        return;
    }

    on_hint_(menu_item_hint((*items)[static_cast<std::size_t>(active_item_)]));
}

void ContextMenu::move_selection(int delta) {
    const std::vector<MenuItem>* items = current_items();
    if (items == nullptr) {
        return;
    }

    menu_move_selection(*items, active_item_, delta);
    notify_hint();
    mark_dirty();
}

void ContextMenu::open_submenu() {
    const std::vector<MenuItem>* items = current_items();
    if (items == nullptr || active_item_ < 0 || active_item_ >= static_cast<int>(items->size())) {
        return;
    }

    const MenuItem& item = (*items)[static_cast<std::size_t>(active_item_)];
    if (!menu_item_has_submenu(item) || !item.enabled) {
        return;
    }

    submenu_path_.push_back(active_item_);
    active_item_ = 0;
    if (!item_selectable(active_item_)) {
        move_selection(1);
    } else {
        notify_hint();
    }
    mark_layout_dirty();
}

void ContextMenu::close_submenu() {
    if (submenu_path_.empty()) {
        return;
    }

    active_item_ = submenu_path_.back();
    submenu_path_.pop_back();
    notify_hint();
    mark_layout_dirty();
}

void ContextMenu::activate_item(int index) {
    std::vector<MenuItem>* items = &items_;
    for (int path_index : submenu_path_) {
        if (path_index < 0 || path_index >= static_cast<int>(items->size())) {
            return;
        }
        items = &(*items)[static_cast<std::size_t>(path_index)].children;
    }

    if (index < 0 || index >= static_cast<int>(items->size())) {
        return;
    }

    MenuItem& item = (*items)[static_cast<std::size_t>(index)];
    if (item.kind == MenuItemKind::Separator || !item.enabled) {
        return;
    }

    if (menu_item_has_submenu(item)) {
        active_item_ = index;
        open_submenu();
        return;
    }

    if (item.kind == MenuItemKind::Checkbox) {
        item.checked = !item.checked;
    }

    if (item.action) {
        item.action();
    }
    if (on_action_) {
        on_action_(parse_menu_mnemonic(item.label).text);
    }

    if (item.kind == MenuItemKind::Checkbox) {
        mark_dirty();
        return;
    }

    close();
}

bool ContextMenu::hit_panel(Point local, int& panel_index, int& row) const {
    const std::vector<MenuPanelLayout> panels = open_panels();
    for (int i = static_cast<int>(panels.size()) - 1; i >= 0; --i) {
        const MenuPanelLayout& panel = panels[static_cast<std::size_t>(i)];
        if (local.x >= panel.x && local.x < panel.x + panel.width && local.y >= panel.y &&
            local.y < panel.y + panel.height) {
            panel_index = i;
            row = local.y - panel.y - 1;
            return true;
        }
    }
    return false;
}

bool ContextMenu::handle_event(const Event& event) {
    if (!open_) {
        return false;
    }

    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (mouse->action != MouseAction::Click && mouse->action != MouseAction::Release) {
            return true;
        }

        const Point local{mouse->position.x - bounds_.x, mouse->position.y - bounds_.y};
        int panel_index = 0;
        int row = 0;
        if (!hit_panel(local, panel_index, row)) {
            close();
            return true;
        }

        while (static_cast<int>(submenu_path_.size()) > panel_index) {
            submenu_path_.pop_back();
        }

        if (row >= 0) {
            active_item_ = row;
            if (static_cast<std::size_t>(panel_index) + 1 < open_panels().size()) {
                mark_dirty();
                return true;
            }
            activate_item(row);
        }
        return true;
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key) {
        return true;
    }

    if (key->key == Key::Escape) {
        if (!submenu_path_.empty()) {
            close_submenu();
        } else {
            close();
        }
        return true;
    }

    switch (key->key) {
    case Key::Up: move_selection(-1); return true;
    case Key::Down: move_selection(1); return true;
    case Key::Enter: activate_item(active_item_); return true;
    case Key::Left:
        if (!submenu_path_.empty()) {
            close_submenu();
        } else {
            close();
        }
        return true;
    case Key::Right: {
        const std::size_t depth_before = submenu_path_.size();
        open_submenu();
        return submenu_path_.size() > depth_before;
    }
    default: break;
    }

    return true;
}

} // namespace tuinator
