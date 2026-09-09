#pragma once

#include <tuinator/backend/platform.hpp>
#include <tuinator/core/event.hpp>
#include <tuinator/core/geometry.hpp>
#include <tuinator/render/style.hpp>

#include <memory>
#include <optional>
#include <string_view>

namespace tuinator {

class TerminalImage;

struct BeginFrameOptions {
    bool full_redraw = true;
    /// When false on a full redraw, the previous frame stays visible until paint overwrites it.
    /// Application always sets this to true on full redraws to avoid stale screen artifacts.
    bool clear_buffer = true;
    Rect dirty_region{};
};

class TerminalBackend {
  public:
    virtual ~TerminalBackend() = default;

    virtual void init() = 0;
    virtual void shutdown() = 0;

    virtual Size terminal_size() const = 0;
    virtual std::optional<Event> poll_event() = 0;
    virtual std::optional<Event> poll_event_nonblocking() { return std::nullopt; }

    virtual void begin_frame(BeginFrameOptions options = {}) = 0;
    virtual void end_frame() = 0;

    /// Drop cached terminal graphics (Kitty/Sixel) after resize or terminal reset.
    virtual void invalidate_graphics() {}

    /// Move the hardware mouse cursor without a full frame erase/repaint.
    virtual void refresh_mouse_cursor() {}

    /// Hide the hardware mouse cursor while a shell terminal owns the view.
    virtual void set_mouse_cursor_suppressed(bool suppressed) { (void)suppressed; }

    /// Show the hardware text cursor at a terminal cell (shell terminals).
    virtual void set_text_cursor(std::optional<Point> position) { (void)position; }

    virtual void draw_text(int x, int y, std::string_view text, Style style) = 0;
    virtual void draw_image(int x, int y, Size cell_size, const TerminalImage& image);

    /// True when the backend will emit 24-bit RGB (SGR 38;2 / 48;2).
    virtual bool true_color() const { return false; }

    /// True while a mouse button is held (used for smoother drag polling).
    virtual bool pointer_active() const { return false; }

    /// Set getch timeout in ms (-1 = blocking, 0 = non-blocking).
    virtual void set_poll_timeout_ms(int timeout_ms) { (void)timeout_ms; }

    /// Use the terminal alternate screen buffer (xterm 1049).
    virtual void set_alternate_screen(bool enabled) { alternate_screen_ = enabled; }
    bool alternate_screen() const { return alternate_screen_; }

    /// Erase the last frame on shutdown (ignored when alternate screen is active).
    virtual void set_clear_on_shutdown(bool enabled) { clear_on_shutdown_ = enabled; }
    bool clear_on_shutdown() const { return clear_on_shutdown_; }

    /// Platform default: ncursesw on POSIX, PDCurses on Windows.
    static std::unique_ptr<TerminalBackend> create();

    /// Alias for create(). Kept for older call sites.
    static std::unique_ptr<TerminalBackend> create_ncurses();

  protected:
    bool alternate_screen_ = true;
    bool clear_on_shutdown_ = true;
};

} // namespace tuinator
