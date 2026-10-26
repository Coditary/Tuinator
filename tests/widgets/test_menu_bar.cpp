#include <tuinator/layout/box.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/capabilities.hpp>
#include <tuinator/widgets/controls/text_input.hpp>
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

TUINATOR_TEST(menu_bar_preferred_size_matches_menu_labels) {
    tuinator::MenuBar menu(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    menu.set_menus({{"File", {{"Open", []() {}, "Ctrl+O"}}}});
    const tuinator::Size size = menu.preferred_size();
    TUINATOR_CHECK(size.width >= 8);
    TUINATOR_CHECK_EQ(size.height, 1);
}

TUINATOR_TEST(menu_bar_enter_activates_highlighted_item) {
    bool opened = false;
    tuinator::MenuBar menu(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    menu.set_menus({{"&File", {{"&Open", [&opened]() { opened = true; }, "Ctrl+O"}}}});
    menu.layout({0, 0, 40, 8});
    menu.set_focused(true);

    tuinator::KeyPress down{};
    down.key = tuinator::Key::Down;
    TUINATOR_CHECK(menu.handle_event(down));

    TUINATOR_CHECK(menu.handle_event(tuinator::KeyPress{tuinator::Key::Enter}));
    TUINATOR_CHECK(opened);
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
