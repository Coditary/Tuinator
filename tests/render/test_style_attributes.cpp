#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/terminal/ansi_terminal_buffer.hpp>

#include "test_harness.hpp"

TUINATOR_TEST(style_text_attributes_default_to_off) {
    const tuinator::Style style{};
    TUINATOR_CHECK(!style.italic);
    TUINATOR_CHECK(!style.underline);
    TUINATOR_CHECK(!style.strikethrough);
}

TUINATOR_TEST(memory_backend_preserves_text_attributes) {
    tuinator::MemoryTerminalBackend backend({8, 1});
    backend.init();
    backend.begin_frame();

    tuinator::Style style{};
    style.italic = true;
    style.underline = true;
    style.strikethrough = true;
    backend.draw_text(0, 0, "abc", style);
    backend.end_frame();

    const auto& cell = backend.cells()[0][1];
    TUINATOR_CHECK(cell.style.italic);
    TUINATOR_CHECK(cell.style.underline);
    TUINATOR_CHECK(cell.style.strikethrough);
}

TUINATOR_TEST(ansi_buffer_maps_vterm_text_attributes) {
    tuinator::AnsiTerminalBuffer buffer;
    buffer.resize({8, 1});
    buffer.feed("\033[3mi\033[4mu\033[9ms\033[0mplain");

    std::string glyph;
    tuinator::Style style{};

    TUINATOR_CHECK(buffer.cell_at(0, 0, glyph, style));
    TUINATOR_CHECK(style.italic);

    TUINATOR_CHECK(buffer.cell_at(0, 1, glyph, style));
    TUINATOR_CHECK(style.underline);

    TUINATOR_CHECK(buffer.cell_at(0, 2, glyph, style));
    TUINATOR_CHECK(style.strikethrough);

    TUINATOR_CHECK(buffer.cell_at(0, 3, glyph, style));
    TUINATOR_CHECK(!style.italic);
    TUINATOR_CHECK(!style.underline);
    TUINATOR_CHECK(!style.strikethrough);
}
