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

    virtual void draw_text(int x, int y, std::string_view text, Style style) = 0;
    virtual void draw_image(int x, int y, Size cell_size, const TerminalImage& image);

    /// True when the backend will emit 24-bit RGB (SGR 38;2 / 48;2).
    virtual bool true_color() const { return false; }

    /// True while a mouse button is held (used for smoother drag polling).
    virtual bool pointer_active() const { return false; }

    /// Set getch timeout in ms (-1 = blocking, 0 = non-blocking).
    virtual void set_poll_timeout_ms(int timeout_ms) { (void)timeout_ms; }

    /// Platform default: ncursesw on POSIX, PDCurses on Windows.
    static std::unique_ptr<TerminalBackend> create();

    /// Alias for create(). Kept for older call sites.
    static std::unique_ptr<TerminalBackend> create_ncurses();
};

} // namespace tuinator
