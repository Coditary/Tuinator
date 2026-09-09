#include <tuinator/core/action_registry.hpp>
#include <tuinator/layout/box.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/capabilities.hpp>
#include <tuinator/widgets/controls/text_input.hpp>
#include <tuinator/widgets/menu/command_palette.hpp>
#include <tuinator/widgets/menu/context_menu.hpp>
#include <tuinator/widgets/menu/menu_bar.hpp>
#include <tuinator/widgets/menu/menu_common.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(menu_mnemonic_parsing) {
    const tuinator::ParsedMenuText parsed = tuinator::parse_menu_mnemonic("&File");
    TUINATOR_CHECK_EQ(parsed.text, "File");
    TUINATOR_CHECK_EQ(parsed.mnemonic, 'f');
}

TUINATOR_TEST(menu_bar_dropdown_renders_box) {
    tuinator::MemoryTerminalBackend backend({40, 10});
    backend.init();

    tuinator::MenuBar menu(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    menu.set_menus({{"&File",
                     {
                         {"&Open", []() {}, "Ctrl+O"},
                         tuinator::MenuItem::separator(),
                         {"&Quit", []() {}},
                     }}});
    menu.layout({0, 0, 40, 6});
    menu.set_focused(true);

    tuinator::KeyPress open{};
    open.key = tuinator::Key::Down;
    menu.handle_event(open);

    tuinator::test::render_root(menu, backend);

    bool found_corner = false;
    for (int y = 0; y < backend.terminal_size().height; ++y) {
        for (int x = 0; x < backend.terminal_size().width; ++x) {
            if (tuinator::test::cell_at(backend, x, y) == '+') {
                found_corner = true;
            }
        }
    }
    TUINATOR_CHECK(found_corner);
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 2, "Ctrl+O"));
}

TUINATOR_TEST(menu_item_separator_factory) {
    const tuinator::MenuItem item = tuinator::MenuItem::separator();
    TUINATOR_CHECK(item.kind == tuinator::MenuItemKind::Separator);
}

TUINATOR_TEST(menu_submenu_factory_has_children) {
    const tuinator::MenuItem item = tuinator::MenuItem::submenu("&Export", {
                                                                               {"PDF", []() {}},
                                                                               {"PNG", []() {}},
                                                                           });
    TUINATOR_CHECK(item.kind == tuinator::MenuItemKind::Submenu);
    TUINATOR_CHECK_EQ(item.children.size(), 2U);
}

TUINATOR_TEST(menu_bar_submenu_opens_to_the_right) {
    tuinator::MemoryTerminalBackend backend({50, 12});
    backend.init();

    tuinator::MenuBar menu(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    menu.set_menus({{"&File",
                     {
                         tuinator::MenuItem::submenu("&Export", {{"PDF", []() {}}, {"PNG", []() {}}}),
                     }}});
    menu.layout({0, 0, 50, 10});
    menu.set_focused(true);

    tuinator::KeyPress open{};
    open.key = tuinator::Key::Down;
    menu.handle_event(open);

    tuinator::KeyPress right{};
    right.key = tuinator::Key::Right;
    menu.handle_event(right);

    tuinator::test::render_root(menu, backend);

    bool found_pdf = false;
    for (int y = 0; y < backend.terminal_size().height; ++y) {
        if (tuinator::test::row_contains(backend, y, "PDF")) {
            found_pdf = true;
        }
    }
    TUINATOR_CHECK(found_pdf);
}

TUINATOR_TEST(action_registry_matches_ctrl_shortcut) {
    tuinator::ActionRegistry registry;
    bool fired = false;
    tuinator::RegisteredAction action;
    action.id = "save";
    action.shortcut = "Ctrl+S";
    action.callback = [&fired]() { fired = true; };
    registry.register_action(std::move(action));

    tuinator::KeyPress key{};
    key.ctrl = true;
    key.character = 's';
    TUINATOR_CHECK(registry.handle_key(key));
    TUINATOR_CHECK(fired);
}

TUINATOR_TEST(command_palette_filters_entries) {
    tuinator::CommandPalette palette;
    palette.set_entries({
        {"new", "New File", "File", "Ctrl+N", []() {}},
        {"open", "Open File", "File", "Ctrl+O", []() {}},
        {"quit", "Quit", "App", "Ctrl+Q", []() {}},
    });
    palette.layout({0, 0, 60, 12});
    palette.open();

    tuinator::KeyPress key{};
    key.character = 'q';
    palette.handle_event(key);

    tuinator::MemoryTerminalBackend backend({60, 12});
    backend.init();
    tuinator::test::render_root(palette, backend);
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 3, "Quit"));
}

TUINATOR_TEST(context_menu_renders_at_anchor) {
    tuinator::MemoryTerminalBackend backend({40, 10});
    backend.init();

    tuinator::ContextMenu menu;
    menu.set_items({{"Copy", []() {}}, {"Paste", []() {}}});
    menu.layout({0, 0, 40, 10});
    menu.show({2, 2});

    tuinator::test::render_root(menu, backend);
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 3, "Copy"));
}

TUINATOR_TEST(menu_bar_escape_closes_open_menu_while_other_widget_focused) {
    tuinator::VBox root;
    auto menu = std::make_unique<tuinator::MenuBar>(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    menu->set_menus({{"&File", {{"&Open", []() {}}}}});
    auto input = std::make_unique<tuinator::TextInput>();
    input->set_focused(true);
    root.add_child(std::move(menu));
    root.add_child(std::move(input));
    root.layout({0, 0, 40, 8});

    tuinator::MouseEvent click{};
    click.action = tuinator::MouseAction::Click;
    click.position = {3, 0};
    TUINATOR_CHECK(root.handle_event(click));
    TUINATOR_CHECK(tuinator::find_keyboard_capture_widget(&root) != nullptr);

    tuinator::KeyPress escape{};
    escape.key = tuinator::Key::Escape;
    TUINATOR_CHECK(tuinator::dispatch_keyboard_capture(&root, escape));
    TUINATOR_CHECK(tuinator::find_keyboard_capture_widget(&root) == nullptr);
}
