#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/terminal/ansi_terminal_buffer.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(ansi_terminal_buffer_renders_colored_text) {
    tuinator::AnsiTerminalBuffer buffer;
    buffer.resize({24, 3});
    buffer.feed("\033[31mhello\033[0m world");

    tuinator::MemoryTerminalBackend backend({24, 3});
    backend.init();
    backend.begin_frame();
    tuinator::Canvas canvas(backend);
    const tuinator::Theme theme = tuinator::dark_theme();
    tuinator::PaintContext ctx{canvas, theme};
    buffer.paint(ctx, {0, 0});
    backend.end_frame();

    TUINATOR_CHECK(tuinator::test::row_has(backend, "hello"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "world"));
}

TUINATOR_TEST(ansi_terminal_buffer_handles_carriage_return) {
    tuinator::AnsiTerminalBuffer buffer;
    buffer.resize({16, 2});
    buffer.feed("prompt> \rprompt> ok");

    tuinator::MemoryTerminalBackend backend({16, 2});
    backend.init();
    backend.begin_frame();
    tuinator::Canvas canvas(backend);
    const tuinator::Theme theme = tuinator::dark_theme();
    tuinator::PaintContext ctx{canvas, theme};
    buffer.paint(ctx, {0, 0});
    backend.end_frame();

    TUINATOR_CHECK(tuinator::test::row_has(backend, "prompt> ok"));
}

TUINATOR_TEST(ansi_terminal_buffer_keeps_default_background_transparent) {
    tuinator::AnsiTerminalBuffer buffer;
    buffer.resize({8, 1});
    buffer.feed("hello");

    std::string glyph;
    tuinator::Style style{};
    TUINATOR_CHECK(buffer.cell_at(0, 0, glyph, style));
    TUINATOR_CHECK_EQ(glyph, "h");
    TUINATOR_CHECK_EQ(style.background, tuinator::Color::Default);
    TUINATOR_CHECK(!style.background_rgb.has_value());
}

TUINATOR_TEST(ansi_terminal_buffer_reports_cursor_after_echoed_input) {
    tuinator::AnsiTerminalBuffer buffer;
    buffer.resize({8, 1});
    buffer.feed("ab");

    int row = 0;
    int col = 0;
    bool visible = false;
    buffer.cursor_position(row, col, visible);
    TUINATOR_CHECK(visible);
    TUINATOR_CHECK_EQ(row, 0);
    TUINATOR_CHECK_EQ(col, 2);
}

TUINATOR_TEST(ansi_terminal_buffer_tracks_cursor_callbacks) {
    tuinator::AnsiTerminalBuffer buffer;
    buffer.resize({8, 1});
    buffer.feed("\033[5C"); // cursor forward 5

    int row = 0;
    int col = 0;
    bool visible = false;
    buffer.cursor_position(row, col, visible);
    TUINATOR_CHECK_EQ(col, 5);

    buffer.feed("\033[?25l"); // hide cursor
    buffer.cursor_position(row, col, visible);
    TUINATOR_CHECK(!visible);

    buffer.feed("\033[?25h"); // show cursor
    buffer.cursor_position(row, col, visible);
    TUINATOR_CHECK(visible);
}

TUINATOR_TEST(ansi_terminal_buffer_uses_rgb_for_bold_blue) {
    tuinator::AnsiTerminalBuffer buffer;
    buffer.resize({4, 1});
    buffer.feed("\033[34mA\033[1;34mB\033[0m");

    tuinator::Style normal{};
    tuinator::Style bold{};
    std::string glyph;
    TUINATOR_CHECK(buffer.cell_at(0, 0, glyph, normal));
    TUINATOR_CHECK(buffer.cell_at(0, 1, glyph, bold));
    TUINATOR_CHECK(normal.foreground_rgb.has_value());
    TUINATOR_CHECK(bold.foreground_rgb.has_value());

    const int normal_luma = normal.foreground_rgb->r + normal.foreground_rgb->g + normal.foreground_rgb->b;
    const int bold_luma = bold.foreground_rgb->r + bold.foreground_rgb->g + bold.foreground_rgb->b;
    TUINATOR_CHECK(bold_luma > normal_luma);
}

TUINATOR_TEST(ansi_terminal_buffer_paints_colored_cells_at_grid_columns) {
    tuinator::AnsiTerminalBuffer buffer;
    buffer.resize({12, 1});
    buffer.feed("\033[41mA\033[0m\033[42mB\033[0m");

    tuinator::MemoryTerminalBackend backend({12, 1});
    backend.init();
    backend.begin_frame();
    tuinator::Canvas canvas(backend);
    const tuinator::Theme theme = tuinator::dark_theme();
    tuinator::PaintContext ctx{canvas, theme};
    buffer.paint(ctx, {0, 0});
    backend.end_frame();

    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 0, 0), 'A');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 1, 0), 'B');
}
