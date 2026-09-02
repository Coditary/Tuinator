#include "backend/curses_backend.hpp"

#include "backend/curses_config.hpp"

#include <tuinator/core/event.hpp>
#include <tuinator/debug/startup_profiler.hpp>

#include <array>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <vector>
#include <tuinator/render/color.hpp>
#include <tuinator/render/graphics_encode.hpp>
#include <tuinator/render/graphics_protocol.hpp>
#include <tuinator/render/terminal_image.hpp>
#include <tuinator/render/text.hpp>

namespace tuinator::detail {

namespace {

constexpr int kColorCount = 8;
constexpr int kPairSlots = 1 + kColorCount + kColorCount + (kColorCount * kColorCount);

constexpr std::array<Color, kColorCount> kPalette = {
    Color::Black, Color::Red, Color::Green, Color::Yellow,
    Color::Blue, Color::Magenta, Color::Cyan, Color::White,
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
    case 27:         return Key::Escape;
    case '\n':
    case '\r':
    case KEY_ENTER:  return Key::Enter;
    case '\t':       return Key::Tab;
    case KEY_BTAB:   return Key::BackTab;
    case KEY_BACKSPACE:
    case 127:
    case 8:          return Key::Backspace;
    case KEY_UP:     return Key::Up;
    case KEY_DOWN:   return Key::Down;
    case KEY_LEFT:   return Key::Left;
    case KEY_RIGHT:  return Key::Right;
    case KEY_HOME:   return Key::Home;
    case KEY_END:    return Key::End;
    case KEY_PPAGE:  return Key::PageUp;
    case KEY_NPAGE:  return Key::PageDown;
    case KEY_DC:     return Key::Delete;
    default:         return Key::Unknown;
    }
}

int rgb8_to_curses(std::uint8_t value) {
    return (static_cast<int>(value) * 1000 + 127) / 255;
}

int to_curses_color(Color color) {
    switch (color) {
    case Color::Black:   return COLOR_BLACK;
    case Color::Red:     return COLOR_RED;
    case Color::Green:   return COLOR_GREEN;
    case Color::Yellow:  return COLOR_YELLOW;
    case Color::Blue:    return COLOR_BLUE;
    case Color::Magenta: return COLOR_MAGENTA;
    case Color::Cyan:    return COLOR_CYAN;
    case Color::White:   return COLOR_WHITE;
    case Color::Default: return -1;
    }
    return -1;
}

Rgb palette_to_rgb(Color color) {
    switch (color) {
    case Color::Black:   return {0, 0, 0};
    case Color::Red:     return {220, 50, 47};
    case Color::Green:   return {80, 200, 120};
    case Color::Yellow:  return {220, 200, 50};
    case Color::Blue:    return {80, 120, 220};
    case Color::Magenta: return {200, 80, 200};
    case Color::Cyan:    return {80, 200, 220};
    case Color::White:   return {230, 230, 230};
    case Color::Default: return {};
    }
    return {};
}

FILE* open_tty_output() {
    return std::fopen("/dev/tty", "we");
}

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

bool terminal_name_suggests_xterm_mouse() {
    const char* term = std::getenv("TERM");
    if (term == nullptr) {
#if TUINATOR_PLATFORM_WINDOWS
        return true;
#else
        return false;
#endif
    }

    return std::strstr(term, "xterm") != nullptr
        || std::strstr(term, "rxvt") != nullptr
        || std::strstr(term, "screen") != nullptr
        || std::strstr(term, "tmux") != nullptr
        || std::strstr(term, "alacritty") != nullptr
        || std::strstr(term, "kitty") != nullptr
        || std::strstr(term, "foot") != nullptr
        || std::strstr(term, "wezterm") != nullptr
        || std::strstr(term, "ghostty") != nullptr
        || std::strstr(term, "vscode") != nullptr;
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
        std::fprintf(log, "tuinator-mouse: ch=%d KEY_MOUSE=%d x=%d y=%d bstate=0x%lx\n",
                     ch, KEY_MOUSE, mouse.x, mouse.y, static_cast<unsigned long>(mouse.bstate));
        std::fclose(log);
    }
}

MouseAction mouse_action_from_state(mmask_t state) {
    if (state & BUTTON1_CLICKED) {
        return MouseAction::Click;
    }

    if (state & BUTTON1_RELEASED) {
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

    if (state & (BUTTON1_PRESSED | BUTTON1_DOUBLE_CLICKED | BUTTON1_TRIPLE_CLICKED)) {
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

void atexit_restore_terminal() {
    restore_terminal_state();
}

} // namespace

CursesBackend::CursesBackend() = default;

CursesBackend::~CursesBackend() {
    shutdown();
}

FILE* CursesBackend::output_stream() const {
    if (tty_out_ != nullptr) {
        return tty_out_;
    }

    return stdout;
}

void CursesBackend::write_tty_sequence(const char* sequence) {
    send_tty_sequence_to(output_stream(), sequence);
}

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
#if defined(TUINATOR_BACKEND_NCURSES) && defined(NCURSES_EXT_FUNCS)
        if (true_color_enabled_) {
            extended_colors_available_ =
                init_extended_color(kExtendedColorBase, 1000, 0, 0) != ERR;
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

    mouse_cursor_visible_ = [] {
        const char* setting = std::getenv("TUINATOR_MOUSE_CURSOR");
        return setting == nullptr || setting[0] == '\0' || std::strcmp(setting, "0") != 0;
    }();

    constexpr mmask_t kWanted = ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION;

    mmask_t available{};
    const mmask_t enabled = mousemask(kWanted, &available);
    mouseinterval(0);
    mouse_enabled_ = enabled != 0;

    if (terminal_name_suggests_xterm_mouse()) {
        // Reset then enable SGR + drag tracking. Write to /dev/tty — not stdout/endwin.
        // Default mode 1003 (all motion). Override: TUINATOR_MOUSE_TRACK=1002
        write_tty_sequence("\033[?1000l\033[?1002l\033[?1003l\033[?1006l");

        char enable[40];
        const int mode = mouse_tracking_mode();
        if (mode == 1003 || mode == 1002) {
            std::snprintf(enable, sizeof(enable), "\033[?1006;%dh", mode);
        } else {
            std::snprintf(enable, sizeof(enable), "\033[?%dh", mode);
        }
        write_tty_sequence(enable);

        xterm_mouse_enabled_ = true;
        mouse_enabled_ = true;

        debug_mouse(enable);
    }

    if (mouse_debug_enabled()) {
        if (FILE* log = std::fopen(mouse_debug_log_path(), "a")) {
            std::fprintf(log,
                         "tuinator-mouse: backend=%s TERM=%s has_mouse=%d mousemask=0x%lx available=0x%lx xterm_ext=%d track=%d KEY_MOUSE=%d\n",
                         TUINATOR_BACKEND_NAME,
                         std::getenv("TERM") ? std::getenv("TERM") : "(null)",
                         has_mouse() ? 1 : 0,
                         static_cast<unsigned long>(enabled),
                         static_cast<unsigned long>(available),
                         xterm_mouse_enabled_ ? 1 : 0,
                         mouse_tracking_mode(),
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
        return 1003;
    }

    if (std::strcmp(mode, "1002") == 0) {
        return 1002;
    }
    if (std::strcmp(mode, "1000") == 0) {
        return 1000;
    }

    return 1003;
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
    if (position.x < 0 || position.y < 0
        || position.x >= term.width || position.y >= term.height) {
        send_tty_sequence_to(output, "\033[?25l");
        return;
    }

    char sequence[48];
    std::snprintf(sequence, sizeof(sequence), "\033[%d;%dH\033[?25h",
                  position.y + 1, position.x + 1);
    send_tty_sequence_to(output, sequence);
}

void CursesBackend::refresh_mouse_cursor() {
    position_hardware_mouse_cursor(output_stream());
}

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

std::optional<Event> CursesBackend::poll_event() {
    return read_event(true);
}

std::optional<Event> CursesBackend::poll_event_nonblocking() {
    return read_event(false);
}

void CursesBackend::begin_frame(BeginFrameOptions options) {
    pending_ansi_draws_.clear();
    pending_image_draws_.clear();

    if (options.full_redraw) {
        clear();
        return;
    }

    clear_region(options.dirty_region);
}

void CursesBackend::clear_region(Rect region) {
    const Size term = terminal_size();
    region = intersect(region, {{0, 0}, term});
    if (region.width <= 0 || region.height <= 0) {
        return;
    }

    const attr_t attrs = has_colors() ? COLOR_PAIR(1) : A_NORMAL;
    for (int y = region.y; y < region.bottom(); ++y) {
        for (int x = region.x; x < region.right(); ++x) {
            mvaddch(y, x, ' ' | attrs);
        }
    }
}

void CursesBackend::invalidate_graphics() {
    cleanup_kitty_graphics();
}

void CursesBackend::prepare_refresh(FILE* output) {
    if (output != nullptr) {
        send_tty_sequence_to(output, "\033[?25l\033[H");
    }

    // Keep ncurses' logical cursor aligned with the physical home position.
    move(0, 0);
}

void CursesBackend::present_frame() {
    FILE* output = output_stream();
    prepare_refresh(output);
    refresh();
    flush_ansi_draws(output);
    flush_image_draws(output);
    position_hardware_mouse_cursor(output);
}

void CursesBackend::end_frame() {
    present_frame();
}

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
    const std::uint32_t key =
        (static_cast<std::uint32_t>(rgb.r) << 16U)
        | (static_cast<std::uint32_t>(rgb.g) << 8U)
        | static_cast<std::uint32_t>(rgb.b);

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
        (static_cast<std::uint64_t>(static_cast<std::uint32_t>(fg_id)) << 32U)
        | static_cast<std::uint32_t>(bg_id);

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
        if (std::strstr(colorterm, "truecolor") != nullptr
            || std::strstr(colorterm, "24bit") != nullptr
            || std::strstr(colorterm, "true-color") != nullptr) {
            return true;
        }
    }

    const char* term = std::getenv("TERM");
    if (term != nullptr) {
        if (std::strstr(term, "direct") != nullptr
            || std::strstr(term, "ghostty") != nullptr
            || std::strstr(term, "kitty") != nullptr
            || std::strstr(term, "wezterm") != nullptr
            || std::strstr(term, "alacritty") != nullptr
            || std::strstr(term, "foot") != nullptr) {
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

void CursesBackend::draw_text_ansi(FILE* output, int x, int y, std::string_view text, Style style) {
    if (output == nullptr) {
        return;
    }

    char header[128];
    int hlen = 0;

    if (style.foreground_rgb.has_value()) {
        const Rgb& rgb = *style.foreground_rgb;
        hlen += std::snprintf(header + hlen, sizeof(header) - hlen,
                              "\033[38;2;%u;%u;%um", rgb.r, rgb.g, rgb.b);
    }

    if (style.background_rgb.has_value()) {
        const Rgb& rgb = *style.background_rgb;
        hlen += std::snprintf(header + hlen, sizeof(header) - hlen,
                              "\033[48;2;%u;%u;%um", rgb.r, rgb.g, rgb.b);
    }

    if (style.bold) {
        hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[1m");
    }

    if (style.dim) {
        hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[2m");
    }

    if (style.reverse) {
        hlen += std::snprintf(header + hlen, sizeof(header) - hlen, "\033[7m");
    }

    std::fprintf(output, "\033[%d;%dH", y + 1, x + 1);
    if (hlen > 0) {
        std::fwrite(header, 1, static_cast<std::size_t>(hlen), output);
    }
    if (!text.empty()) {
        std::fwrite(text.data(), 1, text.size(), output);
    }
    std::fputs("\033[0m", output);
}

void CursesBackend::flush_ansi_draws(FILE* output) {
    if (pending_ansi_draws_.empty()) {
        return;
    }

    if (output == nullptr) {
        pending_ansi_draws_.clear();
        return;
    }

    for (const AnsiDraw& draw : pending_ansi_draws_) {
        draw_text_ansi(output, draw.x, draw.y, draw.text, draw.style);
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
            const bool placement_changed =
                draw.x != last_kitty_placement_.x
                || draw.y != last_kitty_placement_.y
                || draw.cols != last_kitty_placement_.cols
                || draw.rows != last_kitty_placement_.rows;

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
            x,
            y,
            cell_size.width,
            cell_size.height,
            {},
            encode_kitty_place(cell_size.width, cell_size.height),
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

    const bool wants_rgb =
        style.foreground_rgb.has_value() || style.background_rgb.has_value();

    if (true_color_enabled_ && wants_rgb && extended_colors_available_) {
        return extended_color_pair_for(style);
    }

    if (wants_rgb) {
        const Color fg_palette = style.foreground != Color::Default
            ? style.foreground
            : (style.foreground_rgb.has_value() ? nearest_ansi_color(*style.foreground_rgb) : Color::Default);
        const Color bg_palette = style.background != Color::Default
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

    const int pair = color_pair_for(style);
    const bool has_rgb =
        style.foreground_rgb.has_value() || style.background_rgb.has_value();
    const bool extended_pair = extended_colors_available_ && has_rgb;
    const bool ansi_only = true_color_enabled_ && has_rgb && !extended_pair;

    if (ansi_only) {
        queue_ansi_draw(x, y, text, style);
        return;
    }

    const bool extended_draw = extended_pair;
    draw_text_cells(y, x, text, attributes, pair, extended_draw);
}

} // namespace tuinator::detail
