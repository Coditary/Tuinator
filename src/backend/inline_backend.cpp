#include <tuinator/backend/inline_backend.hpp>
#include <tuinator/render/color.hpp>
#include <tuinator/render/text.hpp>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <string>
#include <thread>
#include <vector>

#if TUINATOR_PLATFORM_POSIX
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace tuinator {

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

bool uses_relative_draw(const InlineBackendOptions& options) {
    return options.anchor_row <= 0 && !options.pin_to_bottom;
}

#if TUINATOR_PLATFORM_POSIX

std::optional<KeyPress> decode_key_byte(unsigned char byte) {
    if (byte == '\r' || byte == '\n') {
        return KeyPress{Key::Enter, '\0'};
    }
    if (byte == 127 || byte == 8) {
        return KeyPress{Key::Backspace, '\0'};
    }
    if (byte == 27) {
        return KeyPress{Key::Escape, '\0'};
    }
    if (byte >= 1 && byte <= 26) {
        KeyPress press{};
        press.ctrl = true;
        press.character = static_cast<char>('a' + byte - 1);
        return press;
    }
    if (byte >= 32 && byte <= 126) {
        return KeyPress{Key::Unknown, static_cast<char>(byte)};
    }
    return std::nullopt;
}

#endif

} // namespace

InlineTerminalBackend::InlineTerminalBackend(InlineBackendOptions options)
    : options_(std::move(options)), relative_draw_(uses_relative_draw(options_)), true_color_(options_.true_color) {
    if (options_.output != nullptr) {
        output_ = options_.output;
    }
}

InlineTerminalBackend::~InlineTerminalBackend() {
    if (initialized_) {
        shutdown_impl();
    }
    if (owned_tty_ != nullptr) {
        std::fclose(owned_tty_);
        owned_tty_ = nullptr;
        output_ = stdout;
    }
}

void InlineTerminalBackend::write_output(std::string_view data) {
    if (data.empty()) {
        return;
    }
    std::fwrite(data.data(), 1, data.size(), output_);
    std::fflush(output_);
}

int InlineTerminalBackend::compute_region_height(int term_height, int available_rows) const {
    const int clamped_term = std::clamp(term_height, 1, 256);
    const int clamped_available = std::clamp(available_rows, 1, 256);

    int height = options_.height > 0 ? options_.height : std::max(8, clamped_term / 3);
    height = std::max(options_.min_height, height);
    if (options_.max_height > 0) {
        height = std::min(options_.max_height, height);
    }
    height = std::min(height, clamped_available);
    height = std::min(height, clamped_term);
    return std::max(1, height);
}

void InlineTerminalBackend::place_anchor(int term_height) {
    if (options_.anchor_row > 0) {
        anchor_row_ = options_.anchor_row;
    } else if (options_.pin_to_bottom) {
        anchor_row_ = std::max(1, term_height - region_height_ + 1);
    } else if (cursor_anchor_row_ > 0) {
        anchor_row_ = cursor_anchor_row_;
    } else {
        anchor_row_ = std::max(1, term_height - region_height_ + 1);
    }

    const int max_anchor = std::max(1, term_height - region_height_ + 1);
    anchor_row_ = std::clamp(anchor_row_, 1, max_anchor);
}

void InlineTerminalBackend::sync_geometry_relative(const Size& term) {
    region_width_ = std::clamp(term.width, 1, 512);
    const int term_height = std::clamp(term.height, 1, 256);
    const int new_height = compute_region_height(term_height, term_height);
    if (band_height_ <= 0 || new_height != band_height_) {
        band_height_ = new_height;
    }
    region_height_ = band_height_;
}

Size InlineTerminalBackend::query_terminal_size() const {
    if (options_.terminal_size_query) {
        const Size override_size = options_.terminal_size_query();
        if (override_size.width > 0 && override_size.height > 0) {
            return override_size;
        }
    }
#if TUINATOR_PLATFORM_POSIX
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
#endif
    return {80, 24};
}

void InlineTerminalBackend::sync_geometry(const Size& term, bool allow_reanchor) {
    if (relative_draw_) {
        sync_geometry_relative(term);
        return;
    }

    const int term_width = std::clamp(term.width, 1, 512);
    const int term_height = std::clamp(term.height, 1, 256);

    region_width_ = term_width;

    if (!anchor_locked_ || allow_reanchor) {
        const int provisional_available = cursor_anchor_row_ > 0 && !options_.pin_to_bottom && options_.anchor_row <= 0
                                              ? std::max(1, term_height - cursor_anchor_row_ + 1)
                                              : term_height;
        region_height_ = compute_region_height(term_height, provisional_available);
        place_anchor(term_height);
        const int available = std::max(1, term_height - anchor_row_ + 1);
        region_height_ = std::min(region_height_, available);
    } else {
        const int max_anchor = std::max(1, term_height - region_height_ + 1);
        anchor_row_ = std::clamp(anchor_row_, 1, max_anchor);
        const int available = std::max(1, term_height - anchor_row_ + 1);
        region_height_ = std::min(region_height_, available);
    }
}

void InlineTerminalBackend::init() {
    if (initialized_) {
        return;
    }

    relative_draw_ = uses_relative_draw(options_);
    if (output_ == nullptr) {
#if TUINATOR_PLATFORM_POSIX
        owned_tty_ = std::fopen("/dev/tty", "we");
#endif
        output_ = owned_tty_ != nullptr ? owned_tty_ : stdout;
        if (output_ != nullptr) {
            std::setvbuf(output_, nullptr, _IONBF, 0);
        }
    }

    std::fflush(stdout);
    std::fflush(stderr);

    const Size term = query_terminal_size();
    sync_geometry(term, true);

    anchor_locked_ = true;
    resize_buffer(region_width_, region_height_);
    previous_cells_.assign(static_cast<std::size_t>(region_height_),
                           std::vector<Cell>(static_cast<std::size_t>(region_width_)));

    std::fputs("\033[?25l", output_);
    std::fflush(output_);
    acquire_stdin();
    initialized_ = true;
}

void InlineTerminalBackend::shutdown() { shutdown_impl(); }

void InlineTerminalBackend::shutdown_impl() {
    if (!initialized_) {
        return;
    }

    if (options_.clear_on_shutdown) {
        clear_terminal_region();
    }

    if (relative_draw_) {
        fputc('\n', output_);
    }

    drain_stdin();
    release_stdin();

    std::fputs("\033[?25h", output_);
    std::fflush(output_);
    initialized_ = false;
    anchor_locked_ = false;
    relative_draw_ = false;
    cursor_anchor_row_ = 0;
    band_height_ = 0;
    last_emitted_band_height_ = 0;
    frames_drawn_ = 0;
}

void InlineTerminalBackend::resize_buffer(int width, int height) {
    region_width_ = std::clamp(width, 1, 512);
    region_height_ = std::clamp(height, 1, 128);
    cells_.assign(static_cast<std::size_t>(region_height_), std::vector<Cell>(static_cast<std::size_t>(region_width_)));
}

Size InlineTerminalBackend::terminal_size() const { return {region_width_, region_height_}; }

void InlineTerminalBackend::acquire_stdin() {
#if TUINATOR_PLATFORM_POSIX
    if (stdin_captured_) {
        return;
    }

    if (isatty(STDIN_FILENO)) {
        if (tcgetattr(STDIN_FILENO, &stdin_original_) != 0) {
            return;
        }

        termios raw = stdin_original_;
        raw.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
        raw.c_iflag &= static_cast<tcflag_t>(~(IXON | ICRNL));
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) {
            return;
        }
    } else if (!options_.keyboard_input) {
        return;
    } else {
        stdin_original_flags_ = fcntl(STDIN_FILENO, F_GETFL, 0);
        if (stdin_original_flags_ >= 0) {
            fcntl(STDIN_FILENO, F_SETFL, stdin_original_flags_ | O_NONBLOCK);
            stdin_nonblocking_set_ = true;
        }
    }

    stdin_captured_ = true;
    drain_stdin();
#endif
}

void InlineTerminalBackend::release_stdin() {
#if TUINATOR_PLATFORM_POSIX
    if (!stdin_captured_) {
        return;
    }

    if (isatty(STDIN_FILENO)) {
        tcsetattr(STDIN_FILENO, TCSANOW, &stdin_original_);
    }
    if (stdin_nonblocking_set_) {
        fcntl(STDIN_FILENO, F_SETFL, stdin_original_flags_);
        stdin_nonblocking_set_ = false;
    }
    stdin_captured_ = false;
#endif
}

void InlineTerminalBackend::drain_stdin() {
#if TUINATOR_PLATFORM_POSIX
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
#endif
}

std::optional<Event> InlineTerminalBackend::read_stdin_event(bool allow_block) {
#if TUINATOR_PLATFORM_POSIX
    if (!stdin_captured_) {
        return std::nullopt;
    }

    unsigned char byte = 0;
    const ssize_t bytes = read(STDIN_FILENO, &byte, 1);
    if (bytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return std::nullopt;
        }
        return std::nullopt;
    }
    if (bytes == 0) {
        if (!allow_block || poll_timeout_ms_ <= 0) {
            return std::nullopt;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(poll_timeout_ms_));
        return std::nullopt;
    }

    if (!options_.keyboard_input) {
        return std::nullopt;
    }

    if (const std::optional<KeyPress> key = decode_key_byte(byte)) {
        return *key;
    }
#else
    (void)allow_block;
#endif
    return std::nullopt;
}

std::optional<Event> InlineTerminalBackend::poll_event_nonblocking() {
    if (!initialized_) {
        return std::nullopt;
    }

    if (!options_.keyboard_input) {
        drain_stdin();
        return std::nullopt;
    }

    return read_stdin_event(false);
}

std::optional<Event> InlineTerminalBackend::poll_event() {
#if TUINATOR_PLATFORM_POSIX
    const Size term = query_terminal_size();
    const int old_width = region_width_;
    const int old_height = region_height_;
    if (relative_draw_) {
        sync_geometry_relative(term);
        if (region_width_ != old_width || region_height_ != old_height) {
            resize_buffer(region_width_, band_height_);
            return Resize{region_width_, band_height_};
        }
    } else {
        sync_geometry(term, false);
        if (region_width_ != old_width || region_height_ != old_height) {
            resize_buffer(region_width_, region_height_);
            return Resize{region_width_, region_height_};
        }
    }
#endif

    if (!options_.keyboard_input) {
        drain_stdin();
    } else if (const std::optional<Event> key = read_stdin_event(false)) {
        return key;
    }

    if (poll_timeout_ms_ > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(poll_timeout_ms_));
        if (!options_.keyboard_input) {
            drain_stdin();
        } else if (const std::optional<Event> key_after_wait = read_stdin_event(false)) {
            return key_after_wait;
        }
    }
    return std::nullopt;
}

void InlineTerminalBackend::set_poll_timeout_ms(int timeout_ms) { poll_timeout_ms_ = timeout_ms; }

void InlineTerminalBackend::begin_frame(BeginFrameOptions options) {
    if (options.full_redraw) {
        if (options.clear_buffer) {
            for (auto& row : cells_) {
                for (Cell& cell : row) {
                    cell = Cell{};
                }
            }
        }
        return;
    }

    Rect region = intersect(options.dirty_region, {{0, 0}, terminal_size()});
    if (region.width <= 0 || region.height <= 0) {
        return;
    }

    for (int y = region.y; y < region.bottom(); ++y) {
        for (int x = region.x; x < region.right(); ++x) {
            cells_[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] = Cell{};
        }
    }
}

bool InlineTerminalBackend::style_equal(const Style& a, const Style& b) const {
    return a.foreground == b.foreground && a.background == b.background && a.foreground_rgb == b.foreground_rgb &&
           a.background_rgb == b.background_rgb && a.bold == b.bold && a.dim == b.dim && a.reverse == b.reverse;
}

bool InlineTerminalBackend::cell_equal(const Cell& a, const Cell& b) const {
    return a.text == b.text && a.wide_tail == b.wide_tail && style_equal(a.style, b.style);
}

void InlineTerminalBackend::end_frame() {
    emit_frame_to_terminal();
    previous_cells_ = cells_;
}

void InlineTerminalBackend::draw_text(int x, int y, std::string_view text, Style style) {
    if (text.empty() || y < 0 || y >= region_height_) {
        return;
    }

    int column = x;
    std::size_t offset = 0;
    while (offset < text.size() && column < region_width_) {
        if (text[offset] == '\n') {
            break;
        }

        const std::size_t bytes = text_byte_length_for_width(text.substr(offset), 1);
        if (bytes == 0) {
            break;
        }

        const std::string grapheme(text.substr(offset, bytes));
        const int width = std::max(1, text_display_width(grapheme));

        if (column >= 0) {
            cells_[static_cast<std::size_t>(y)][static_cast<std::size_t>(column)] = {grapheme, style, false};
            for (int w = 1; w < width && column + w < region_width_; ++w) {
                cells_[static_cast<std::size_t>(y)][static_cast<std::size_t>(column + w)] = {"", style, true};
            }
        }

        column += width;
        offset += bytes;
    }
}

void InlineTerminalBackend::append_style(std::string& out, const Style& style) const {
    if (true_color_ && style.foreground_rgb.has_value()) {
        const Rgb& rgb = *style.foreground_rgb;
        out += "\033[38;2;" + std::to_string(rgb.r) + ';' + std::to_string(rgb.g) + ';' + std::to_string(rgb.b) + 'm';
    } else if (style.foreground != Color::Default) {
        const int code = ansi_color_code(style.foreground, true);
        if (code >= 0) {
            out += "\033[" + std::to_string(code) + 'm';
        }
    }

    if (true_color_ && style.background_rgb.has_value()) {
        const Rgb& rgb = *style.background_rgb;
        out += "\033[48;2;" + std::to_string(rgb.r) + ';' + std::to_string(rgb.g) + ';' + std::to_string(rgb.b) + 'm';
    } else if (style.background != Color::Default) {
        const int code = ansi_color_code(style.background, false);
        if (code >= 0) {
            out += "\033[" + std::to_string(code) + 'm';
        }
    }

    if (style.bold) {
        out += "\033[1m";
    }
    if (style.dim) {
        out += "\033[2m";
    }
    if (style.reverse) {
        out += "\033[7m";
    }
}

int InlineTerminalBackend::last_nonempty_column(int y) const {
    for (int x = region_width_ - 1; x >= 0; --x) {
        const Cell& cell = cells_[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
        if (cell.wide_tail) {
            continue;
        }
        if (!cell.text.empty() && cell.text != " ") {
            return x;
        }
        if (cell.style.foreground != Color::Default || cell.style.background != Color::Default ||
            cell.style.foreground_rgb.has_value() || cell.style.background_rgb.has_value() || cell.style.bold ||
            cell.style.dim || cell.style.reverse) {
            return x;
        }
    }
    return -1;
}

void InlineTerminalBackend::append_erase_lines(std::string& out, int count) const {
    for (int i = 0; i < count; ++i) {
        out += "\r\033[2K";
        if (i + 1 < count) {
            out += "\033[1A";
        }
    }
    if (count > 0) {
        out += "\r";
    }
}

void InlineTerminalBackend::write_row_content(int y, std::string& out) const {
    const int last_col = last_nonempty_column(y);
    if (last_col < 0) {
        return;
    }

    Style active_style{};
    bool have_style = false;
    std::string run;

    auto flush_run = [&]() {
        if (run.empty()) {
            return;
        }
        out += "\033[0m";
        append_style(out, active_style);
        out += run;
        run.clear();
        have_style = false;
    };

    for (int x = 0; x <= last_col; ++x) {
        const Cell& cell = cells_[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
        if (cell.wide_tail) {
            continue;
        }

        const std::string glyph = cell.text.empty() ? " " : cell.text;
        if (!have_style || !style_equal(cell.style, active_style)) {
            flush_run();
            active_style = cell.style;
            have_style = true;
        }
        run += glyph;
    }

    flush_run();
    out += "\033[0m";
}

void InlineTerminalBackend::write_row_absolute(int abs_row, int y) const {
    std::string row;
    row += "\033[" + std::to_string(abs_row) + ";1H\033[2K";
    write_row_content(y, row);
    std::fputs(row.c_str(), output_);
}

void InlineTerminalBackend::emit_frame_relative() {
    std::string frame;
#if TUINATOR_PLATFORM_POSIX
    if (frames_drawn_ > 0) {
        frame += "\033[?2026h";
    }
#endif
    if (frames_drawn_ > 0) {
        append_erase_lines(frame, std::max(band_height_, last_emitted_band_height_));
    }

    for (int y = 0; y < band_height_; ++y) {
        frame += "\r\033[2K";
        write_row_content(y, frame);
        if (y + 1 < band_height_) {
            frame += '\n';
        }
    }

#if TUINATOR_PLATFORM_POSIX
    if (frames_drawn_ > 0) {
        frame += "\033[?2026l";
    }
#endif

    write_output(frame);
    last_emitted_band_height_ = band_height_;
    ++frames_drawn_;
}

void InlineTerminalBackend::emit_frame_to_terminal() {
    if (relative_draw_) {
        const bool size_changed =
            previous_cells_.size() != cells_.size() || (previous_cells_.empty() ? 0 : previous_cells_.front().size()) !=
                                                           (cells_.empty() ? 0 : cells_.front().size());
        if (size_changed) {
            previous_cells_.assign(cells_.size(), std::vector<Cell>(cells_.empty() ? 0 : cells_.front().size()));
        }

        bool changed = size_changed;
        if (!changed) {
            for (int y = 0; y < region_height_; ++y) {
                for (int x = 0; x < region_width_; ++x) {
                    if (!cell_equal(cells_[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)],
                                    previous_cells_[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)])) {
                        changed = true;
                        break;
                    }
                }
                if (changed) {
                    break;
                }
            }
        }

        if (changed) {
            emit_frame_relative();
        }
        return;
    }

    if (previous_cells_.size() != cells_.size() || (previous_cells_.empty() ? 0 : previous_cells_.front().size()) !=
                                                       (cells_.empty() ? 0 : cells_.front().size())) {
        previous_cells_.assign(cells_.size(), std::vector<Cell>(cells_.empty() ? 0 : cells_.front().size()));
        for (int y = 0; y < region_height_; ++y) {
            write_row_absolute(anchor_row_ + y, y);
        }
        std::fflush(output_);
        return;
    }

    bool wrote = false;
    for (int y = 0; y < region_height_; ++y) {
        bool row_changed = false;
        for (int x = 0; x < region_width_; ++x) {
            if (!cell_equal(cells_[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)],
                            previous_cells_[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)])) {
                row_changed = true;
                break;
            }
        }
        if (row_changed) {
            write_row_absolute(anchor_row_ + y, y);
            wrote = true;
        }
    }

    if (wrote) {
        std::fflush(output_);
    }
}

void InlineTerminalBackend::clear_terminal_region() {
    if (relative_draw_) {
        if (frames_drawn_ <= 0) {
            return;
        }

        std::string clear;
        append_erase_lines(clear, std::max(band_height_, last_emitted_band_height_));
        write_output(clear);
        return;
    }

    for (int y = 0; y < region_height_; ++y) {
        std::fprintf(output_, "\033[%d;1H\033[2K", anchor_row_ + y);
    }
    std::fflush(output_);
}

std::unique_ptr<InlineTerminalBackend> InlineTerminalBackend::create(InlineBackendOptions options) {
    return std::make_unique<InlineTerminalBackend>(std::move(options));
}

} // namespace tuinator
