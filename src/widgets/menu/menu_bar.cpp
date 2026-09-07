#include <tuinator/core/event.hpp>
#include <tuinator/render/text.hpp>
#include <tuinator/widgets/menu/menu_bar.hpp>
#include <tuinator/widgets/menu/menu_common.hpp>

#include <algorithm>
#include <cctype>
#include <string>
#include <variant>

namespace tuinator {

MenuItem::MenuItem(std::string label, std::function<void()> action, std::string shortcut, bool enabled)
    : label(std::move(label)), action(std::move(action)), shortcut(std::move(shortcut)), enabled(enabled) {}

MenuItem MenuItem::separator() {
    MenuItem item;
    item.kind = MenuItemKind::Separator;
    return item;
}

MenuItem MenuItem::checkbox(std::string label, bool checked, std::function<void()> action) {
    MenuItem item;
    item.label = std::move(label);
    item.action = std::move(action);
    item.checked = checked;
    item.kind = MenuItemKind::Checkbox;
    return item;
}

MenuItem MenuItem::submenu(std::string label, std::vector<MenuItem> items, std::string shortcut, std::string icon) {
    MenuItem item;
    item.label = std::move(label);
    item.children = std::move(items);
    item.shortcut = std::move(shortcut);
    item.icon = std::move(icon);
    item.kind = MenuItemKind::Submenu;
    return item;
}

ParsedMenuText parse_menu_mnemonic(std::string_view label) {
    ParsedMenuText parsed;
    for (std::size_t i = 0; i < label.size(); ++i) {
        if (label[i] == '&' && i + 1 < label.size()) {
            parsed.mnemonic = static_cast<char>(std::tolower(static_cast<unsigned char>(label[i + 1])));
            parsed.text.push_back(label[i + 1]);
            ++i;
            continue;
        }
        parsed.text.push_back(label[i]);
    }
    return parsed;
}

namespace {

char normalize_key(char ch) { return static_cast<char>(std::tolower(static_cast<unsigned char>(ch))); }

MenuBarLook current_look(const Style& bar, const Style& active, const Style& submenu, const Style& disabled,
                         const Style& shortcut, const Style& border, BorderGlyphs glyphs) {
    MenuBarLook look;
    look.bar_style = bar;
    look.active_style = active;
    look.submenu_style = submenu;
    look.disabled_style = disabled;
    look.shortcut_style = shortcut;
    look.border_style = border;
    look.glyphs = std::move(glyphs);
    return look;
}

} // namespace

MenuBar::MenuBar(Style style, Style active_style) : style_(style), active_style_(active_style) {
    submenu_style_ = style_;
    submenu_style_.dim = true;
    disabled_style_ = style_;
    disabled_style_.dim = true;
    shortcut_style_ = style_;
    shortcut_style_.dim = true;
    border_style_ = style_;
    border_style_.foreground = Color::Cyan;
}

void MenuBar::apply_look(const MenuBarLook& look) {
    style_ = look.bar_style;
    active_style_ = look.active_style;
    submenu_style_ = look.submenu_style;
    disabled_style_ = look.disabled_style;
    shortcut_style_ = look.shortcut_style;
    border_style_ = look.border_style;
    glyphs_ = look.glyphs;
    mark_dirty();
}

void MenuBar::set_menus(std::vector<Menu> menus) {
    menus_ = std::move(menus);
    close_menu();
    mark_layout_dirty();
}

void MenuBar::set_on_action(std::function<void(const std::string&, const std::string&)> callback) {
    on_action_ = std::move(callback);
}

void MenuBar::set_on_hint(std::function<void(const std::string& hint)> callback) { on_hint_ = std::move(callback); }

void MenuBar::set_border_glyphs(BorderGlyphs glyphs) {
    glyphs_ = std::move(glyphs);
    mark_dirty();
}

const std::vector<MenuItem>* MenuBar::current_items() const {
    if (!open_ || active_menu_ < 0 || active_menu_ >= static_cast<int>(menus_.size())) {
        return nullptr;
    }
    return menu_follow_path(menus_[static_cast<std::size_t>(active_menu_)].items, submenu_path_);
}

int MenuBar::dropdown_rows() const {
    const std::vector<MenuItem>* items = current_items();
    return items == nullptr ? 0 : static_cast<int>(items->size());
}

int MenuBar::dropdown_width_for_menu(int menu_index) const {
    if (menu_index < 0 || menu_index >= static_cast<int>(menus_.size())) {
        return 20;
    }

    int width = menu_panel_width(menus_[static_cast<std::size_t>(menu_index)].items);
    if (open_ && active_menu_ == menu_index) {
        for (const MenuPanelLayout& panel : open_panels()) {
            width = std::max(width, panel.x + panel.width);
        }
    }
    return width;
}

std::vector<MenuPanelLayout> MenuBar::open_panels() const {
    std::vector<MenuPanelLayout> panels;
    if (!open_ || active_menu_ < 0) {
        return panels;
    }

    const DropdownLayout root = dropdown_layout();
    MenuPanelLayout first;
    first.x = root.x;
    first.y = root.y;
    first.width = root.width;
    first.height = root.height;
    panels.push_back(first);

    const std::vector<MenuItem>* items = &menus_[static_cast<std::size_t>(active_menu_)].items;
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
        if (panel.x + panel.width > bounds_.width) {
            panel.x = std::max(0, parent_panel.x - panel.width + 1);
        }
        if (bounds_.height > 0 && panel.y + panel.height > bounds_.height) {
            panel.y = std::max(0, bounds_.height - panel.height);
        }
        panels.push_back(panel);
    }

    return panels;
}

MenuBar::DropdownLayout MenuBar::dropdown_layout() const {
    DropdownLayout layout;
    if (!open_ || active_menu_ < 0) {
        return layout;
    }

    layout.x = menu_anchor_x(active_menu_);
    layout.y = 1;
    layout.width = menu_panel_width(menus_[static_cast<std::size_t>(active_menu_)].items);
    layout.height = dropdown_rows() + 2;

    if (layout.x + layout.width > bounds_.width) {
        layout.x = std::max(0, bounds_.width - layout.width);
    }
    if (bounds_.height > 0 && layout.y + layout.height > bounds_.height) {
        layout.y = std::max(1, bounds_.height - layout.height);
    }

    return layout;
}

int MenuBar::menu_anchor_x(int menu_index) const {
    int x = 0;
    for (int i = 0; i < menu_index; ++i) {
        const ParsedMenuText parsed = parse_menu_mnemonic(menus_[static_cast<std::size_t>(i)].title);
        x += text_display_width(" " + parsed.text + " ") + 1;
    }
    return x;
}

Size MenuBar::preferred_size() const {
    int width = 0;
    for (const Menu& menu : menus_) {
        const ParsedMenuText parsed = parse_menu_mnemonic(menu.title);
        width += text_display_width(parsed.text) + 3;
    }

    for (int i = 0; i < static_cast<int>(menus_.size()); ++i) {
        width = std::max(width, dropdown_width_for_menu(i));
    }

    int height = 1;
    if (open_) {
        height = 1;
        for (const MenuPanelLayout& panel : open_panels()) {
            height = std::max(height, panel.y + panel.height);
        }
    }

    return {std::max(20, width), std::max(1, height)};
}

void MenuBar::paint_menu_label(Canvas& canvas, int x, int y, std::string_view label, const Style& style) const {
    tuinator::paint_menu_label(canvas, x, y, label, style);
}

void MenuBar::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    Style clear_style;
    canvas.fill_rect({0, 0, bounds_.width, bounds_.height}, ' ', clear_style);

    int x = 0;
    for (int i = 0; i < static_cast<int>(menus_.size()); ++i) {
        const Menu& menu = menus_[static_cast<std::size_t>(i)];
        const ParsedMenuText parsed = parse_menu_mnemonic(menu.title);
        const std::string padded = " " + parsed.text + " ";
        const bool active = open_ && i == active_menu_;
        const Style& style = active ? active_style_ : style_;
        paint_menu_label(canvas, x + 1, 0, menu.title, style);
        x += text_display_width(padded) + 1;
    }

    if (!open_ || active_menu_ < 0) {
        return;
    }

    paint_dropdown(canvas, dropdown_layout());
}

void MenuBar::paint_dropdown(Canvas& canvas, const DropdownLayout& /*layout*/) const {
    const MenuBarLook look =
        current_look(style_, active_style_, submenu_style_, disabled_style_, shortcut_style_, border_style_, glyphs_);

    const std::vector<MenuPanelLayout> panels = open_panels();
    const std::vector<MenuItem>* items = &menus_[static_cast<std::size_t>(active_menu_)].items;
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

bool MenuBar::item_selectable(int index) const {
    const std::vector<MenuItem>* items = current_items();
    if (items == nullptr || index < 0 || index >= static_cast<int>(items->size())) {
        return false;
    }
    return menu_item_selectable((*items)[static_cast<std::size_t>(index)]);
}

void MenuBar::notify_hint() {
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

void MenuBar::move_selection(int delta) {
    const std::vector<MenuItem>* items = current_items();
    if (items == nullptr) {
        return;
    }

    menu_move_selection(*items, active_item_, delta);
    notify_hint();
    mark_dirty();
}

bool MenuBar::activate_mnemonic(char ch, bool alt_pressed) {
    ch = normalize_key(ch);
    if (ch == '\0') {
        return false;
    }

    if (!open_) {
        for (int i = 0; i < static_cast<int>(menus_.size()); ++i) {
            const ParsedMenuText parsed = parse_menu_mnemonic(menus_[static_cast<std::size_t>(i)].title);
            if (parsed.mnemonic == ch) {
                if (!alt_pressed && !is_focused()) {
                    continue;
                }
                open_menu(i);
                return true;
            }
        }
        return false;
    }

    const std::vector<MenuItem>* items = current_items();
    if (items == nullptr) {
        return false;
    }

    for (int i = 0; i < static_cast<int>(items->size()); ++i) {
        const MenuItem& item = (*items)[static_cast<std::size_t>(i)];
        if (!item_selectable(i) || !item.enabled) {
            continue;
        }
        const ParsedMenuText parsed = parse_menu_mnemonic(item.label);
        if (parsed.mnemonic == ch) {
            active_item_ = i;
            activate_item(i);
            return true;
        }
    }

    return false;
}

bool MenuBar::handle_event(const Event& event) {
    if (menus_.empty()) {
        return false;
    }

    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (mouse->action != MouseAction::Click && mouse->action != MouseAction::Release) {
            return false;
        }

        if (!contains_point(mouse->position)) {
            close_menu();
            return false;
        }

        const Point local{mouse->position.x - bounds_.x, mouse->position.y - bounds_.y};
        if (local.y == 0) {
            int x = 0;
            for (int i = 0; i < static_cast<int>(menus_.size()); ++i) {
                const ParsedMenuText parsed = parse_menu_mnemonic(menus_[static_cast<std::size_t>(i)].title);
                const int width = text_display_width(" " + parsed.text + " ") + 1;
                if (local.x >= x && local.x < x + width) {
                    if (open_ && active_menu_ == i) {
                        close_menu();
                    } else {
                        open_menu(i);
                    }
                    return true;
                }
                x += width;
            }
            return false;
        }

        if (open_) {
            const std::vector<MenuPanelLayout> panels = open_panels();
            for (std::size_t depth = 0; depth < panels.size(); ++depth) {
                const MenuPanelLayout& panel = panels[depth];
                if (local.x >= panel.x && local.x < panel.x + panel.width && local.y >= panel.y &&
                    local.y < panel.y + panel.height) {
                    const int row = local.y - panel.y - 1;
                    if (row >= 0) {
                        while (submenu_path_.size() > depth) {
                            submenu_path_.pop_back();
                        }
                        active_item_ = row;
                        if (depth < panels.size() - 1) {
                            mark_dirty();
                            return true;
                        }
                        activate_item(row);
                    }
                    return true;
                }
            }
        }

        return false;
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key) {
        return false;
    }

    if (key->key == Key::Escape && open_) {
        if (!submenu_path_.empty()) {
            close_submenu();
            return true;
        }
        close_menu();
        return true;
    }

    if (key->alt && key->character >= 'a' && key->character <= 'z') {
        if (activate_mnemonic(key->character, true)) {
            return true;
        }
    }
    if (key->alt && key->character >= 'A' && key->character <= 'Z') {
        if (activate_mnemonic(key->character, true)) {
            return true;
        }
    }

    if ((key->character >= 'a' && key->character <= 'z') || (key->character >= 'A' && key->character <= 'Z')) {
        if (!is_focused() && !open_) {
            return false;
        }
        if (activate_mnemonic(key->character, false)) {
            return true;
        }
    }

    if (!is_focused()) {
        return false;
    }

    if (!open_) {
        if (key->key == Key::Right) {
            const int next = active_menu_ < 0 ? 0 : (active_menu_ + 1) % static_cast<int>(menus_.size());
            open_menu(next);
            return true;
        }
        if (key->key == Key::Left) {
            const int next = active_menu_ < 0 ? static_cast<int>(menus_.size()) - 1
                                              : (active_menu_ - 1 + static_cast<int>(menus_.size())) %
                                                    static_cast<int>(menus_.size());
            open_menu(next);
            return true;
        }
        if (key->key == Key::Enter || key->key == Key::Down) {
            open_menu(active_menu_ < 0 ? 0 : active_menu_);
            return true;
        }
        return false;
    }

    switch (key->key) {
    case Key::Up: move_selection(-1); return true;
    case Key::Down: move_selection(1); return true;
    case Key::Enter: activate_item(active_item_); return true;
    case Key::Left:
        if (!submenu_path_.empty()) {
            close_submenu();
        } else {
            close_menu();
        }
        return true;
    case Key::Right: {
        const std::size_t depth_before = submenu_path_.size();
        open_submenu();
        if (submenu_path_.size() > depth_before) {
            return true;
        }
        const int next = (active_menu_ + 1) % static_cast<int>(menus_.size());
        open_menu(next);
        return true;
    }
    default: break;
    }

    return false;
}

void MenuBar::close_menu() {
    if (!open_) {
        return;
    }

    open_ = false;
    active_menu_ = -1;
    active_item_ = 0;
    submenu_path_.clear();
    notify_hint();
    mark_layout_dirty();
}

void MenuBar::open_menu(int index) {
    if (index < 0 || index >= static_cast<int>(menus_.size())) {
        return;
    }

    open_ = true;
    active_menu_ = index;
    active_item_ = 0;
    submenu_path_.clear();
    if (!item_selectable(active_item_)) {
        move_selection(1);
    } else {
        notify_hint();
    }
    mark_layout_dirty();
}

void MenuBar::open_submenu() {
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

void MenuBar::close_submenu() {
    if (submenu_path_.empty()) {
        return;
    }

    active_item_ = submenu_path_.back();
    submenu_path_.pop_back();
    notify_hint();
    mark_layout_dirty();
}

void MenuBar::activate_item(int index) {
    if (active_menu_ < 0 || active_menu_ >= static_cast<int>(menus_.size())) {
        return;
    }

    std::vector<MenuItem>* items = &menus_[static_cast<std::size_t>(active_menu_)].items;
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
        const Menu& menu = menus_[static_cast<std::size_t>(active_menu_)];
        on_action_(parse_menu_mnemonic(menu.title).text, parse_menu_mnemonic(item.label).text);
    }

    if (item.kind == MenuItemKind::Checkbox) {
        mark_dirty();
        return;
    }

    close_menu();
}

} // namespace tuinator
