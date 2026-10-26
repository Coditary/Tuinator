#include <tuinator/core/event.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/chrome/status_bar.hpp>
#include <tuinator/widgets/display/big_text.hpp>
#include <tuinator/widgets/display/label.hpp>
#include <tuinator/widgets/display/progress_bar.hpp>
#include <tuinator/widgets/display/separator.hpp>
#include <tuinator/widgets/display/throbber.hpp>
#include <tuinator/widgets/menu/command_palette.hpp>
#include <tuinator/widgets/menu/context_menu.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(edge_label_empty_string_renders) {
    tuinator::MemoryTerminalBackend backend({8, 2});
    backend.init();
    tuinator::Label label("");
    label.layout({0, 0, 8, 1});
    tuinator::test::render_root(label, backend);
    TUINATOR_CHECK(true);
}

TUINATOR_TEST(edge_separator_zero_width_paint_safe) {
    tuinator::MemoryTerminalBackend backend({4, 2});
    backend.init();
    tuinator::Separator sep;
    sep.layout({0, 0, 0, 1});
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    sep.paint(ctx);
    TUINATOR_CHECK(true);
}

TUINATOR_TEST(edge_status_bar_empty_text) {
    tuinator::StatusBar bar;
    bar.set_text("");
    bar.layout({0, 0, 20, 1});
    tuinator::MemoryTerminalBackend backend({20, 1});
    backend.init();
    tuinator::test::render_root(bar, backend);
    TUINATOR_CHECK(true);
}

TUINATOR_TEST(edge_progress_bar_value_clamps_beyond_unit_interval) {
    tuinator::ProgressBar bar;
    bar.set_value(1.5);
    TUINATOR_CHECK(bar.value() <= 1.0);
    bar.set_value(-0.25);
    TUINATOR_CHECK(bar.value() >= 0.0);
}

TUINATOR_TEST(edge_progress_bar_indeterminate_zero_width_safe) {
    tuinator::MemoryTerminalBackend backend({4, 2});
    backend.init();
    tuinator::ProgressBar bar;
    bar.set_options(tuinator::ProgressBarOptions{.layout = tuinator::ProgressBarLayout::Pulse});
    bar.layout({0, 0, 0, 1});
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    bar.paint(ctx);
    TUINATOR_CHECK(true);
}

TUINATOR_TEST(edge_throbber_paint_with_minimal_bounds) {
    tuinator::MemoryTerminalBackend backend({6, 2});
    backend.init();
    tuinator::Throbber throbber("dots");
    throbber.layout({0, 0, 2, 1});
    tuinator::test::render_root(throbber, backend);
    TUINATOR_CHECK(true);
}

TUINATOR_TEST(edge_big_text_single_character) {
    tuinator::MemoryTerminalBackend backend({12, 10});
    backend.init();
    tuinator::BigText big("!", tuinator::BigTextKind::Block);
    big.layout({0, 0, 12, 10});
    tuinator::test::render_root(big, backend);
    TUINATOR_CHECK(big.preferred_size().height >= 7);
}

TUINATOR_TEST(edge_command_palette_query_with_no_matches) {
    tuinator::CommandPalette palette;
    palette.set_entries({{"open", "Open File", "File", "Ctrl+O", []() {}}});
    palette.layout({0, 0, 40, 10});
    palette.open();
    palette.handle_event(tuinator::KeyPress{.character = 'z'});
    palette.handle_event(tuinator::KeyPress{.character = 'z'});

    tuinator::MemoryTerminalBackend backend({40, 10});
    backend.init();
    tuinator::test::render_root(palette, backend);
    TUINATOR_CHECK(!tuinator::test::row_has(backend, "Open File"));
}

TUINATOR_TEST(edge_context_menu_show_off_screen_clamps_paint) {
    tuinator::MemoryTerminalBackend backend({20, 6});
    backend.init();
    tuinator::ContextMenu menu;
    menu.set_items({{"Action", []() {}}});
    menu.layout({0, 0, 20, 6});
    menu.show({100, 100});
    tuinator::test::render_root(menu, backend);
    TUINATOR_CHECK(menu.is_open());
}
