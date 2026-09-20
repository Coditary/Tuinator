#include <tuinator/debug/startup_profiler.hpp>
#include <tuinator/render/color.hpp>
#include <tuinator/render/graphics_encode.hpp>
#include <tuinator/render/graphics_protocol.hpp>
#include <tuinator/render/terminal_image.hpp>
#include <tuinator/render/text.hpp>

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <clocale>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <variant>

#include "backend/ansi_backend.hpp"

namespace tuinator::detail {

namespace {

int ansi_color_code(Color color, bool foreground) {
    const int base = foreground ? 30 : 40;
    switch (color) {
    case Color::Black: return base + 0;
    case Color::Red: return base + 1;
    case Color::Green: return base + 2;
    case Color::Yellow: return base + 3;
    case Color::Blue: return base + 4;
    case Color::Magenta: return base + 5;
    case Color::Cyan: return base + 6;
    case Color::White: return base + 7;
    case Color::Default: return -1;
    }
    return -1;
}

Rgb palette_to_rgb(Color color) {
    switch (color) {
    case Color::Black: return {0, 0, 0};
    case Color::Red: return {220, 50, 47};
    case Color::Green: return {80, 200, 120};
    case Color::Yellow: return {220, 200, 50};
    case Color::Blue: return {80, 120, 220};
    case Color::Magenta: return {200, 80, 200};
    case Color::Cyan: return {80, 200, 220};
    case Color::White: return {230, 230, 230};
    case Color::Default: return {};
    }
    return {};
}

Style style_for_ansi_output(Style style, bool true_color) {
    if (true_color) {
        if (!style.foreground_rgb.has_value() && style.foreground != Color::Default) {
            style.foreground_rgb = palette_to_rgb(style.foreground);
        }
        if (!style.background_rgb.has_value() && style.background != Color::Default) {
            style.background_rgb = palette_to_rgb(style.background);
        }
    }
    return style;
}

bool ansi_style_equal(const Style& a, const Style& b) {
    return a.foreground == b.foreground && a.background == b.background && a.foreground_rgb == b.foreground_rgb &&
           a.background_rgb == b.background_rgb && a.bold == b.bold && a.dim == b.dim && a.reverse == b.reverse &&
           a.italic == b.italic && a.underline == b.underline && a.strikethrough == b.strikethrough;
}

FILE* open_tty_output() { return std::fopen("/dev/tty", "we"); }

void reset_tty_attributes() {
    if (FILE* tty = open_tty_output()) {
        std::fputs("\033[0m", tty);
        std::fflush(tty);
        std::fclose(tty);
    }
}

void send_tty_sequence_to(FILE* output, const char* sequence) {
    if (output == nullptr || sequence == nullptr || sequence[0] == '\0') {
        return;
    }

    fputs(sequence, output);
    fflush(output);
}

bool detect_sync_updates() {
    const char* env = std::getenv("TUINATOR_SYNC_UPDATE");
    if (env != nullptr && env[0] != '\0') {
        return std::strcmp(env, "0") != 0;
    }

    const char* term = std::getenv("TERM");
    if (term == nullptr) {
        return false;
    }

    return std::strstr(term, "kitty") != nullptr || std::strstr(term, "foot") != nullptr ||
           std::strstr(term, "alacritty") != nullptr || std::strstr(term, "wezterm") != nullptr ||
           std::strstr(term, "ghostty") != nullptr || std::strstr(term, "contour") != nullptr;
}

bool terminal_name_suggests_xterm_mouse() {
    const char* term = std::getenv("TERM");
    if (term == nullptr) {
        return false;
    }

    return std::strstr(term, "xterm") != nullptr || std::strstr(term, "rxvt") != nullptr ||
           std::strstr(term, "screen") != nullptr || std::strstr(term, "tmux") != nullptr ||
           std::strstr(term, "alacritty") != nullptr || std::strstr(term, "kitty") != nullptr ||
           std::strstr(term, "foot") != nullptr || std::strstr(term, "wezterm") != nullptr ||
           std::strstr(term, "ghostty") != nullptr || std::strstr(term, "contour") != nullptr ||
           std::strstr(term, "konsole") != nullptr || std::strstr(term, "vscode") != nullptr;
}

void cleanup_kitty_graphics_on_tty(FILE* tty) {
    if (tty == nullptr) {
        return;
    }
    fputs("\033_Ga=d,d=A\033\\", tty);
}

void restore_terminal_state() {
    if (FILE* tty = open_tty_output()) {
        cleanup_kitty_graphics_on_tty(tty);
        // Pop kitty keyboard flags and reset modifyOtherKeys, otherwise the shell
        // keeps receiving CSI-u key events after the app exits.
        fputs("\033[<u\033[>4;0m", tty);
        fputs("\033[?1000l\033[?1002l\033[?1003l\033[?1006l\033[?2004l", tty);
        fputs("\033[0m\033[?25h", tty);
        fflush(tty);
        fclose(tty);
    }
}

void atexit_restore_terminal() { restore_terminal_state(); }

#if TUINATOR_PLATFORM_POSIX

termios g_crash_termios{};
std::atomic<bool> g_crash_termios_valid{false};

// Best-effort restore on fatal signals so Ctrl+C/SIGTERM never leaves the terminal
// in raw mode, the alternate screen, or kitty keyboard mode.
void on_fatal_signal(int sig) {
    restore_terminal_state();
    if (g_crash_termios_valid.load()) {
        tcsetattr(STDIN_FILENO, TCSANOW, &g_crash_termios);
    }
    std::signal(sig, SIG_DFL);
    raise(sig);
}

void install_fatal_signal_handlers() {
    static bool installed = false;
    if (installed) {
        return;
    }
    installed = true;
    std::signal(SIGINT, on_fatal_signal);
    std::signal(SIGTERM, on_fatal_signal);
    std::signal(SIGHUP, on_fatal_signal);
}

#endif

} // namespace

AnsiBackend::AnsiBackend() = default;

AnsiBackend::~AnsiBackend() { shutdown(); }

FILE* AnsiBackend::output_stream() const { return tty_out_ != nullptr ? tty_out_ : stdout; }

void AnsiBackend::write_tty_sequence(const char* sequence) { send_tty_sequence_to(output_stream(), sequence); }

void AnsiBackend::close_terminal_streams() {
    if (tty_in_ != nullptr) {
        std::fclose(tty_in_);
        tty_in_ = nullptr;
    }

    if (tty_out_ != nullptr) {
        std::fclose(tty_out_);
        tty_out_ = nullptr;
    }
}

Size AnsiBackend::query_terminal_size() const {
    winsize ws{};
    const int fd = open("/dev/tty", O_RDWR | O_CLOEXEC);
    if (fd >= 0) {
        if (ioctl(fd, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 && ws.ws_row > 0) {
            close(fd);
            return {static_cast<int>(ws.ws_col), static_cast<int>(ws.ws_row)};
        }
        close(fd);
    }

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 && ws.ws_row > 0) {
        return {static_cast<int>(ws.ws_col), static_cast<int>(ws.ws_row)};
    }

    return {terminal_width_, terminal_height_};
}

void AnsiBackend::acquire_stdin() {
    if (stdin_captured_) {
        return;
    }

    if (isatty(STDIN_FILENO)) {
        if (tcgetattr(STDIN_FILENO, &stdin_original_) != 0) {
            return;
        }

#if TUINATOR_PLATFORM_POSIX
        g_crash_termios = stdin_original_;
        g_crash_termios_valid.store(true);
#endif

        termios raw = stdin_original_;
        raw.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
        raw.c_iflag &= static_cast<tcflag_t>(~(IXON | ICRNL));
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) {
            return;
        }
    } else {
        stdin_original_flags_ = fcntl(STDIN_FILENO, F_GETFL, 0);
        if (stdin_original_flags_ >= 0) {
            fcntl(STDIN_FILENO, F_SETFL, stdin_original_flags_ | O_NONBLOCK);
            stdin_nonblocking_set_ = true;
        }
    }

    stdin_captured_ = true;
    drain_stdin();
}

void AnsiBackend::release_stdin() {
    if (!stdin_captured_) {
        return;
    }

    if (isatty(STDIN_FILENO)) {
        tcsetattr(STDIN_FILENO, TCSANOW, &stdin_original_);
    }
#if TUINATOR_PLATFORM_POSIX
    g_crash_termios_valid.store(false);
#endif
    if (stdin_nonblocking_set_) {
        fcntl(STDIN_FILENO, F_SETFL, stdin_original_flags_);
        stdin_nonblocking_set_ = false;
    }
    stdin_captured_ = false;
}

void AnsiBackend::drain_stdin() {
    char buffer[256];
    for (;;) {
        pollfd fds{};
        fds.fd = STDIN_FILENO;
        fds.events = POLLIN;
        if (poll(&fds, 1, 0) <= 0 || !(fds.revents & POLLIN)) {
            return;
        }
        if (read(STDIN_FILENO, buffer, sizeof(buffer)) <= 0) {
            return;
        }
    }
}

void AnsiBackend::init() {
    if (initialized_) {
        return;
    }

    // Match the curses backend: adopt the environment locale so UTF-8 detection
    // (locale_supports_utf8 / detect_glyph_set) enables Unicode box-drawing glyphs.
    // Without this the locale stays "C" and the UI falls back to ASCII borders.
    std::setlocale(LC_ALL, "");

    startup_profile_mark("ansi.before_tty");
    tty_out_ = std::fopen("/dev/tty", "we");
    tty_in_ = std::fopen("/dev/tty", "re");
    if (tty_out_ != nullptr) {
        std::setvbuf(tty_out_, nullptr, _IONBF, 0);
    }

    const Size term = query_terminal_size();
    terminal_width_ = term.width;
    terminal_height_ = term.height;
    ansi_cells_width_ = terminal_width_;
    ansi_cells_height_ = terminal_height_;
    ansi_cells_.assign(static_cast<std::size_t>(std::max(0, terminal_width_)) *
                           static_cast<std::size_t>(std::max(0, terminal_height_)),
                       AnsiCell{});

    true_color_enabled_ = detect_true_color();
    sync_updates_supported_ = detect_sync_updates();

    install_terminal_signal_handlers();
#if TUINATOR_PLATFORM_POSIX
    install_fatal_signal_handlers();
#endif
    acquire_stdin();

    write_tty_sequence("\033[?25l");

    if (alternate_screen()) {
        write_tty_sequence("\033[?1049h");
        alternate_screen_active_ = true;
    }

    write_tty_sequence("\033[2J\033[H");
    // Report Ctrl/Alt/Shift modifiers on function keys (Ctrl+Delete -> ESC [ 3 ; 5 ~).
    write_tty_sequence("\033[>4;2m");
    // Kitty keyboard: disambiguate (1) + event types (2) + alternate keys (4). Deliberately
    // WITHOUT "all keys as CSI" (8) / "associated text" (16): with flag 8 the terminal stops
    // composing dead keys/IME input itself, and some terminals (e.g. ghostty before #11149)
    // then drop composed characters entirely. With flags 7, text (incl. dead-key compositions
    // like à or a standalone backtick, and AltGr symbols) arrives as plain UTF-8, while
    // Ctrl/Alt/Esc ambiguity and modified function keys are still reported as CSI u.
    write_tty_sequence("\033[>7u");
    write_tty_sequence("\033[?2004h");
    enable_mouse();

    static bool atexit_registered = false;
    if (!atexit_registered) {
        std::atexit(atexit_restore_terminal);
        atexit_registered = true;
    }

    initialized_ = true;
    startup_profile_mark("ansi.init_done");
}

void AnsiBackend::enable_mouse() {
    mouse_cursor_user_enabled_ = [] {
        const char* setting = std::getenv("TUINATOR_MOUSE_CURSOR");
        return setting != nullptr && setting[0] != '\0' && std::strcmp(setting, "0") != 0;
    }();
    mouse_cursor_visible_ = mouse_cursor_user_enabled_ && !mouse_cursor_suppressed_;
    mouse_enabled_ = true;

    if (terminal_name_suggests_xterm_mouse()) {
        write_tty_sequence("\033[?1000l\033[?1002l\033[?1003l\033[?1006l");

        char enable[48];
        const int mode = mouse_tracking_mode();
        std::snprintf(enable, sizeof(enable), "\033[?%dh\033[?1006h", mode);
        write_tty_sequence(enable);
        xterm_mouse_enabled_ = true;
    }
}

void AnsiBackend::disable_mouse() {
    if (xterm_mouse_enabled_) {
        write_tty_sequence("\033[?1000l\033[?1002l\033[?1003l\033[?1006l");
        xterm_mouse_enabled_ = false;
    }
}

int AnsiBackend::mouse_tracking_mode() const {
    const char* mode = std::getenv("TUINATOR_MOUSE_TRACK");
    if (mode == nullptr || mode[0] == '\0') {
        return 1002;
    }

    if (std::strcmp(mode, "1002") == 0) {
        return 1002;
    }
    if (std::strcmp(mode, "1000") == 0) {
        return 1000;
    }
    if (std::strcmp(mode, "1003") == 0) {
        return 1003;
    }

    return 1002;
}

void AnsiBackend::set_poll_timeout_ms(int timeout_ms) { poll_timeout_ms_ = timeout_ms; }

void AnsiBackend::position_hardware_mouse_cursor(FILE* output) {
    if (!initialized_ || output == nullptr) {
        return;
    }

    if (!mouse_cursor_visible_ || !last_mouse_position_.has_value()) {
        send_tty_sequence_to(output, "\033[?25l");
        return;
    }

    const Point position = *last_mouse_position_;
    const Size term = terminal_size();
    if (position.x < 0 || position.y < 0 || position.x >= term.width || position.y >= term.height) {
        send_tty_sequence_to(output, "\033[?25l");
        return;
    }

    char sequence[48];
    std::snprintf(sequence, sizeof(sequence), "\033[%d;%dH\033[?25h", position.y + 1, position.x + 1);
    send_tty_sequence_to(output, sequence);
}

void AnsiBackend::refresh_mouse_cursor() { position_hardware_mouse_cursor(output_stream()); }

void AnsiBackend::set_mouse_cursor_suppressed(bool suppressed) {
    mouse_cursor_suppressed_ = suppressed;
    mouse_cursor_visible_ = mouse_cursor_user_enabled_ && !mouse_cursor_suppressed_;
}

void AnsiBackend::set_text_cursor(std::optional<Point> position) { text_cursor_position_ = position; }

void AnsiBackend::cleanup_kitty_graphics() {
    if (FILE* output = output_stream()) {
        cleanup_kitty_graphics_on_tty(output);
        std::fflush(output);
    }
    kitty_slots_.clear();
    next_kitty_slot_id_ = 1;
    next_kitty_image_id_ = 1;
}

std::optional<std::uint32_t> AnsiBackend::find_kitty_slot(std::uint32_t content_hash, int x, int y, int cols,
                                                          int rows) const {
    for (const auto& entry : kitty_slots_) {
        const KittySlot& slot = entry.second;
        if (slot.ready && slot.x == x && slot.y == y && slot.cols == cols && slot.rows == rows) {
            return entry.first;
        }
    }

    // Reuse when the same pixels moved (one slot with this hash). Multiple
    // on-screen copies of the same file get separate slots keyed by position.
    std::optional<std::uint32_t> sole_hash_match;
    int hash_matches = 0;
    for (const auto& entry : kitty_slots_) {
        if (entry.second.content_hash != content_hash) {
            continue;
        }
        sole_hash_match = entry.first;
        ++hash_matches;
    }
    if (hash_matches == 1 && sole_hash_match.has_value()) {
        const KittySlot& slot = kitty_slots_.at(*sole_hash_match);
        // Same frame, second copy of the same pixels at a different cell → new slot.
        // Next frame, sole slot not touched yet → moved image reuses the slot.
        if (!slot.touched_this_frame) {
            return sole_hash_match;
        }
    }

    return std::nullopt;
}

void AnsiBackend::shutdown() {
    if (!initialized_) {
        restore_terminal_state();
        return;
    }

    disable_mouse();
    cleanup_kitty_graphics();
    write_tty_sequence("\033[<u");
    write_tty_sequence("\033[>4;0m");
    if (FILE* output = output_stream()) {
        if (alternate_screen_active_) {
            send_tty_sequence_to(output, "\033[?1049l");
            alternate_screen_active_ = false;
        } else if (clear_on_shutdown()) {
            send_tty_sequence_to(output, "\033[2J");
        }
    }
    reset_tty_attributes();
    release_stdin();
    close_terminal_streams();
    restore_terminal_state();
    initialized_ = false;
    mouse_enabled_ = false;
    pending_ansi_draws_.clear();
    left_button_down_ = false;
    input_.reset();
}

Size AnsiBackend::terminal_size() const { return {terminal_width_, terminal_height_}; }

void AnsiBackend::track_input_event(const Event& event) {
    if (const MouseEvent* mouse = std::get_if<MouseEvent>(&event)) {
        if (mouse->action == MouseAction::Press && mouse->left_pressed) {
            left_button_down_ = true;
        } else if (mouse->action == MouseAction::Release) {
            left_button_down_ = false;
        } else if (mouse->action == MouseAction::Move) {
            left_button_down_ = mouse->left_pressed;
        }
        last_mouse_position_ = mouse->position;
    }
}

std::optional<Event> AnsiBackend::read_event(bool block) {
    // A lone ESC byte is ambiguous with the start of an escape sequence; give the
    // terminal this long to deliver continuation bytes before emitting Key::Escape.
    constexpr int kEscapeTimeoutMs = 50;

    for (;;) {
        if (!pending_events_.empty()) {
            Event event = std::move(pending_events_.front());
            pending_events_.pop_front();
            track_input_event(event);
            return event;
        }

        if (TerminalInput::take_resize_pending()) {
            invalidate_graphics();
            const Size size = query_terminal_size();
            terminal_width_ = size.width;
            terminal_height_ = size.height;
            if (size.width != ansi_cells_width_ || size.height != ansi_cells_height_) {
                ansi_cells_width_ = size.width;
                ansi_cells_height_ = size.height;
                ansi_cells_.assign(static_cast<std::size_t>(std::max(0, size.width)) *
                                       static_cast<std::size_t>(std::max(0, size.height)),
                                   AnsiCell{});
            }
            return Resize{size.width, size.height};
        }

        pollfd fds{};
        fds.fd = STDIN_FILENO;
        fds.events = POLLIN;
        int timeout = block ? poll_timeout_ms_ : 0;
        if (block && input_.has_pending_escape()) {
            timeout = timeout < 0 ? kEscapeTimeoutMs : std::min(timeout, kEscapeTimeoutMs);
        }
        const int ready = poll(&fds, 1, timeout);
        if (ready < 0) {
            if (errno == EINTR) {
                continue;
            }
            return std::nullopt;
        }

        if (ready == 0) {
            if (!block) {
                return std::nullopt;
            }
            if (input_.has_pending_escape()) {
                if (std::optional<Event> escape = input_.flush_escape()) {
                    return escape;
                }
            }
            // Poll timeout expired: return so the application can process timers and
            // periodic idle work (session polling, spinners) instead of sleeping here
            // until the next input byte arrives.
            return std::nullopt;
        }

        if (!(fds.revents & POLLIN)) {
            if (!block) {
                return std::nullopt;
            }
            continue;
        }

        unsigned char buffer[64];
        const ssize_t bytes = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (bytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if (!block) {
                    return std::nullopt;
                }
                continue;
            }
            return std::nullopt;
        }
        if (bytes == 0) {
            if (!block) {
                return std::nullopt;
            }
            continue;
        }

        // Decode every byte of the chunk; leftover events are queued, not dropped.
        for (Event& event : input_.feed_bytes(buffer, static_cast<std::size_t>(bytes))) {
            pending_events_.push_back(std::move(event));
        }

        if (pending_events_.empty() && !block) {
            return std::nullopt;
        }
    }
}

std::optional<Event> AnsiBackend::poll_event() { return read_event(true); }

std::optional<Event> AnsiBackend::poll_event_nonblocking() { return read_event(false); }

void AnsiBackend::begin_frame(BeginFrameOptions options) {
    pending_ansi_draws_.clear();
    pending_image_draws_.clear();
    for (auto& [hash, slot] : kitty_slots_) {
        slot.touched_this_frame = false;
    }

    const Size term = query_terminal_size();
    terminal_width_ = term.width;
    terminal_height_ = term.height;

    const Rect terminal{{0, 0}, term};
    if (terminal.width != ansi_cells_width_ || terminal.height != ansi_cells_height_) {
        ansi_cells_width_ = terminal.width;
        ansi_cells_height_ = terminal.height;
        ansi_cells_.assign(static_cast<std::size_t>(std::max(0, terminal.width)) *
                               static_cast<std::size_t>(std::max(0, terminal.height)),
                           AnsiCell{});
    }

    full_frame_redraw_ = options.full_redraw;
    if (options.full_redraw) {
        frame_clip_ = terminal;
        ansi_clip_ = terminal;
        invalidate_ansi_cells();
        if (options.clear_buffer) {
            cleanup_kitty_graphics();
            if (FILE* output = output_stream()) {
                send_tty_sequence_to(output, "\033[2J\033[H");
            }
        }
        return;
    }

    frame_clip_ = intersect(options.dirty_region, terminal);
    if (options.ansi_visible_region.width > 0 || options.ansi_visible_region.height > 0) {
        ansi_clip_ = intersect(options.ansi_visible_region, terminal);
    } else {
        ansi_clip_ = frame_clip_;
    }
    clear_region(options.dirty_region);

    // Defer the overlap cleanup to flush_image_draws: deleting the graphic
    // here, before the widget tree repaints, would force a delete + full
    // re-transmit every frame even when the image is re-placed unchanged
    // (visible as flicker while streaming into an overlapping dirty region).
    const Rect kitty = kitty_placement_rect();
    kitty_pending_overlap_ =
        kitty.width > 0 && kitty.height > 0 ? intersect(frame_clip_, kitty) : Rect{};
}

Rect AnsiBackend::kitty_placement_rect() const {
    Rect bounds{};
    for (const auto& entry : kitty_slots_) {
        const KittySlot& slot = entry.second;
        if (!slot.ready || slot.cols <= 0 || slot.rows <= 0) {
            continue;
        }

        const Rect placement{slot.x, slot.y, slot.cols, slot.rows};
        if (bounds.width <= 0 || bounds.height <= 0) {
            bounds = placement;
            continue;
        }

        const int left = std::min(bounds.x, placement.x);
        const int top = std::min(bounds.y, placement.y);
        const int right = std::max(bounds.right(), placement.right());
        const int bottom = std::max(bounds.bottom(), placement.bottom());
        bounds = {left, top, right - left, bottom - top};
    }

    return bounds;
}

void AnsiBackend::clear_region(Rect region) {
    const Size term = terminal_size();
    region = intersect(region, {{0, 0}, term});
    if (region.width <= 0 || region.height <= 0) {
        return;
    }

    const std::string blanks(static_cast<std::size_t>(region.width), ' ');
    for (int y = region.y; y < region.bottom(); ++y) {
        queue_ansi_draw(region.x, y, blanks, {});
    }
}

void AnsiBackend::invalidate_graphics() { cleanup_kitty_graphics(); }

void AnsiBackend::present_text_cursor(FILE* output) {
    if (text_cursor_position_.has_value()) {
        const Point position = *text_cursor_position_;
        text_cursor_position_.reset();

        if (output == nullptr) {
            return;
        }

        if (!hardware_text_cursor_visible_) {
            const std::string sequence =
                "\033[" + std::to_string(position.y + 1) + ";" + std::to_string(position.x + 1) + "H\033[?25h";
            send_tty_sequence_to(output, sequence.c_str());
            hardware_text_cursor_visible_ = true;
            placed_text_cursor_ = position;
            return;
        }

        if (!placed_text_cursor_.has_value() || placed_text_cursor_->x != position.x ||
            placed_text_cursor_->y != position.y) {
            const std::string sequence =
                "\033[" + std::to_string(position.y + 1) + ";" + std::to_string(position.x + 1) + "H";
            send_tty_sequence_to(output, sequence.c_str());
            placed_text_cursor_ = position;
        }
        return;
    }

    placed_text_cursor_.reset();
    if (hardware_text_cursor_visible_) {
        if (output != nullptr) {
            send_tty_sequence_to(output, "\033[?25l");
        }
        hardware_text_cursor_visible_ = false;
    }

    if (mouse_cursor_visible_) {
        position_hardware_mouse_cursor(output);
    } else if (output != nullptr) {
        send_tty_sequence_to(output, "\033[?25l");
    }
}

void AnsiBackend::present_frame() {
    FILE* output = output_stream();
    const bool sync_updates = sync_updates_supported_;
    if (sync_updates && output != nullptr) {
        send_tty_sequence_to(output, "\033[?2026h");
    }

    flush_ansi_draws(output);
    flush_image_draws(output);
    present_text_cursor(output);

    if (sync_updates && output != nullptr) {
        send_tty_sequence_to(output, "\033[?2026l");
    }
}

void AnsiBackend::end_frame() { present_frame(); }

bool AnsiBackend::detect_true_color() const {
    const char* colorterm = std::getenv("COLORTERM");
    if (colorterm != nullptr) {
        if (std::strstr(colorterm, "truecolor") != nullptr || std::strstr(colorterm, "24bit") != nullptr ||
            std::strstr(colorterm, "true-color") != nullptr) {
            return true;
        }
    }

    const char* term = std::getenv("TERM");
    if (term != nullptr) {
        if (std::strstr(term, "direct") != nullptr || std::strstr(term, "ghostty") != nullptr ||
            std::strstr(term, "kitty") != nullptr || std::strstr(term, "wezterm") != nullptr ||
            std::strstr(term, "alacritty") != nullptr || std::strstr(term, "foot") != nullptr) {
            return true;
        }
    }

    return false;
}

void AnsiBackend::queue_ansi_draw(int x, int y, std::string_view text, Style style) {
    pending_ansi_draws_.push_back(AnsiDraw{
        x,
        y,
        std::string(text),
        style,
    });
}

void AnsiBackend::emit_ansi_run(FILE* output, int x, int y, std::string_view text, const Style& style) {
    if (output == nullptr || text.empty()) {
        return;
    }

    const Style resolved = style_for_ansi_output(style, true_color_enabled_);

    char header[256];
    int hlen = std::snprintf(header, sizeof(header), "\033[%d;%dH\033[0m", y + 1, x + 1);

    if (true_color_enabled_) {
        if (resolved.foreground_rgb.has_value()) {
            const Rgb& rgb = *resolved.foreground_rgb;
            hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[38;2;%u;%u;%um", rgb.r, rgb.g, rgb.b);
        } else if (resolved.foreground != Color::Default) {
            const int code = ansi_color_code(resolved.foreground, true);
            if (code >= 0) {
                hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[%dm", code);
            }
        }

        if (resolved.background_rgb.has_value()) {
            const Rgb& rgb = *resolved.background_rgb;
            hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[48;2;%u;%u;%um", rgb.r, rgb.g, rgb.b);
        } else if (resolved.background != Color::Default) {
            const int code = ansi_color_code(resolved.background, false);
            if (code >= 0) {
                hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[%dm", code);
            }
        }
    } else {
        if (resolved.foreground != Color::Default) {
            const int code = ansi_color_code(resolved.foreground, true);
            if (code >= 0) {
                hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[%dm", code);
            }
        }
        if (resolved.background != Color::Default) {
            const int code = ansi_color_code(resolved.background, false);
            if (code >= 0) {
                hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[%dm", code);
            }
        }
    }

    if (resolved.bold) {
        hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[1m");
    }
    if (resolved.dim) {
        hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[2m");
    }
    if (resolved.italic) {
        hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[3m");
    }
    if (resolved.underline) {
        hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[4m");
    }
    if (resolved.reverse) {
        hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[7m");
    }
    if (resolved.strikethrough) {
        hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[9m");
    }

    std::fwrite(header, 1, static_cast<std::size_t>(hlen), output);
    std::fwrite(text.data(), 1, text.size(), output);
}

void AnsiBackend::invalidate_ansi_cells() {
    for (AnsiCell& cell : ansi_cells_) {
        cell.known = false;
    }
}

void AnsiBackend::flush_ansi_draws(FILE* output) {
    if (pending_ansi_draws_.empty()) {
        return;
    }

    if (output == nullptr || ansi_cells_width_ <= 0 || ansi_cells_height_ <= 0) {
        pending_ansi_draws_.clear();
        return;
    }

    std::unordered_map<int, std::vector<std::size_t>> draws_by_row;
    for (std::size_t i = 0; i < pending_ansi_draws_.size(); ++i) {
        const AnsiDraw& draw = pending_ansi_draws_[i];
        if (draw.y < ansi_clip_.y || draw.y >= ansi_clip_.bottom() || draw.y >= ansi_cells_height_) {
            continue;
        }
        draws_by_row[draw.y].push_back(i);
    }

    const int clip_left = std::max(0, ansi_clip_.x);
    const int clip_right = std::min(ansi_clip_.right(), ansi_cells_width_);
    const int row_len = clip_right - clip_left;
    if (row_len <= 0) {
        pending_ansi_draws_.clear();
        return;
    }

    std::vector<AnsiCell> row_cur(static_cast<std::size_t>(row_len));
    std::string run_text;

    for (const auto& entry : draws_by_row) {
        const int row_y = entry.first;
        const std::vector<std::size_t>& indices = entry.second;
        const std::size_t row_base = static_cast<std::size_t>(row_y) * static_cast<std::size_t>(ansi_cells_width_);

        for (int i = 0; i < row_len; ++i) {
            row_cur[static_cast<std::size_t>(i)] = ansi_cells_[row_base + static_cast<std::size_t>(clip_left + i)];
        }

        for (const std::size_t idx : indices) {
            const AnsiDraw& draw = pending_ansi_draws_[idx];
            int cx = draw.x;
            for (const TextGlyph& glyph : text_glyph_breaks(draw.text)) {
                const int gw = glyph.width;
                if (cx + gw <= clip_left) {
                    cx += gw;
                    continue;
                }
                if (cx >= clip_right) {
                    break;
                }
                if (cx < clip_left || cx + gw > clip_right) {
                    cx += gw;
                    continue;
                }

                AnsiCell& cell = row_cur[static_cast<std::size_t>(cx - clip_left)];
                if (cell.width == 0 && cx > clip_left) {
                    AnsiCell& owner = row_cur[static_cast<std::size_t>(cx - clip_left - 1)];
                    owner.glyph = " ";
                    owner.style = Style{};
                    owner.width = 1;
                }
                if (cell.width == 2 && cx + 1 < clip_right) {
                    AnsiCell& tail = row_cur[static_cast<std::size_t>(cx - clip_left + 1)];
                    tail.glyph = " ";
                    tail.style = Style{};
                    tail.width = 1;
                }

                cell.glyph = draw.text.substr(glyph.offset, glyph.length);
                cell.style = draw.style;
                cell.width = static_cast<std::uint8_t>(gw);

                if (gw == 2) {
                    AnsiCell& cont = row_cur[static_cast<std::size_t>(cx - clip_left + 1)];
                    if (cont.width == 2 && cx + 2 < clip_right) {
                        AnsiCell& tail = row_cur[static_cast<std::size_t>(cx - clip_left + 2)];
                        tail.glyph = " ";
                        tail.style = Style{};
                        tail.width = 1;
                    }
                    cont.glyph.clear();
                    cont.style = draw.style;
                    cont.width = 0;
                }

                cx += gw;
            }
        }

        run_text.clear();
        Style run_style{};
        int run_start = -1;

        auto flush_run = [&] {
            if (run_start >= 0) {
                emit_ansi_run(output, run_start, row_y, run_text, run_style);
                run_start = -1;
                run_text.clear();
            }
        };

        for (int i = 0; i < row_len; ++i) {
            const int x = clip_left + i;
            AnsiCell& cur = row_cur[static_cast<std::size_t>(i)];
            AnsiCell& prev = ansi_cells_[row_base + static_cast<std::size_t>(x)];

            const bool changed = !prev.known || prev.width != cur.width || prev.glyph != cur.glyph ||
                                 !ansi_style_equal(prev.style, cur.style);
            if (!changed) {
                if (cur.width != 0) {
                    flush_run();
                }
                continue;
            }

            if (cur.width == 0) {
                prev = cur;
                prev.known = true;
                continue;
            }

            if (run_start >= 0 && ansi_style_equal(run_style, cur.style)) {
                run_text += cur.glyph;
            } else {
                flush_run();
                run_start = x;
                run_style = cur.style;
                run_text = cur.glyph;
            }
            prev = cur;
            prev.known = true;
        }
        flush_run();
    }

    std::fflush(output);
    pending_ansi_draws_.clear();
}

void AnsiBackend::flush_image_draws(FILE* output) {
    // Safety net for stale kitty graphics: the frame's dirty region overlapped
    // the placement but no widget re-placed an image — the graphic would cover
    // freshly drawn text, so delete it (after the text flush, not in
    // begin_frame, to avoid delete+retransmit flicker when it IS re-placed).
    const bool replanned = std::any_of(pending_image_draws_.begin(), pending_image_draws_.end(),
                                       [](const ImageDraw& draw) { return !draw.place.empty(); });
    bool stale = false;
    if (kitty_pending_overlap_.width > 0 && kitty_pending_overlap_.height > 0 && !replanned) {
        for (const auto& entry : kitty_slots_) {
            const KittySlot& slot = entry.second;
            if (!slot.ready || slot.cols <= 0 || slot.rows <= 0) {
                continue;
            }
            const Rect placement{slot.x, slot.y, slot.cols, slot.rows};
            if (intersect(kitty_pending_overlap_, placement).width > 0) {
                stale = true;
                break;
            }
        }
    }
    kitty_pending_overlap_ = {};

    if (pending_image_draws_.empty()) {
        if (stale) {
            cleanup_kitty_graphics();
        }
        return;
    }

    if (output == nullptr) {
        pending_image_draws_.clear();
        return;
    }

    for (const ImageDraw& draw : pending_image_draws_) {
        if (!draw.place.empty()) {
            KittySlot& slot = kitty_slots_[draw.slot_id];
            const bool placement_changed = draw.x != slot.x || draw.y != slot.y || draw.cols != slot.cols ||
                                           draw.rows != slot.rows || draw.image_id != slot.live_image_id;

            if (!draw.transmit.empty()) {
                std::fwrite(draw.transmit.data(), 1, draw.transmit.size(), output);
            }

            if (!placement_changed && draw.transmit.empty()) {
                continue;
            }

            if (placement_changed && slot.live_image_id == draw.image_id && slot.live_image_id != 0) {
                // Move/resize of the visible image: do not rely on terminals
                // honoring placement-id replacement — explicitly delete this
                // id's placements first (keeps the transmitted data), then
                // re-place below. Same output burst, so no visible blank.
                const std::string del = encode_kitty_delete(draw.image_id);
                std::fwrite(del.data(), 1, del.size(), output);
            }

            std::fprintf(output, "\033[%d;%dH", draw.y + 1, draw.x + 1);
            std::fwrite(draw.place.data(), 1, draw.place.size(), output);
            if (slot.live_image_id != 0 && slot.live_image_id != draw.image_id) {
                // Ping-pong id switch within this slot: delete the previous
                // frame's id after the new placement is on screen. Transmit and
                // place may be queued as separate draws in the same frame.
                const std::string del = encode_kitty_delete(slot.live_image_id);
                std::fwrite(del.data(), 1, del.size(), output);
            }
            slot.live_image_id = draw.image_id;
            slot.x = draw.x;
            slot.y = draw.y;
            slot.cols = draw.cols;
            slot.rows = draw.rows;
            continue;
        }

        if (!draw.transmit.empty()) {
            std::fprintf(output, "\033[%d;%dH", draw.y + 1, draw.x + 1);
            std::fwrite(draw.transmit.data(), 1, draw.transmit.size(), output);
        }
    }

    for (auto it = kitty_slots_.begin(); it != kitty_slots_.end();) {
        KittySlot& slot = it->second;
        if (slot.touched_this_frame) {
            ++it;
            continue;
        }

        if (slot.live_image_id != 0) {
            const std::string del = encode_kitty_delete(slot.live_image_id);
            std::fwrite(del.data(), 1, del.size(), output);
        }
        if (slot.ping_id_a != 0 && slot.ping_id_a != slot.live_image_id) {
            const std::string del = encode_kitty_delete(slot.ping_id_a);
            std::fwrite(del.data(), 1, del.size(), output);
        }
        if (slot.ping_id_b != 0 && slot.ping_id_b != slot.live_image_id && slot.ping_id_b != slot.ping_id_a) {
            const std::string del = encode_kitty_delete(slot.ping_id_b);
            std::fwrite(del.data(), 1, del.size(), output);
        }
        it = kitty_slots_.erase(it);
    }

    if (stale) {
        cleanup_kitty_graphics();
    }

    std::fflush(output);
    pending_image_draws_.clear();
}

void AnsiBackend::draw_image(int x, int y, Size cell_size, const TerminalImage& image) {
    if (image.empty() || x < 0 || y < 0) {
        return;
    }

    const GraphicsProtocol protocol = active_graphics_protocol();
    if (protocol == GraphicsProtocol::Kitty) {
        const std::uint32_t hash = terminal_image_content_hash(image);
        if (hash == 0) {
            return;
        }

        std::uint32_t slot_id = 0;
        if (const std::optional<std::uint32_t> existing =
                find_kitty_slot(hash, x, y, cell_size.width, cell_size.height)) {
            slot_id = *existing;
        } else {
            slot_id = next_kitty_slot_id_++;
            kitty_slots_[slot_id] = KittySlot{};
        }

        KittySlot& slot = kitty_slots_[slot_id];
        slot.touched_this_frame = true;

        std::uint32_t image_id = slot.live_image_id;
        if (!slot.ready || hash != slot.content_hash) {
            if (slot.ping_id_a == 0) {
                slot.ping_id_a = next_kitty_image_id_++;
                slot.ping_id_b = next_kitty_image_id_++;
                slot.ping_use_a = false;
            } else {
                slot.ping_use_a = !slot.ping_use_a;
            }

            image_id = slot.ping_use_a ? slot.ping_id_a : slot.ping_id_b;
            slot.content_hash = hash;
            slot.ready = true;
            pending_image_draws_.push_back(ImageDraw{
                x,
                y,
                cell_size.width,
                cell_size.height,
                image_id,
                slot_id,
                encode_kitty_transmit(image, image_id),
                {},
            });
        }

        pending_image_draws_.push_back(ImageDraw{
            x,
            y,
            cell_size.width,
            cell_size.height,
            image_id,
            slot_id,
            {},
            encode_kitty_place(cell_size.width, cell_size.height, image_id),
        });
        return;
    }

    ImageDraw draw{
        x,
        y,
        cell_size.width,
        cell_size.height,
        0,
        0U,
        encode_terminal_image(protocol, image, x, y, cell_size.width, cell_size.height),
        {},
    };
    if (!draw.transmit.empty()) {
        pending_image_draws_.push_back(std::move(draw));
    }
}

void AnsiBackend::draw_text(int x, int y, std::string_view text, Style style) {
    if (text.empty() || y < 0 || x < 0) {
        return;
    }

    const Size term = terminal_size();
    if (y >= term.height || x >= term.width) {
        return;
    }

    std::string sanitized;
    sanitized.reserve(text.size());
    for (char ch : text) {
        // C0 controls (incl. ESC) and DEL must never reach the terminal raw:
        // widget text is user-controlled and could otherwise inject escape sequences.
        const unsigned char byte = static_cast<unsigned char>(ch);
        sanitized.push_back(byte < 32 || byte == 127 ? ' ' : ch);
    }
    sanitized.resize(text_byte_length_for_width(sanitized, term.width - x));
    if (sanitized.empty()) {
        return;
    }

    const Style draw_style = style_for_ansi_output(style, true_color_enabled_);
    queue_ansi_draw(x, y, sanitized, draw_style);
}

} // namespace tuinator::detail
