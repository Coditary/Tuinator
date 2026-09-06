#include "test_harness.hpp"

#include <tuinator/backend/inline_backend.hpp>

#include <cstdio>
#include <cstdlib>
#include <string>

TUINATOR_TEST(inline_backend_draw_and_size) {
    tuinator::InlineTerminalBackend backend({.height = 4, .anchor_row = 5});
    backend.init();

    TUINATOR_CHECK_EQ(backend.terminal_size().height, 4);
    TUINATOR_CHECK_EQ(backend.anchor_row(), 5);

    backend.begin_frame();
    backend.draw_text(0, 0, "inline", tuinator::Style{.foreground = tuinator::Color::Green});
    backend.end_frame();

    backend.shutdown();
}

#if TUINATOR_PLATFORM_POSIX

TUINATOR_TEST(inline_backend_relative_redraw_erases_previous_band) {
    char* buffer = nullptr;
    size_t length = 0;
    FILE* capture = open_memstream(&buffer, &length);
    TUINATOR_CHECK(capture != nullptr);

    tuinator::InlineTerminalBackend backend({.height = 3, .clear_on_shutdown = false, .output = capture});
    backend.init();

    backend.begin_frame();
    backend.draw_text(0, 0, "first", tuinator::Style{.foreground = tuinator::Color::Green});
    backend.end_frame();

    backend.begin_frame();
    backend.draw_text(0, 0, "second", tuinator::Style{.foreground = tuinator::Color::Green});
    backend.end_frame();

    std::fflush(capture);
    const std::string out(buffer != nullptr ? buffer : "", length);
    TUINATOR_CHECK(out.find("\033[1A") != std::string::npos);
    TUINATOR_CHECK(out.find("second") != std::string::npos);

    backend.shutdown();
    std::fclose(capture);
    std::free(buffer);
}

#endif
