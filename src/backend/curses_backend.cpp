#include <tuinator/core/event.hpp>
#include <tuinator/debug/startup_profiler.hpp>
#include <tuinator/render/color.hpp>
#include <tuinator/render/graphics_encode.hpp>
#include <tuinator/render/graphics_protocol.hpp>
#include <tuinator/render/terminal_image.hpp>
#include <tuinator/render/text.hpp>
#include <tuinator/render/tty_overlay.hpp>

#include <array>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <string>
#include <vector>

#include "backend/curses_backend.hpp"
#include "backend/curses_config.hpp"

namespace tuinator::detail {

namespace {

constexpr int kColorCount = 8;
constexpr int kPairSlots = 1 + kColorCount + kColorCount + (kColorCount * kColorCount);

constexpr std::array<Color, kColorCount> kPalette = {
    Color::Black, Color::Red, Color::Green, Color::Yellow, Color::Blue, Color::Magenta, Color::Cyan, Color::White,
};

const char* mouse_debug_log_path() {
#if TUINATOR_PLATFORM_WINDOWS
    static char path[512];
    const char* temp = std::getenv("TEMP");
    if (temp != nullptr && temp[0] != '\0') {
        std::snprintf(path, sizeof(path), "%s\\tuinator-mouse.log", temp);
        return path;
    }

    return "tuinator-mouse.log";
#else
    return "/tmp/tuinator-mouse.log";
#endif
}

Key map_key(int ch) {
    switch (ch) {
    case KEY_RESIZE: return Key::Resize;
    case 27: return Key::Escape;
    case '\n':
    case '\r':
    case KEY_ENTER: return Key::Enter;
    case '\t': return Key::Tab;
    case KEY_BTAB: return Key::BackTab;
    case KEY_BACKSPACE:
    case 127:
    case 8: return Key::Backspace;
    case KEY_UP: return Key::Up;
    case KEY_DOWN: return Key::Down;
    case KEY_LEFT: return Key::Left;
    case KEY_RIGHT: return Key::Right;
    case KEY_HOME: return Key::Home;
    case KEY_END: return Key::End;
    case KEY_PPAGE: return Key::PageUp;
    case KEY_NPAGE: return Key::PageDown;
    case KEY_DC: return Key::Delete;
    default: return Key::Unknown;
    }
}

int rgb8_to_curses(std::uint8_t value) { return (static_cast<int>(value) * 1000 + 127) / 255; }

int to_curses_color(Color color) {
    switch (color) {
    case Color::Black: return COLOR_BLACK;
    case Color::Red: return COLOR_RED;
    case Color::Green: return COLOR_GREEN;
    case Color::Yellow: return COLOR_YELLOW;
    case Color::Blue: return COLOR_BLUE;
    case Color::Magenta: return COLOR_MAGENTA;
    case Color::Cyan: return COLOR_CYAN;
    case Color::White: return COLOR_WHITE;
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

Style style_for_truecolor_ansi(Style style) {
    if (!style.foreground_rgb.has_value() && style.foreground != Color::Default) {
        style.foreground_rgb = palette_to_rgb(style.foreground);
    }
    if (!style.background_rgb.has_value() && style.background != Color::Default) {
        style.background_rgb = palette_to_rgb(style.background);
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

int pair_slot_for_codes(int fg_code, int bg_code) {
    if (fg_code == -1 && bg_code == -1) {
        return 0;
    }

    if (fg_code != -1 && bg_code == -1) {
        for (int i = 0; i < kColorCount; ++i) {
            if (to_curses_color(kPalette[static_cast<std::size_t>(i)]) == fg_code) {
                return 1 + i;
            }
        }
        return 0;
    }

    if (fg_code == -1 && bg_code != -1) {
        for (int i = 0; i < kColorCount; ++i) {
            if (to_curses_color(kPalette[static_cast<std::size_t>(i)]) == bg_code) {
                return 1 + kColorCount + i;
            }
        }
        return 0;
    }

    int fg_index = -1;
    int bg_index = -1;
    for (int i = 0; i < kColorCount; ++i) {
        if (to_curses_color(kPalette[static_cast<std::size_t>(i)]) == fg_code) {
            fg_index = i;
        }
        if (to_curses_color(kPalette[static_cast<std::size_t>(i)]) == bg_code) {
            bg_index = i;
        }
    }

    if (fg_index < 0 || bg_index < 0) {
        return 0;
    }

    return 1 + kColorCount + kColorCount + (fg_index * kColorCount) + bg_index;
}

void send_tty_sequence_to(FILE* output, const char* sequence) {
    if (output == nullptr || sequence == nullptr || sequence[0] == '\0') {
        return;
    }

    fputs(sequence, output);
    fflush(output);
}

// Synchronized output (CSI ? 2026 h/l) makes each frame atomic: the terminal
// buffers all writes between the markers and presents them at once, so even a
// full-screen clear+repaint does not flicker. Unknown private modes are ignored
// by terminals that lack support, but we still gate on a allowlist plus env
// override (TUINATOR_SYNC_UPDATE=0/1) to stay conservative.
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
#if TUINATOR_PLATFORM_WINDOWS
        return true;
#else
        return false;
#endif
    }

    return std::strstr(term, "xterm") != nullptr || std::strstr(term, "rxvt") != nullptr ||
           std::strstr(term, "screen") != nullptr || std::strstr(term, "tmux") != nullptr ||
           std::strstr(term, "alacritty") != nullptr || std::strstr(term, "kitty") != nullptr ||
           std::strstr(term, "foot") != nullptr || std::strstr(term, "wezterm") != nullptr ||
           std::strstr(term, "ghostty") != nullptr || std::strstr(term, "vscode") != nullptr;
}

bool mouse_debug_enabled() {
    const char* debug = std::getenv("TUINATOR_MOUSE_DEBUG");
    return debug != nullptr && debug[0] != '\0' && std::strcmp(debug, "0") != 0;
}

void debug_mouse(const char* message) {
    if (!mouse_debug_enabled()) {
        return;
    }

    if (FILE* log = std::fopen(mouse_debug_log_path(), "a")) {
        std::fprintf(log, "tuinator-mouse: %s\n", message);
        std::fclose(log);
    }
}

void debug_mouse_event(int ch, const MEVENT& mouse) {
    if (!mouse_debug_enabled()) {
        return;
    }

    if (FILE* log = std::fopen(mouse_debug_log_path(), "a")) {
        std::fprintf(log, "tuinator-mouse: ch=%d KEY_MOUSE=%d x=%d y=%d bstate=0x%lx\n", ch, KEY_MOUSE, mouse.x,
                     mouse.y, static_cast<unsigned long>(mouse.bstate));
        std::fclose(log);
    }
}

MouseButton mouse_button_from_state(mmask_t state) {
#ifdef BUTTON3_PRESSED
    if (state &
        (BUTTON3_PRESSED | BUTTON3_RELEASED | BUTTON3_CLICKED | BUTTON3_DOUBLE_CLICKED | BUTTON3_TRIPLE_CLICKED)) {
        return MouseButton::Right;
    }
#endif

#ifdef BUTTON2_PRESSED
    if (state &
        (BUTTON2_PRESSED | BUTTON2_RELEASED | BUTTON2_CLICKED | BUTTON2_DOUBLE_CLICKED | BUTTON2_TRIPLE_CLICKED)) {
        return MouseButton::Middle;
    }
#endif

    return MouseButton::Left;
}

MouseAction mouse_action_from_state(mmask_t state) {
    const mmask_t clicked = BUTTON1_CLICKED
#ifdef BUTTON2_CLICKED
                            | BUTTON2_CLICKED
#endif
#ifdef BUTTON3_CLICKED
                            | BUTTON3_CLICKED
#endif
        ;
    if (state & clicked) {
        return MouseAction::Click;
    }

    const mmask_t released = BUTTON1_RELEASED
#ifdef BUTTON2_RELEASED
                             | BUTTON2_RELEASED
#endif
#ifdef BUTTON3_RELEASED
                             | BUTTON3_RELEASED
#endif
        ;
    if (state & released) {
        return MouseAction::Release;
    }

#ifdef BUTTON4_PRESSED
    if (state & (BUTTON4_PRESSED | BUTTON4_RELEASED | BUTTON4_CLICKED)) {
        return MouseAction::WheelUp;
    }
#endif

#ifdef BUTTON5_PRESSED
    if (state & (BUTTON5_PRESSED | BUTTON5_RELEASED | BUTTON5_CLICKED)) {
        return MouseAction::WheelDown;
    }
#endif

#ifdef BUTTON6_PRESSED
    if (state & (BUTTON6_PRESSED | BUTTON6_RELEASED | BUTTON6_CLICKED)) {
        return MouseAction::WheelLeft;
    }
#endif

#ifdef BUTTON7_PRESSED
    if (state & (BUTTON7_PRESSED | BUTTON7_RELEASED | BUTTON7_CLICKED)) {
        return MouseAction::WheelRight;
    }
#endif

    const mmask_t pressed = BUTTON1_PRESSED | BUTTON1_DOUBLE_CLICKED | BUTTON1_TRIPLE_CLICKED
#ifdef BUTTON2_PRESSED
                            | BUTTON2_PRESSED | BUTTON2_DOUBLE_CLICKED | BUTTON2_TRIPLE_CLICKED
#endif
#ifdef BUTTON3_PRESSED
                            | BUTTON3_PRESSED | BUTTON3_DOUBLE_CLICKED | BUTTON3_TRIPLE_CLICKED
#endif
        ;
    if (state & pressed) {
        if (state & REPORT_MOUSE_POSITION) {
            return MouseAction::Move;
        }
        return MouseAction::Press;
    }

    if (state & REPORT_MOUSE_POSITION) {
        return MouseAction::Move;
    }

    return MouseAction::Move;
}

void draw_text_cells(int y, int x, std::string_view text, attr_t attributes, int pair, bool extended_pair) {
    if (y < 0 || x < 0 || text.empty()) {
        return;
    }

    if (y >= LINES || x >= COLS) {
        return;
    }

    std::string sanitized;
    sanitized.reserve(text.size());
    for (char ch : text) {
        sanitized.push_back(ch == '\n' ? ' ' : ch);
    }
    const std::size_t fit = text_byte_length_for_width(sanitized, COLS - x);
    sanitized.resize(fit);
    if (sanitized.empty()) {
        return;
    }
    text = sanitized;
#if defined(TUINATOR_BACKEND_NCURSES)
    std::mbstate_t state{};
    const char* source = text.data();
    const std::size_t needed = std::mbsrtowcs(nullptr, &source, 0, &state);
    if (needed != static_cast<std::size_t>(-1)) {
        std::vector<wchar_t> wide(needed);
        source = text.data();
        state = {};
        if (std::mbsrtowcs(wide.data(), &source, needed, &state) != static_cast<std::size_t>(-1)) {
            if (extended_pair) {
                attr_set(attributes, static_cast<short>(pair), nullptr);
                mvaddnwstr(y, x, wide.data(), static_cast<int>(wide.size()));
                attr_set(A_NORMAL, 0, nullptr);
                return;
            }

            if (pair > 0) {
                attron(COLOR_PAIR(pair) | attributes);
                mvaddnwstr(y, x, wide.data(), static_cast<int>(wide.size()));
                attroff(COLOR_PAIR(pair) | attributes);
                return;
            }

            attron(attributes);
            mvaddnwstr(y, x, wide.data(), static_cast<int>(wide.size()));
            attroff(attributes);
            return;
        }
    }
#else
    (void)extended_pair;
#endif

    if (pair > 0) {
        attron(COLOR_PAIR(pair) | attributes);
        mvaddnstr(y, x, text.data(), static_cast<int>(text.size()));
        attroff(COLOR_PAIR(pair) | attributes);
        return;
    }

    attron(attributes);
    mvaddnstr(y, x, text.data(), static_cast<int>(text.size()));
    attroff(attributes);
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
        fputs("\033[?1000l\033[?1002l\033[?1003l\033[?1006l", tty);
        fputs("\033[0m\033[?25h", tty);
        fflush(tty);
        fclose(tty);
    }
}

void atexit_restore_terminal() { restore_terminal_state(); }

} // namespace

CursesBackend::CursesBackend() = default;

CursesBackend::~CursesBackend() { shutdown(); }

FILE* CursesBackend::output_stream() const {
    if (tty_out_ != nullptr) {
        return tty_out_;
    }

    return stdout;
}

void CursesBackend::write_tty_sequence(const char* sequence) { send_tty_sequence_to(output_stream(), sequence); }

void CursesBackend::close_terminal_streams() {
    if (tty_in_ != nullptr) {
        std::fclose(tty_in_);
        tty_in_ = nullptr;
    }

    if (tty_out_ != nullptr) {
        std::fclose(tty_out_);
        tty_out_ = nullptr;
    }

    unified_output_ = false;
}

void CursesBackend::init_curses_screen() {
#if TUINATOR_PLATFORM_POSIX && defined(TUINATOR_BACKEND_NCURSES)
    tty_out_ = std::fopen("/dev/tty", "w");
    tty_in_ = std::fopen("/dev/tty", "r");
    if (tty_out_ != nullptr && tty_in_ != nullptr) {
        const char* term = std::getenv("TERM");
        screen_ = newterm(term != nullptr ? term : "xterm-256color", tty_out_, tty_in_);
        if (screen_ != nullptr) {
            set_term(screen_);
            unified_output_ = true;
            return;
        }
    }

    close_terminal_streams();
#endif

    initscr();
}

void CursesBackend::init() {
    if (initialized_) {
        return;
    }

    startup_profile_mark("curses.before_initscr");
#if defined(TUINATOR_BACKEND_NCURSES)
    setlocale(LC_ALL, "");
    use_env(TRUE);
    use_extended_names(TRUE);
#endif
    init_curses_screen();
    startup_profile_mark("curses.after_initscr");

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    leaveok(stdscr, TRUE);
    scrollok(stdscr, FALSE);
    timeout(poll_timeout_ms_);

    if (alternate_screen()) {
        write_tty_sequence("\033[?1049h");
        alternate_screen_active_ = true;
    }

    erase();
    refresh();
    startup_profile_mark("curses.after_initial_refresh");

    if (has_colors()) {
        startup_profile_mark("curses.before_colors");
        start_color();
#if defined(TUINATOR_BACKEND_NCURSES)
        use_default_colors();
#endif
        colors_enabled_ = true;
        true_color_enabled_ = detect_true_color();
        sync_updates_supported_ = detect_sync_updates();
#if defined(TUINATOR_BACKEND_NCURSES) && defined(NCURSES_EXT_FUNCS)
        if (true_color_enabled_) {
            extended_colors_available_ = init_extended_color(kExtendedColorBase, 1000, 0, 0) != ERR;
        }
#else
        extended_colors_available_ = false;
#endif
        setup_default_color_pair();
        startup_profile_mark("curses.after_colors");
    }

    enable_mouse();

    static bool atexit_registered = false;
    if (!atexit_registered) {
        std::atexit(atexit_restore_terminal);
        atexit_registered = true;
    }

    initialized_ = true;
    startup_profile_mark("curses.init_done");
}

void CursesBackend::enable_mouse() {
    if (!has_mouse()) {
        debug_mouse("has_mouse() returned false");
    }

    mouse_cursor_user_enabled_ = [] {
        const char* setting = std::getenv("TUINATOR_MOUSE_CURSOR");
        return setting != nullptr && setting[0] != '\0' && std::strcmp(setting, "0") != 0;
    }();
    mouse_cursor_visible_ = mouse_cursor_user_enabled_ && !mouse_cursor_suppressed_;

    constexpr mmask_t kWanted = ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION;

    mmask_t available{};
    const mmask_t enabled = mousemask(kWanted, &available);
    mouseinterval(0);
    mouse_enabled_ = enabled != 0;

    if (terminal_name_suggests_xterm_mouse()) {
        // Reset then enable SGR + drag tracking. Write to /dev/tty — not stdout/endwin.
        // Default mode 1003 (all motion). Override: TUINATOR_MOUSE_TRACK=1002
        write_tty_sequence("\033[?1000l\033[?1002l\033[?1003l\033[?1006l");

        char enable[48];
        const int mode = mouse_tracking_mode();
        std::snprintf(enable, sizeof(enable), "\033[?%dh\033[?1006h", mode);
        write_tty_sequence(enable);

        xterm_mouse_enabled_ = true;
        mouse_enabled_ = true;

        debug_mouse(enable);
    }

    if (mouse_debug_enabled()) {
        if (FILE* log = std::fopen(mouse_debug_log_path(), "a")) {
            std::fprintf(log,
                         "tuinator-mouse: backend=%s TERM=%s has_mouse=%d mousemask=0x%lx available=0x%lx xterm_ext=%d "
                         "track=%d KEY_MOUSE=%d\n",
                         TUINATOR_BACKEND_NAME, std::getenv("TERM") ? std::getenv("TERM") : "(null)",
                         has_mouse() ? 1 : 0, static_cast<unsigned long>(enabled),
                         static_cast<unsigned long>(available), xterm_mouse_enabled_ ? 1 : 0, mouse_tracking_mode(),
                         KEY_MOUSE);
            std::fclose(log);
        }
    }
}

void CursesBackend::disable_mouse() {
    if (xterm_mouse_enabled_) {
        write_tty_sequence("\033[?1000l\033[?1002l\033[?1003l\033[?1006l");
        xterm_mouse_enabled_ = false;
    }
}

int CursesBackend::mouse_tracking_mode() const {
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

void CursesBackend::set_poll_timeout_ms(int timeout_ms) {
    poll_timeout_ms_ = timeout_ms;
    timeout(timeout_ms);
}

void CursesBackend::position_hardware_mouse_cursor(FILE* output) {
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

void CursesBackend::refresh_mouse_cursor() { position_hardware_mouse_cursor(output_stream()); }

void CursesBackend::set_mouse_cursor_suppressed(bool suppressed) {
    mouse_cursor_suppressed_ = suppressed;
    mouse_cursor_visible_ = mouse_cursor_user_enabled_ && !mouse_cursor_suppressed_;
}

void CursesBackend::set_text_cursor(std::optional<Point> position) { text_cursor_position_ = position; }

void CursesBackend::cleanup_kitty_graphics() {
    if (FILE* output = output_stream()) {
        cleanup_kitty_graphics_on_tty(output);
        std::fflush(output);
    }
    kitty_image_ready_ = false;
    kitty_cached_hash_ = 0;
    last_kitty_placement_ = {};
}

void CursesBackend::shutdown() {
    if (!initialized_) {
        restore_terminal_state();
        return;
    }

    disable_mouse();
    cleanup_kitty_graphics();
    if (FILE* output = output_stream()) {
        if (alternate_screen_active_) {
            send_tty_sequence_to(output, "\033[?1049l");
            alternate_screen_active_ = false;
        } else if (clear_on_shutdown()) {
            send_tty_sequence_to(output, "\033[2J");
        }
    }
    reset_tty_attributes();
    endwin();
#if defined(TUINATOR_BACKEND_NCURSES)
    if (screen_ != nullptr) {
        delscreen(screen_);
        screen_ = nullptr;
    }
#endif
    close_terminal_streams();
    restore_terminal_state();
    initialized_ = false;
    colors_enabled_ = false;
    true_color_enabled_ = false;
    extended_colors_available_ = false;
    mouse_enabled_ = false;
    pair_cache_.fill(0);
    extended_color_cache_.clear();
    extended_pair_cache_.clear();
    pending_ansi_draws_.clear();
    next_pair_id_ = 2;
    next_extended_color_ = kExtendedColorBase;
    next_extended_pair_ = kExtendedPairBase;
    left_button_down_ = false;
}

Size CursesBackend::terminal_size() const {
    int height = 0;
    int width = 0;
    getmaxyx(stdscr, height, width);
    return {width, height};
}

std::optional<Event> CursesBackend::read_event(bool block) {
    if (!block) {
        nodelay(stdscr, TRUE);
    }

    const int ch = getch();

    if (!block) {
        nodelay(stdscr, FALSE);
    }

    if (ch == ERR) {
        return std::nullopt;
    }

    if (ch == KEY_RESIZE) {
        invalidate_graphics();
        const auto size = terminal_size();
        return Resize{size.width, size.height};
    }

    if (ch == KEY_MOUSE) {
        MEVENT mouse{};
        if (getmouse(&mouse) != OK) {
            debug_mouse("getmouse() failed");
            return std::nullopt;
        }

        debug_mouse_event(ch, mouse);

        const mmask_t state = mouse.bstate;
        if (state & BUTTON1_PRESSED) {
            left_button_down_ = true;
        }
        if (state & BUTTON1_RELEASED || state & BUTTON1_CLICKED) {
            left_button_down_ = false;
        }

        MouseEvent event{};
        event.position = {mouse.x, mouse.y};
        event.action = mouse_action_from_state(state);
        event.button = mouse_button_from_state(state);
        event.left_pressed = left_button_down_;
        last_mouse_position_ = event.position;
        return event;
    }

    if (ch == 27) {
        if (!block) {
            nodelay(stdscr, TRUE);
        }
        const int next = getch();
        if (!block) {
            nodelay(stdscr, FALSE);
        }
        if (next != ERR) {
            KeyPress alt_press{};
            alt_press.key = Key::Unknown;
            alt_press.alt = true;
            if (next >= 32 && next <= 126) {
                alt_press.character = static_cast<char>(next);
            }
            return alt_press;
        }
        return KeyPress{Key::Escape, '\0'};
    }

    const Key mapped = map_key(ch);
    if (mapped != Key::Unknown) {
        return KeyPress{mapped, '\0'};
    }

    if (ch >= 1 && ch <= 26) {
        KeyPress ctrl_press{};
        ctrl_press.key = Key::Unknown;
        ctrl_press.ctrl = true;
        ctrl_press.character = static_cast<char>('a' + ch - 1);
        return ctrl_press;
    }

    KeyPress press{};
    press.key = Key::Unknown;
    if (ch >= 32 && ch <= 126) {
        press.character = static_cast<char>(ch);
    }

    return press;
}

std::optional<Event> CursesBackend::poll_event() { return read_event(true); }

std::optional<Event> CursesBackend::poll_event_nonblocking() { return read_event(false); }

void CursesBackend::begin_frame(BeginFrameOptions options) {
    pending_ansi_draws_.clear();
    pending_image_draws_.clear();

    const Rect terminal{{0, 0}, terminal_size()};
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
        // Full frames paint non-RGB cells through curses, which the ANSI shadow
        // buffer cannot see; everything known so far is stale afterwards.
        invalidate_ansi_cells();
        if (options.clear_buffer) {
            cleanup_kitty_graphics();
            if (FILE* output = output_stream()) {
                send_tty_sequence_to(output, "\033[2J");
            }
            clear();
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
    if (true_color_enabled_) {
        // True-color partial frames repaint through the diffed ANSI flush, so
        // blanking the band directly on the tty here would only flash - and it
        // would happen outside the synchronized-update bracket. Just drop kitty
        // placements overlapping the band.
        const Rect kitty = kitty_placement_rect();
        if (kitty.width > 0 && kitty.height > 0) {
            const Rect overlap = intersect(frame_clip_, kitty);
            if (overlap.width > 0 && overlap.height > 0) {
                cleanup_kitty_graphics();
            }
        }
    } else {
        clear_partial_overlays(frame_clip_);
    }
}

void CursesBackend::clear_partial_overlays(Rect region) {
    if (region.width <= 0 || region.height <= 0) {
        return;
    }

    if (FILE* output = output_stream()) {
        clear_tty_overlay_region(output, region, terminal_size());
    }

    const Rect kitty = kitty_placement_rect();
    if (kitty.width > 0 && kitty.height > 0) {
        const Rect overlap = intersect(region, kitty);
        if (overlap.width > 0 && overlap.height > 0) {
            cleanup_kitty_graphics();
        }
    }
}

Rect CursesBackend::kitty_placement_rect() const {
    if (!kitty_image_ready_ || last_kitty_placement_.cols <= 0 || last_kitty_placement_.rows <= 0) {
        return {};
    }

    return {last_kitty_placement_.x, last_kitty_placement_.y, last_kitty_placement_.cols, last_kitty_placement_.rows};
}

void CursesBackend::clear_region(Rect region) {
    const Size term = terminal_size();
    region = intersect(region, {{0, 0}, term});
    if (region.width <= 0 || region.height <= 0) {
        return;
    }

    if (true_color_enabled_) {
        // True-color partial frames bypass curses entirely (text is flushed as direct
        // ANSI), so blanks written to stdscr would never reach the screen. Queue the
        // erase as ANSI draws instead; they flush before this frame's widget draws.
        const std::string blanks(static_cast<std::size_t>(region.width), ' ');
        for (int y = region.y; y < region.bottom(); ++y) {
            queue_ansi_draw(region.x, y, blanks, {});
        }
        return;
    }

    const attr_t attrs = has_colors() ? COLOR_PAIR(1) : A_NORMAL;
    for (int y = region.y; y < region.bottom(); ++y) {
        for (int x = region.x; x < region.right(); ++x) {
            mvaddch(y, x, ' ' | attrs);
        }
    }
}

void CursesBackend::invalidate_graphics() { cleanup_kitty_graphics(); }

void CursesBackend::prepare_refresh(FILE* output) {
    (void)output;
    move(0, 0);
}

void CursesBackend::present_text_cursor(FILE* output) {
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

void CursesBackend::present_frame() {
    FILE* output = output_stream();
    const bool sync_updates = sync_updates_supported_;
    if (sync_updates && output != nullptr) {
        send_tty_sequence_to(output, "\033[?2026h");
    }

    prepare_refresh(output);
    if (full_frame_redraw_) {
        refresh();
    } else if (!true_color_enabled_) {
        // touchline() refreshes whole rows and would stomp ANSI in sibling panes.
        for (int y = frame_clip_.y; y < frame_clip_.bottom(); ++y) {
            touchline(stdscr, y, 1);
        }
        wnoutrefresh(stdscr);
        doupdate();
    }
    flush_ansi_draws(output);
    flush_image_draws(output);
    present_text_cursor(output);

    if (sync_updates && output != nullptr) {
        send_tty_sequence_to(output, "\033[?2026l");
    }
}

void CursesBackend::end_frame() { present_frame(); }

void CursesBackend::setup_default_color_pair() {
    init_pair(1, -1, -1);
    pair_cache_[0] = 1;
}

int CursesBackend::ensure_color_pair(int fg_code, int bg_code) {
    const int slot = pair_slot_for_codes(fg_code, bg_code);
    if (slot <= 0) {
        return 1;
    }

    if (pair_cache_[static_cast<std::size_t>(slot)] == 0) {
        const int pair_id = next_pair_id_++;
        init_pair(pair_id, fg_code, bg_code);
        pair_cache_[static_cast<std::size_t>(slot)] = static_cast<std::int16_t>(pair_id);
    }

    return pair_cache_[static_cast<std::size_t>(slot)];
}

int CursesBackend::ensure_extended_color(Rgb rgb) {
    const std::uint32_t key = (static_cast<std::uint32_t>(rgb.r) << 16U) | (static_cast<std::uint32_t>(rgb.g) << 8U) |
                              static_cast<std::uint32_t>(rgb.b);

    if (const auto it = extended_color_cache_.find(key); it != extended_color_cache_.end()) {
        return it->second;
    }

    const int color_id = next_extended_color_++;
#if defined(TUINATOR_BACKEND_NCURSES) && defined(NCURSES_EXT_FUNCS)
    if (init_extended_color(color_id, rgb8_to_curses(rgb.r), rgb8_to_curses(rgb.g), rgb8_to_curses(rgb.b)) == ERR) {
        return COLOR_WHITE;
    }
#endif
    extended_color_cache_.emplace(key, color_id);
    return color_id;
}

int CursesBackend::ensure_extended_pair(int fg_id, int bg_id) {
    const std::uint64_t key =
        (static_cast<std::uint64_t>(static_cast<std::uint32_t>(fg_id)) << 32U) | static_cast<std::uint32_t>(bg_id);

    if (const auto it = extended_pair_cache_.find(key); it != extended_pair_cache_.end()) {
        return it->second;
    }

    const int pair_id = next_extended_pair_++;
#if defined(TUINATOR_BACKEND_NCURSES) && defined(NCURSES_EXT_FUNCS)
    if (init_extended_pair(pair_id, fg_id, bg_id) == ERR) {
        return 1;
    }
#else
    init_pair(pair_id, fg_id, bg_id);
#endif
    extended_pair_cache_.emplace(key, pair_id);
    return pair_id;
}

int CursesBackend::resolve_extended_color_id(Color palette, const std::optional<Rgb>& rgb) {
    if (rgb.has_value()) {
        return ensure_extended_color(*rgb);
    }

    if (palette == Color::Default) {
        return -1;
    }

    return ensure_extended_color(palette_to_rgb(palette));
}

bool CursesBackend::detect_true_color() const {
#if !defined(TUINATOR_BACKEND_NCURSES)
    (void)this;
    return false;
#else
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
#endif
}

bool CursesBackend::style_uses_rgb(const Style& style) const {
    return style.foreground_rgb.has_value() || style.background_rgb.has_value();
}

void CursesBackend::queue_ansi_draw(int x, int y, std::string_view text, Style style) {
    pending_ansi_draws_.push_back(AnsiDraw{
        x,
        y,
        std::string(text),
        style,
    });
}

void CursesBackend::emit_ansi_run(FILE* output, int x, int y, std::string_view text, const Style& style) {
    if (output == nullptr || text.empty()) {
        return;
    }

    const Style resolved = style_for_truecolor_ansi(style);

    char header[192];
    int hlen = std::snprintf(header, sizeof(header), "\033[%d;%dH\033[0m", y + 1, x + 1);

    if (resolved.foreground_rgb.has_value()) {
        const Rgb& rgb = *resolved.foreground_rgb;
        hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[38;2;%u;%u;%um", rgb.r, rgb.g, rgb.b);
    }

    if (resolved.background_rgb.has_value()) {
        const Rgb& rgb = *resolved.background_rgb;
        hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[48;2;%u;%u;%um", rgb.r, rgb.g, rgb.b);
    }

    if (resolved.bold) {
        hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[1m");
    }

    if (resolved.dim) {
        hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[2m");
    }

    if (resolved.reverse) {
        hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[7m");
    }

    std::fwrite(header, 1, static_cast<std::size_t>(hlen), output);
    std::fwrite(text.data(), 1, text.size(), output);
}

void CursesBackend::invalidate_ansi_cells() {
    for (AnsiCell& cell : ansi_cells_) {
        cell.known = false;
    }
}

void CursesBackend::flush_ansi_draws(FILE* output) {
    if (pending_ansi_draws_.empty()) {
        return;
    }

    if (output == nullptr || ansi_cells_width_ <= 0 || ansi_cells_height_ <= 0) {
        pending_ansi_draws_.clear();
        return;
    }

    // Group draws by terminal row, preserving paint order within a row.
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

        // Compose this frame's draws onto the previous screen state.
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
                    // Straddles the clip edge: terminals cannot draw half a glyph.
                    cx += gw;
                    continue;
                }

                AnsiCell& cell = row_cur[static_cast<std::size_t>(cx - clip_left)];
                if (cell.width == 0 && cx > clip_left) {
                    // Overwriting the second half of a wide glyph destroys its owner.
                    AnsiCell& owner = row_cur[static_cast<std::size_t>(cx - clip_left - 1)];
                    owner.glyph = " ";
                    owner.style = Style{};
                    owner.width = 1;
                }
                if (cell.width == 2 && cx + 1 < clip_right) {
                    // Overwriting the first half of a wide glyph clears its tail.
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

        // Diff against the shadow buffer and emit only changed runs.
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
                // Continuation cell: emitted via its owner's wide glyph.
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

void CursesBackend::flush_image_draws(FILE* output) {
    if (pending_image_draws_.empty()) {
        return;
    }

    if (output == nullptr) {
        pending_image_draws_.clear();
        return;
    }

    for (const ImageDraw& draw : pending_image_draws_) {
        if (!draw.place.empty()) {
            const bool placement_changed = draw.x != last_kitty_placement_.x || draw.y != last_kitty_placement_.y ||
                                           draw.cols != last_kitty_placement_.cols ||
                                           draw.rows != last_kitty_placement_.rows;

            if (!draw.transmit.empty()) {
                std::fwrite(draw.transmit.data(), 1, draw.transmit.size(), output);
                last_kitty_placement_.hash = kitty_cached_hash_;
            }

            if (!placement_changed && draw.transmit.empty()) {
                continue;
            }

            std::fprintf(output, "\033[%d;%dH", draw.y + 1, draw.x + 1);
            std::fwrite(draw.place.data(), 1, draw.place.size(), output);
            last_kitty_placement_ = {draw.x, draw.y, draw.cols, draw.rows, kitty_cached_hash_};
            continue;
        }

        if (!draw.transmit.empty()) {
            std::fprintf(output, "\033[%d;%dH", draw.y + 1, draw.x + 1);
            std::fwrite(draw.transmit.data(), 1, draw.transmit.size(), output);
        }
    }

    std::fflush(output);
    pending_image_draws_.clear();
}

void CursesBackend::draw_image(int x, int y, Size cell_size, const TerminalImage& image) {
    if (image.empty() || x < 0 || y < 0) {
        return;
    }

    const GraphicsProtocol protocol = active_graphics_protocol();
    if (protocol == GraphicsProtocol::Kitty) {
        ImageDraw draw{
            x, y, cell_size.width, cell_size.height, {}, encode_kitty_place(cell_size.width, cell_size.height),
        };

        const std::uint32_t hash = terminal_image_content_hash(image);
        if (!kitty_image_ready_ || hash != kitty_cached_hash_) {
            draw.transmit = encode_kitty_transmit(image);
            kitty_cached_hash_ = hash;
            kitty_image_ready_ = true;
        }

        if (!draw.place.empty()) {
            pending_image_draws_.push_back(std::move(draw));
        }
        return;
    }

    ImageDraw draw{
        x,
        y,
        cell_size.width,
        cell_size.height,
        encode_terminal_image(protocol, image, x, y, cell_size.width, cell_size.height),
        {},
    };
    if (!draw.transmit.empty()) {
        pending_image_draws_.push_back(std::move(draw));
    }
}

int CursesBackend::extended_color_pair_for(Style style) {
    const int fg = resolve_extended_color_id(style.foreground, style.foreground_rgb);
    const int bg = resolve_extended_color_id(style.background, style.background_rgb);
    return ensure_extended_pair(fg, bg);
}

int CursesBackend::color_pair_for(Style style) {
    if (!colors_enabled_) {
        return 0;
    }

    const bool wants_rgb = style.foreground_rgb.has_value() || style.background_rgb.has_value();

    if (true_color_enabled_ && wants_rgb && extended_colors_available_) {
        return extended_color_pair_for(style);
    }

    if (wants_rgb) {
        const Color fg_palette =
            style.foreground != Color::Default
                ? style.foreground
                : (style.foreground_rgb.has_value() ? nearest_ansi_color(*style.foreground_rgb) : Color::Default);
        const Color bg_palette =
            style.background != Color::Default
                ? style.background
                : (style.background_rgb.has_value() ? nearest_ansi_color(*style.background_rgb) : Color::Default);

        const int fg = fg_palette == Color::Default ? -1 : to_curses_color(fg_palette);
        const int bg = bg_palette == Color::Default ? -1 : to_curses_color(bg_palette);
        return ensure_color_pair(fg, bg);
    }

    const int fg = style.foreground == Color::Default ? -1 : to_curses_color(style.foreground);
    const int bg = style.background == Color::Default ? -1 : to_curses_color(style.background);

    if (fg == -1 && bg == -1) {
        return 1;
    }

    return ensure_color_pair(fg, bg);
}

void CursesBackend::draw_text(int x, int y, std::string_view text, Style style) {
    if (text.empty() || y < 0 || x < 0) {
        return;
    }

    const Size term = terminal_size();
    if (y >= term.height || x >= term.width) {
        return;
    }
    text = text.substr(0, text_byte_length_for_width(text, term.width - x));
    if (text.empty()) {
        return;
    }

    attr_t attributes = A_NORMAL;
    if (style.bold) {
        attributes |= A_BOLD;
    }
    if (style.dim) {
        attributes |= A_DIM;
    }
    if (style.reverse) {
        attributes |= A_REVERSE;
    }
    if (style.underline) {
        attributes |= A_UNDERLINE;
    }
#ifdef A_ITALIC
    if (style.italic) {
        attributes |= A_ITALIC;
    }
#endif

    const int pair = color_pair_for(style);
    const bool has_rgb = style.foreground_rgb.has_value() || style.background_rgb.has_value();
    const bool partial_frame = !full_frame_redraw_;

    if (true_color_enabled_ && (has_rgb || partial_frame)) {
        const Style draw_style = partial_frame && !has_rgb ? style_for_truecolor_ansi(style) : style;
        queue_ansi_draw(x, y, text, draw_style);
        return;
    }

    const bool extended_pair = extended_colors_available_ && has_rgb;
    const bool extended_draw = extended_pair;
    draw_text_cells(y, x, text, attributes, pair, extended_draw);
}

} // namespace tuinator::detail
