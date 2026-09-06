#include "test_harness.hpp"

#include <tuinator/backend/inline_backend.hpp>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <variant>

#if TUINATOR_PLATFORM_POSIX

namespace {

struct CaptureOutput {
    char* buffer = nullptr;
    size_t length = 0;
    FILE* file = nullptr;

    CaptureOutput() {
        file = open_memstream(&buffer, &length);
    }

    ~CaptureOutput() {
        if (file != nullptr) {
            std::fclose(file);
        }
        std::free(buffer);
    }

    std::string text() const {
        return std::string(buffer != nullptr ? buffer : "", length);
    }
};

struct StdinPipe {
    int fds[2] = {-1, -1};
    int saved = -1;

    bool open() {
        if (pipe(fds) != 0) {
            return false;
        }
        saved = dup(STDIN_FILENO);
        if (saved < 0 || dup2(fds[0], STDIN_FILENO) < 0) {
            return false;
        }
        return true;
    }

    void write_byte(unsigned char byte) {
        write(fds[1], &byte, 1);
    }

    ~StdinPipe() {
        if (saved >= 0) {
            dup2(saved, STDIN_FILENO);
            close(saved);
        }
        if (fds[0] >= 0) {
            close(fds[0]);
        }
        if (fds[1] >= 0) {
            close(fds[1]);
        }
    }
};

} // namespace

#endif

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

TUINATOR_TEST(inline_backend_relative_height_from_options) {
    tuinator::InlineTerminalBackend backend({.height = 7});
    backend.init();
    TUINATOR_CHECK_EQ(backend.terminal_size().height, 7);
    backend.shutdown();
}

TUINATOR_TEST(inline_backend_relative_redraw_erases_previous_band) {
    CaptureOutput capture;
    TUINATOR_CHECK(capture.file != nullptr);

    tuinator::InlineTerminalBackend backend({.height = 3, .clear_on_shutdown = false, .output = capture.file});
    backend.init();

    backend.begin_frame();
    backend.draw_text(0, 0, "first", tuinator::Style{.foreground = tuinator::Color::Green});
    backend.end_frame();

    backend.begin_frame();
    backend.draw_text(0, 0, "second", tuinator::Style{.foreground = tuinator::Color::Green});
    backend.end_frame();

    const std::string out = capture.text();
    TUINATOR_CHECK(out.find("\033[1A") != std::string::npos);
    TUINATOR_CHECK(out.find("second") != std::string::npos);
    backend.shutdown();
}

TUINATOR_TEST(inline_backend_first_frame_does_not_cursor_up) {
    CaptureOutput capture;
    tuinator::InlineTerminalBackend backend({.height = 2, .clear_on_shutdown = false, .output = capture.file});
    backend.init();

    backend.begin_frame();
    backend.draw_text(0, 0, "once", tuinator::Style{.foreground = tuinator::Color::Yellow});
    backend.end_frame();

    const std::string out = capture.text();
    TUINATOR_CHECK(out.find("\033[1A") == std::string::npos);
    TUINATOR_CHECK(out.find("once") != std::string::npos);
    backend.shutdown();
}

TUINATOR_TEST(inline_backend_unchanged_frame_skips_extra_output) {
    CaptureOutput capture;
    tuinator::InlineTerminalBackend backend({.height = 2, .clear_on_shutdown = false, .output = capture.file});
    backend.init();

    const tuinator::Style style{.foreground = tuinator::Color::Blue};
    backend.begin_frame();
    backend.draw_text(0, 0, "same", style);
    backend.end_frame();
    const std::size_t after_first = capture.length;

    backend.begin_frame();
    backend.draw_text(0, 0, "same", style);
    backend.end_frame();
    TUINATOR_CHECK_EQ(capture.length, after_first);

    backend.shutdown();
}

TUINATOR_TEST(inline_backend_true_color_rgb_sequences) {
    CaptureOutput capture;
    tuinator::InlineTerminalBackend backend({.height = 1, .clear_on_shutdown = false, .output = capture.file});
    backend.init();

    backend.begin_frame();
    backend.draw_text(
        0,
        0,
        "rgb",
        tuinator::Style{.foreground_rgb = tuinator::Rgb{0x89, 0xB4, 0xFA}, .bold = true});
    backend.end_frame();

    const std::string out = capture.text();
    TUINATOR_CHECK(out.find("\033[38;2;137;180;250m") != std::string::npos);
    backend.shutdown();
}

TUINATOR_TEST(inline_backend_clear_on_shutdown_erases_band) {
    CaptureOutput capture;
    tuinator::InlineTerminalBackend backend({.height = 2, .clear_on_shutdown = true, .output = capture.file});
    backend.init();

    backend.begin_frame();
    backend.draw_text(0, 0, "gone", tuinator::Style{.foreground = tuinator::Color::Red});
    backend.end_frame();

    backend.shutdown();

    const std::string out = capture.text();
    TUINATOR_CHECK(out.find("\033[1A") != std::string::npos);
    TUINATOR_CHECK(out.find("\033[2K") != std::string::npos);
}

TUINATOR_TEST(inline_backend_pin_to_bottom_uses_absolute_rows) {
    CaptureOutput capture;
    tuinator::InlineTerminalBackend backend({.height = 2, .pin_to_bottom = true, .output = capture.file});
    backend.init();

    backend.begin_frame();
    backend.draw_text(0, 0, "bottom", tuinator::Style{.foreground = tuinator::Color::Cyan});
    backend.end_frame();

    const std::string out = capture.text();
    TUINATOR_CHECK(out.find(";1H\033[2K") != std::string::npos);
    TUINATOR_CHECK(out.find("bottom") != std::string::npos);
    backend.shutdown();
}

TUINATOR_TEST(inline_backend_poll_event_times_out) {
    tuinator::InlineTerminalBackend backend({.height = 2});
    backend.init();
    backend.set_poll_timeout_ms(1);

    const std::optional<tuinator::Event> event = backend.poll_event();
    TUINATOR_CHECK(!event.has_value());

    backend.shutdown();
}

TUINATOR_TEST(inline_backend_keyboard_input_disabled_drains_without_events) {
    tuinator::InlineTerminalBackend backend({.height = 2, .keyboard_input = false});
    backend.init();

    const std::optional<tuinator::Event> event = backend.poll_event_nonblocking();
    TUINATOR_CHECK(!event.has_value());

    backend.shutdown();
}

TUINATOR_TEST(inline_backend_keyboard_input_reads_enter_from_pipe) {
    StdinPipe stdin_pipe;
    TUINATOR_CHECK(stdin_pipe.open());

    CaptureOutput capture;
    tuinator::InlineTerminalBackend backend({.height = 2, .keyboard_input = true, .output = capture.file});
    backend.init();
    stdin_pipe.write_byte('\n');

    const std::optional<tuinator::Event> event = backend.poll_event_nonblocking();
    TUINATOR_CHECK(event.has_value());
    TUINATOR_CHECK(std::holds_alternative<tuinator::KeyPress>(*event));
    TUINATOR_CHECK(std::get<tuinator::KeyPress>(*event).key == tuinator::Key::Enter);

    backend.shutdown();
}

TUINATOR_TEST(inline_backend_keyboard_input_reads_escape_and_printable) {
    StdinPipe stdin_pipe;
    TUINATOR_CHECK(stdin_pipe.open());

    CaptureOutput capture;
    tuinator::InlineTerminalBackend backend({.height = 2, .keyboard_input = true, .output = capture.file});
    backend.init();
    stdin_pipe.write_byte(27);

    const std::optional<tuinator::Event> escape = backend.poll_event_nonblocking();
    TUINATOR_CHECK(escape.has_value());
    TUINATOR_CHECK(std::get<tuinator::KeyPress>(*escape).key == tuinator::Key::Escape);

    stdin_pipe.write_byte('q');
    const std::optional<tuinator::Event> letter = backend.poll_event_nonblocking();
    TUINATOR_CHECK(letter.has_value());
    TUINATOR_CHECK(std::get<tuinator::KeyPress>(*letter).character == 'q');

    backend.shutdown();
}

TUINATOR_TEST(inline_backend_keyboard_input_ignores_unknown_bytes) {
    StdinPipe stdin_pipe;
    TUINATOR_CHECK(stdin_pipe.open());

    tuinator::InlineTerminalBackend backend({.height = 2, .keyboard_input = true});
    backend.init();
    stdin_pipe.write_byte(200);

    const std::optional<tuinator::Event> event = backend.poll_event_nonblocking();
    TUINATOR_CHECK(!event.has_value());

    backend.shutdown();
}

TUINATOR_TEST(inline_backend_stdin_pipe_drains_without_keyboard) {
    StdinPipe stdin_pipe;
    TUINATOR_CHECK(stdin_pipe.open());

    tuinator::InlineTerminalBackend backend({.height = 2, .keyboard_input = false});
    backend.init();
    stdin_pipe.write_byte('\n');
    stdin_pipe.write_byte('x');

    const std::optional<tuinator::Event> event = backend.poll_event_nonblocking();
    TUINATOR_CHECK(!event.has_value());

    backend.shutdown();
}

TUINATOR_TEST(inline_backend_partial_begin_frame_clears_dirty_region) {
    CaptureOutput capture;
    tuinator::InlineTerminalBackend backend({.height = 3, .output = capture.file});
    backend.init();

    backend.begin_frame({.full_redraw = false, .dirty_region = {{0, 1}, {4, 1}}});
    backend.draw_text(0, 1, "row", tuinator::Style{.foreground = tuinator::Color::Magenta});
    backend.end_frame();

    TUINATOR_CHECK(capture.text().find("row") != std::string::npos);
    backend.shutdown();
}

TUINATOR_TEST(inline_backend_full_redraw_without_clear_keeps_buffer) {
    CaptureOutput capture;
    tuinator::InlineTerminalBackend backend({.height = 2, .output = capture.file});
    backend.init();

    backend.begin_frame();
    backend.draw_text(0, 0, "keep", tuinator::Style{.foreground = tuinator::Color::Green});
    backend.end_frame();

    backend.begin_frame({.full_redraw = true, .clear_buffer = false});
    backend.end_frame();

    backend.begin_frame();
    backend.draw_text(0, 1, "add", tuinator::Style{.foreground = tuinator::Color::Blue});
    backend.end_frame();

    const std::string out = capture.text();
    TUINATOR_CHECK(out.find("keep") != std::string::npos);
    TUINATOR_CHECK(out.find("add") != std::string::npos);
    backend.shutdown();
}

TUINATOR_TEST(inline_backend_ansi_and_background_styles) {
    CaptureOutput capture;
    tuinator::InlineTerminalBackend backend({.height = 1, .true_color = false, .output = capture.file});
    backend.init();

    backend.begin_frame();
    backend.draw_text(
        0,
        0,
        "styled",
        tuinator::Style{
            .foreground = tuinator::Color::Red,
            .background = tuinator::Color::Blue,
            .dim = true,
            .reverse = true});
    backend.end_frame();

    const std::string out = capture.text();
    TUINATOR_CHECK(out.find("\033[31m") != std::string::npos);
    TUINATOR_CHECK(out.find("\033[44m") != std::string::npos);
    TUINATOR_CHECK(out.find("\033[2m") != std::string::npos);
    TUINATOR_CHECK(out.find("\033[7m") != std::string::npos);
    backend.shutdown();
}

TUINATOR_TEST(inline_backend_background_rgb_style) {
    CaptureOutput capture;
    tuinator::InlineTerminalBackend backend({.height = 1, .output = capture.file});
    backend.init();

    backend.begin_frame();
    backend.draw_text(
        0,
        0,
        "bg",
        tuinator::Style{.background_rgb = tuinator::Rgb{0x1E, 0x1E, 0x2E}});
    backend.end_frame();

    TUINATOR_CHECK(capture.text().find("\033[48;2;30;30;46m") != std::string::npos);
    backend.shutdown();
}

TUINATOR_TEST(inline_backend_draw_text_bounds_and_wide_grapheme) {
    CaptureOutput capture;
    tuinator::InlineTerminalBackend backend({.height = 2, .output = capture.file});
    backend.init();

    backend.begin_frame();
    backend.draw_text(0, -1, "skip", tuinator::Style{});
    backend.draw_text(0, 9, "skip", tuinator::Style{});
    backend.draw_text(0, 0, "", tuinator::Style{});
    backend.draw_text(0, 0, "⣾", tuinator::Style{.foreground = tuinator::Color::Cyan});
    backend.draw_text(0, 1, "ok", tuinator::Style{.foreground = tuinator::Color::Green});
    backend.end_frame();

    const std::string out = capture.text();
    TUINATOR_CHECK(out.find("⣾") != std::string::npos);
    TUINATOR_CHECK(out.find("ok") != std::string::npos);
    backend.shutdown();
}

TUINATOR_TEST(inline_backend_max_height_clamps_band) {
    tuinator::InlineTerminalBackend backend({.height = 12, .max_height = 5});
    backend.init();
    TUINATOR_CHECK_EQ(backend.terminal_size().height, 5);
    backend.shutdown();
}

TUINATOR_TEST(inline_backend_absolute_partial_row_update) {
    CaptureOutput capture;
    tuinator::InlineTerminalBackend backend({.height = 3, .anchor_row = 4, .output = capture.file});
    backend.init();

    backend.begin_frame();
    backend.draw_text(0, 0, "row0", tuinator::Style{.foreground = tuinator::Color::Green});
    backend.draw_text(0, 1, "row1", tuinator::Style{.foreground = tuinator::Color::Green});
    backend.draw_text(0, 2, "row2", tuinator::Style{.foreground = tuinator::Color::Green});
    backend.end_frame();
    const std::size_t after_first = capture.length;

    backend.begin_frame();
    backend.draw_text(0, 1, "chg!", tuinator::Style{.foreground = tuinator::Color::Yellow});
    backend.end_frame();

    TUINATOR_CHECK(capture.length > after_first);
    TUINATOR_CHECK(capture.text().find("chg!") != std::string::npos);
    backend.shutdown();
}

TUINATOR_TEST(inline_backend_second_frame_uses_kitty_sync) {
    CaptureOutput capture;
    tuinator::InlineTerminalBackend backend({.height = 2, .output = capture.file});
    backend.init();

    backend.begin_frame();
    backend.draw_text(0, 0, "a", tuinator::Style{.foreground = tuinator::Color::Green});
    backend.end_frame();

    backend.begin_frame();
    backend.draw_text(0, 0, "b", tuinator::Style{.foreground = tuinator::Color::Green});
    backend.end_frame();

    const std::string out = capture.text();
    TUINATOR_CHECK(out.find("\033[?2026h") != std::string::npos);
    TUINATOR_CHECK(out.find("\033[?2026l") != std::string::npos);
    backend.shutdown();
}

TUINATOR_TEST(inline_backend_init_and_shutdown_are_idempotent) {
    CaptureOutput capture;
    tuinator::InlineTerminalBackend backend({.height = 2, .output = capture.file});
    backend.init();
    backend.init();
    backend.shutdown();
    backend.shutdown();
}

TUINATOR_TEST(inline_backend_keyboard_input_reads_ctrl_and_backspace) {
    StdinPipe stdin_pipe;
    TUINATOR_CHECK(stdin_pipe.open());

    tuinator::InlineTerminalBackend backend({.height = 2, .keyboard_input = true});
    backend.init();
    stdin_pipe.write_byte(1);

    const std::optional<tuinator::Event> ctrl = backend.poll_event_nonblocking();
    TUINATOR_CHECK(ctrl.has_value());
    const auto& ctrl_press = std::get<tuinator::KeyPress>(*ctrl);
    TUINATOR_CHECK(ctrl_press.ctrl);
    TUINATOR_CHECK(ctrl_press.character == 'a');

    stdin_pipe.write_byte(127);
    const std::optional<tuinator::Event> backspace = backend.poll_event_nonblocking();
    TUINATOR_CHECK(backspace.has_value());
    TUINATOR_CHECK(std::get<tuinator::KeyPress>(*backspace).key == tuinator::Key::Backspace);

    backend.shutdown();
}

TUINATOR_TEST(inline_backend_poll_event_reads_keyboard_from_pipe) {
    StdinPipe stdin_pipe;
    TUINATOR_CHECK(stdin_pipe.open());

    tuinator::InlineTerminalBackend backend({.height = 2, .keyboard_input = true});
    backend.init();
    backend.set_poll_timeout_ms(1);
    stdin_pipe.write_byte('z');

    const std::optional<tuinator::Event> event = backend.poll_event();
    TUINATOR_CHECK(event.has_value());
    TUINATOR_CHECK(std::get<tuinator::KeyPress>(*event).character == 'z');

    backend.shutdown();
}

TUINATOR_TEST(inline_backend_default_color_style) {
    CaptureOutput capture;
    tuinator::InlineTerminalBackend backend({.height = 1, .true_color = false, .output = capture.file});
    backend.init();

    backend.begin_frame();
    backend.draw_text(0, 0, "def", tuinator::Style{.foreground = tuinator::Color::Default});
    backend.end_frame();

    TUINATOR_CHECK(capture.text().find("def") != std::string::npos);
    backend.shutdown();
}

TUINATOR_TEST(inline_backend_create_factory_and_destructor) {
    CaptureOutput capture;
    auto backend = tuinator::InlineTerminalBackend::create({.height = 2, .output = capture.file});
    TUINATOR_CHECK(backend != nullptr);
    backend->init();
    backend->begin_frame();
    backend->draw_text(0, 0, "factory", tuinator::Style{.foreground = tuinator::Color::White});
    backend->end_frame();
}

#endif
