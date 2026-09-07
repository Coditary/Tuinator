#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/render/canvas.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/terminal/ansi_terminal_buffer.hpp>

#include <string>

#include "bench_harness.hpp"

namespace {

std::string colored_scrollback_chunk(int index) {
    std::string out;
    out += "\033[31m[line ";
    out += std::to_string(index);
    out += "]\033[0m output: value=";
    out += std::to_string(index * 17 % 1000);
    out += " status=ok\r\n";
    return out;
}

std::string build_ansi_feed(std::size_t lines) {
    std::string feed;
    for (std::size_t i = 0; i < lines; ++i) {
        feed += colored_scrollback_chunk(static_cast<int>(i));
    }
    return feed;
}

void paint_buffer(tuinator::AnsiTerminalBuffer& buffer, tuinator::MemoryTerminalBackend& backend) {
    backend.begin_frame();
    tuinator::Canvas canvas(backend);
    const tuinator::Theme theme = tuinator::dark_theme();
    tuinator::PaintContext ctx{canvas, theme};
    buffer.paint(ctx, {0, 0});
    backend.end_frame();
}

} // namespace

TUINATOR_PERF_TEST(ansi_terminal_feed_and_paint) {
    const std::string feed = build_ansi_feed(2'000);
    tuinator::AnsiTerminalBuffer buffer;
    buffer.resize({120, 40});
    tuinator::MemoryTerminalBackend backend({120, 40});
    backend.init();

    return tuinator::perf::bench("ansi_terminal_feed_and_paint", 400.0, 1, 10, [&]() {
        buffer.reset();
        buffer.feed(feed);
        paint_buffer(buffer, backend);
    });
}

TUINATOR_PERF_TEST(ansi_terminal_incremental_feed) {
    tuinator::AnsiTerminalBuffer buffer;
    buffer.resize({120, 40});
    tuinator::MemoryTerminalBackend backend({120, 40});
    backend.init();

    return tuinator::perf::bench("ansi_terminal_incremental_feed", 300.0, 1, 15, [&]() {
        buffer.reset();
        for (int i = 0; i < 200; ++i) {
            buffer.feed(colored_scrollback_chunk(i));
        }
        paint_buffer(buffer, backend);
    });
}
