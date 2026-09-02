#include <tuinator/widgets/menu_common.hpp>

#include <tuinator/render/text.hpp>

#include <algorithm>
#include <cctype>

namespace tuinator {

namespace {

char normalize_key(char ch) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
}

} // namespace

MenuBarLook menu_bar_look_classic(const Theme& theme) {
    MenuBarLook look;
    look.id = "classic";
    look.title = "Classic";
    look.bar_style = theme.label;
    look.active_style = theme.button_focused;
    look.submenu_style = theme.label;
    look.submenu_style.dim = true;
    look.disabled_style = theme.muted;
    look.disabled_style.dim = true;
    look.shortcut_style = theme.muted;
    look.shortcut_style.dim = true;
    look.border_style = theme.border;
    look.border_style.foreground = Color::Cyan;
    look.glyphs = theme.glyphs;
    return look;
}

MenuBarLook menu_bar_look_mac(const Theme& theme) {
    MenuBarLook look = menu_bar_look_classic(theme);
    look.id = "mac";
    look.title = "Mac";
    look.aliases = {"osx", "apple"};
    look.bar_style.bold = true;
    look.active_style.reverse = true;
    look.glyphs = unicode_rounded_border_glyphs();
    return look;
}

MenuBarLook menu_bar_look_minimal(const Theme& theme) {
    MenuBarLook look = menu_bar_look_classic(theme);
    look.id = "minimal";
    look.title = "Minimal";
    look.aliases = {"plain", "ascii"};
    look.border_style.dim = true;
    look.glyphs = ascii_border_glyphs();
    look.submenu_style.dim = true;
    return look;
}

const std::vector<MenuBarLook>& all_menu_bar_looks() {
    static const std::vector<MenuBarLook> looks = {
        menu_bar_look_classic(dark_theme()),
        menu_bar_look_mac(dark_theme()),
        menu_bar_look_minimal(dark_theme()),
    };
    return looks;
}

const MenuBarLook* menu_bar_look_named(std::string_view name) {
    const auto equals = [&](const MenuBarLook& look) {
        if (look.id == name || look.title == name) {
            return true;
        }
        for (const std::string& alias : look.aliases) {
            if (alias == name) {
                return true;
            }
        }
        return false;
    };

    for (const MenuBarLook& look : all_menu_bar_looks()) {
        if (equals(look)) {
            return &look;
        }
    }
    return nullptr;
}

bool menu_item_selectable(const MenuItem& item) {
    return item.kind != MenuItemKind::Separator;
}

bool menu_item_has_submenu(const MenuItem& item) {
    return item.kind == MenuItemKind::Submenu || !item.children.empty();
}

std::string menu_item_hint(const MenuItem& item) {
    if (!item.hint.empty()) {
        return item.hint;
    }
    if (menu_item_has_submenu(item)) {
        return "Open submenu";
    }
    if (item.kind == MenuItemKind::Checkbox) {
        return item.checked ? "Toggle off" : "Toggle on";
    }
    return {};
}

int menu_panel_width(const std::vector<MenuItem>& items) {
    int width = 12;
    for (const MenuItem& item : items) {
        if (item.kind == MenuItemKind::Separator) {
            width = std::max(width, 10);
            continue;
        }

        const ParsedMenuText parsed = parse_menu_mnemonic(item.label);
        int row = 4 + text_display_width(parsed.text);
        if (!item.icon.empty()) {
            row += text_display_width(item.icon) + 1;
        }
        if (item.kind == MenuItemKind::Checkbox) {
            row += 2;
        }
        if (menu_item_has_submenu(item)) {
            row += 2;
        }
        if (!item.shortcut.empty()) {
            row += 2 + text_display_width(item.shortcut);
        }
        width = std::max(width, row);
    }

    return width + 2;
}

void menu_move_selection(const std::vector<MenuItem>& items, int& active_item, int delta) {
    const int count = static_cast<int>(items.size());
    if (count == 0) {
        return;
    }

    int next = active_item;
    for (int step = 0; step < count; ++step) {
        next = (next + delta + count) % count;
        if (menu_item_selectable(items[static_cast<std::size_t>(next)])) {
            active_item = next;
            return;
        }
    }
}

const std::vector<MenuItem>* menu_follow_path(
    const std::vector<MenuItem>& root,
    const std::vector<int>& path) {
    const std::vector<MenuItem>* items = &root;
    for (int index : path) {
        if (index < 0 || index >= static_cast<int>(items->size())) {
            return nullptr;
        }
        const MenuItem& item = (*items)[static_cast<std::size_t>(index)];
        if (item.children.empty()) {
            return nullptr;
        }
        items = &item.children;
    }
    return items;
}

void paint_menu_label(Canvas& canvas, int x, int y, std::string_view label, const Style& style) {
    const ParsedMenuText parsed = parse_menu_mnemonic(label);
    if (parsed.mnemonic == '\0') {
        canvas.draw_text({x, y}, parsed.text, style);
        return;
    }

    int column = x;
    for (std::size_t i = 0; i < parsed.text.size(); ++i) {
        const char ch = parsed.text[i];
        Style glyph_style = style;
        if (normalize_key(ch) == parsed.mnemonic) {
            glyph_style.bold = true;
        }
        canvas.draw_text({column, y}, std::string(1, ch), glyph_style);
        column += text_display_width(std::string(1, ch));
    }
}

void paint_menu_panel(
    Canvas& canvas,
    const MenuPanelLayout& layout,
    const std::vector<MenuItem>& items,
    int active_item,
    const MenuBarLook& look) {
    if (layout.width <= 0 || layout.height <= 0) {
        return;
    }

    const Rect box{layout.x, layout.y, layout.width, layout.height};
    canvas.draw_box(box, look.border_style, look.glyphs);

    const int inner_width = std::max(0, layout.width - 2);
    for (int row = 0; row < static_cast<int>(items.size()); ++row) {
        const MenuItem& item = items[static_cast<std::size_t>(row)];
        const int y = layout.y + 1 + row;

        if (item.kind == MenuItemKind::Separator) {
            Style line_style = look.border_style;
            line_style.dim = true;
            canvas.draw_text(
                {layout.x + 1, y},
                std::string(static_cast<std::size_t>(inner_width), look.glyphs.horizontal[0]),
                line_style);
            continue;
        }

        const bool selected = row == active_item;
        Style row_style = selected ? look.active_style : look.submenu_style;
        if (!item.enabled) {
            row_style = look.disabled_style;
        }

        int column = layout.x + 1;
        std::string prefix = selected ? "> " : "  ";
        if (item.kind == MenuItemKind::Checkbox) {
            prefix += item.checked ? "[x] " : "[ ] ";
        }
        canvas.draw_text({column, y}, prefix, row_style);
        column += text_display_width(prefix);

        if (!item.icon.empty()) {
            canvas.draw_text({column, y}, item.icon + " ", row_style);
            column += text_display_width(item.icon) + 1;
        }

        paint_menu_label(canvas, column, y, item.label, row_style);
        column += text_display_width(parse_menu_mnemonic(item.label).text);

        if (!item.shortcut.empty()) {
            const int shortcut_x = layout.x + layout.width - 1 - text_display_width(item.shortcut);
            canvas.draw_text({shortcut_x, y}, item.shortcut, look.shortcut_style);
        } else if (menu_item_has_submenu(item)) {
            const int arrow_x = layout.x + layout.width - 2;
            canvas.draw_text({arrow_x, y}, ">", row_style);
        }
    }
}

} // namespace tuinator
