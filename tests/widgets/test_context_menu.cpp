#include <tuinator/core/event.hpp>
#include <tuinator/widgets/menu/context_menu.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

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

TUINATOR_TEST(context_menu_escape_hides_menu) {
    tuinator::ContextMenu menu;
    menu.set_items({{"Copy", []() {}}});
    menu.layout({0, 0, 30, 8});
    menu.show({1, 1});
    TUINATOR_CHECK(menu.is_open());

    TUINATOR_CHECK(menu.handle_event(tuinator::KeyPress{tuinator::Key::Escape}));
    TUINATOR_CHECK(!menu.is_open());
}
