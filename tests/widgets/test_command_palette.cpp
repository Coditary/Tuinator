#include <tuinator/core/event.hpp>
#include <tuinator/widgets/menu/command_palette.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

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

TUINATOR_TEST(command_palette_escape_closes) {
    tuinator::CommandPalette palette;
    palette.set_entries({{"quit", "Quit", "App", "Ctrl+Q", []() {}}});
    palette.layout({0, 0, 40, 10});
    palette.open();
    TUINATOR_CHECK(palette.is_open());

    TUINATOR_CHECK(palette.handle_event(tuinator::KeyPress{tuinator::Key::Escape}));
    TUINATOR_CHECK(!palette.is_open());
}
