#pragma once

#include <tuinator/render/canvas.hpp>
#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/menu_bar.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

struct MenuBarLook {
    const char* id = "classic";
    const char* title = "Classic";
    Style bar_style{};
    Style active_style{};
    Style submenu_style{};
    Style disabled_style{};
    Style shortcut_style{};
    Style border_style{};
    BorderGlyphs glyphs = ascii_border_glyphs();
    std::vector<std::string> aliases;
};

struct MenuPanelLayout {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

const std::vector<MenuBarLook>& all_menu_bar_looks();
const MenuBarLook* menu_bar_look_named(std::string_view name);
MenuBarLook menu_bar_look_classic(const Theme& theme);
MenuBarLook menu_bar_look_mac(const Theme& theme);
MenuBarLook menu_bar_look_minimal(const Theme& theme);

bool menu_item_selectable(const MenuItem& item);
bool menu_item_has_submenu(const MenuItem& item);
std::string menu_item_hint(const MenuItem& item);
int menu_panel_width(const std::vector<MenuItem>& items);
void menu_move_selection(const std::vector<MenuItem>& items, int& active_item, int delta);

const std::vector<MenuItem>* menu_follow_path(
    const std::vector<MenuItem>& root,
    const std::vector<int>& path);

void paint_menu_label(Canvas& canvas, int x, int y, std::string_view label, const Style& style);

void paint_menu_panel(
    Canvas& canvas,
    const MenuPanelLayout& layout,
    const std::vector<MenuItem>& items,
    int active_item,
    const MenuBarLook& look);

} // namespace tuinator
