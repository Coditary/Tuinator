#pragma once

#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/widgets/widget.hpp>

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace tuinator {

enum class MenuItemKind {
    Action,
    Separator,
    Checkbox,
    Submenu,
};

struct MenuItem {
    std::string label;
    std::function<void()> action;
    std::string shortcut;
    std::string icon;
    std::string hint;
    std::vector<MenuItem> children;
    bool enabled = true;
    bool checked = false;
    MenuItemKind kind = MenuItemKind::Action;

    MenuItem() = default;
    MenuItem(std::string label, std::function<void()> action = {}, std::string shortcut = {}, bool enabled = true);

    static MenuItem separator();
    static MenuItem checkbox(std::string label, bool checked = false, std::function<void()> action = {});
    static MenuItem submenu(
        std::string label,
        std::vector<MenuItem> items,
        std::string shortcut = {},
        std::string icon = {});
};

struct Menu {
    std::string title;
    std::vector<MenuItem> items;
};

struct ParsedMenuText {
    std::string text;
    char mnemonic = '\0';
};

ParsedMenuText parse_menu_mnemonic(std::string_view label);

struct MenuBarLook;

struct MenuPanelLayout;

class MenuBar : public Widget {
public:
    explicit MenuBar(Style style = {}, Style active_style = {});

    void set_menus(std::vector<Menu> menus);
    void set_on_action(std::function<void(const std::string& menu, const std::string& item)> callback);
    void set_on_hint(std::function<void(const std::string& hint)> callback);
    void set_border_glyphs(BorderGlyphs glyphs);
    void apply_look(const MenuBarLook& look);

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return true; }

private:
    struct DropdownLayout {
        int x = 0;
        int y = 1;
        int width = 0;
        int height = 0;
    };

    int dropdown_rows() const;
    int dropdown_width_for_menu(int menu_index) const;
    DropdownLayout dropdown_layout() const;
    int menu_anchor_x(int menu_index) const;

    const std::vector<MenuItem>* current_items() const;
    void close_menu();
    void open_menu(int index);
    void open_submenu();
    void close_submenu();
    void activate_item(int index);
    void move_selection(int delta);
    bool activate_mnemonic(char ch, bool alt_pressed);
    bool item_selectable(int index) const;
    void notify_hint();
    std::vector<MenuPanelLayout> open_panels() const;

    void paint_dropdown(Canvas& canvas, const DropdownLayout& layout) const;
    void paint_menu_label(Canvas& canvas, int x, int y, std::string_view label, const Style& style) const;

    std::vector<Menu> menus_;
    int active_menu_ = -1;
    int active_item_ = 0;
    std::vector<int> submenu_path_;
    bool open_ = false;
    Style style_;
    Style active_style_;
    Style submenu_style_;
    Style disabled_style_;
    Style shortcut_style_;
    Style border_style_;
    BorderGlyphs glyphs_ = ascii_border_glyphs();
    std::function<void(const std::string&, const std::string&)> on_action_;
    std::function<void(const std::string&)> on_hint_;
};

} // namespace tuinator
