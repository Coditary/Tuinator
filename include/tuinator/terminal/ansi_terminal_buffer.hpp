#pragma once

#include <tuinator/core/event.hpp>
#include <tuinator/core/geometry.hpp>
#include <tuinator/render/paint_context.hpp>
#include <tuinator/render/style.hpp>

#include <memory>
#include <string>
#include <vector>

namespace tuinator {

/// Terminal cell grid backed by libvterm (VT100/xterm emulation).
class AnsiTerminalBuffer {
  public:
    AnsiTerminalBuffer();
    ~AnsiTerminalBuffer();

    AnsiTerminalBuffer(const AnsiTerminalBuffer&) = delete;
    AnsiTerminalBuffer& operator=(const AnsiTerminalBuffer&) = delete;

    void resize(Size size);
    Size size() const { return size_; }

    void reset();
    void feed(std::string_view bytes);

    /// Convert a key press to bytes for the PTY (via libvterm keyboard layer).
    std::string keyboard_bytes(const KeyPress& key) const;

    void cursor_position(int& row, int& col, bool& visible) const;

    /// Read a screen cell for cursor rendering (false for wide-char continuations).
    bool cell_at(int row, int col, std::string& glyph, Style& style) const;

    void paint(PaintContext& ctx, Point origin) const;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    Size size_{80, 24};
};

} // namespace tuinator
