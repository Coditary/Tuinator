#pragma once

#include <tuinator/backend/terminal_backend.hpp>
#include <tuinator/core/geometry.hpp>
#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/render/terminal_image.hpp>

#include <functional>
#include <optional>
#include <string_view>

namespace tuinator {

class Canvas {
  public:
    explicit Canvas(TerminalBackend& backend);

    Size size() const;
    Rect bounds() const;

    void set_glyphs(BorderGlyphs glyphs);
    const BorderGlyphs& glyphs() const { return glyphs_; }

    void draw_text(Point position, std::string_view text, Style style = {});

    void draw_char(Point position, char ch, Style style = {});
    void draw_box(Rect rect, Style style = {});
    void draw_box(Rect rect, Style style, const BorderGlyphs& glyphs);
    void draw_hline(int x, int y, int length, Style style = {});
    void draw_vline(int x, int y, int length, Style style = {});
    void fill_rect(Rect rect, char ch, Style style = {});
    void draw_image(Point position, Size cell_size, const TerminalImage& image);

    void set_text_cursor(std::optional<Point> local_position);

    void with_clip(Rect rect, const std::function<void(Canvas&)>& draw) const;

    Point origin() const { return origin_; }
    Canvas scrolled(int dx, int dy) const;

  private:
    Canvas(TerminalBackend& backend, Point origin, Size size, Rect clip, BorderGlyphs glyphs);

    Point to_terminal(Point local) const;
    bool is_visible(Point local) const;

    TerminalBackend& backend_;
    Point origin_{0, 0};
    Size size_;
    Rect clip_{};
    BorderGlyphs glyphs_ = ascii_border_glyphs();
};

} // namespace tuinator
