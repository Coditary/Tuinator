#include <tuinator/render/canvas.hpp>

#include <tuinator/render/terminal_image.hpp>
#include <tuinator/render/text.hpp>

#include <algorithm>
#include <string>

namespace tuinator {

Canvas::Canvas(TerminalBackend& backend)
    : backend_(backend), size_(backend.terminal_size()) {
    clip_ = {0, 0, size_.width, size_.height};
}

Canvas::Canvas(TerminalBackend& backend, Point origin, Size size, Rect clip, BorderGlyphs glyphs)
    : backend_(backend), origin_(origin), size_(size), clip_(clip), glyphs_(std::move(glyphs)) {}

void Canvas::set_glyphs(BorderGlyphs glyphs) {
    glyphs_ = std::move(glyphs);
}

Size Canvas::size() const {
    return size_;
}

Rect Canvas::bounds() const {
    return {{0, 0}, size_};
}

Point Canvas::to_terminal(Point local) const {
    return {origin_.x + local.x, origin_.y + local.y};
}

bool Canvas::is_visible(Point local) const {
    return clip_.contains(to_terminal(local));
}

void Canvas::draw_char(Point position, char ch, Style style) {
    draw_text(position, std::string(1, ch), style);
}

void Canvas::draw_text(Point position, std::string_view text, Style style) {
    if (text.empty()) {
        return;
    }

    int y = position.y;
    const int x = position.x;
    std::size_t start = 0;

    while (start <= text.size()) {
        std::size_t end = text.find('\n', start);
        const bool has_newline = end != std::string_view::npos;
        if (!has_newline) {
            end = text.size();
        }

        const std::string_view line(text.data() + start, end - start);
        if (!line.empty()) {
            const Point terminal = to_terminal({x, y});
            if (terminal.y >= clip_.y && terminal.y < clip_.bottom()) {
                int draw_x = terminal.x;
                std::string_view visible = line;

                if (draw_x < clip_.x) {
                    const int hidden_columns = clip_.x - draw_x;
                    const std::size_t skip_bytes = text_byte_length_for_width(line, hidden_columns);
                    if (skip_bytes < line.size()) {
                        visible = line.substr(skip_bytes);
                        draw_x = clip_.x;
                    } else {
                        visible = {};
                    }
                }

                if (!visible.empty() && draw_x < clip_.right()) {
                    const int available = clip_.right() - draw_x;
                    if (available > 0) {
                        const std::size_t length = text_byte_length_for_width(visible, available);
                        if (length > 0) {
                            backend_.draw_text(draw_x, terminal.y, visible.substr(0, length), style);
                        }
                    }
                }
            }
        }

        if (!has_newline) {
            break;
        }

        ++y;
        start = end + 1;
    }
}

void Canvas::draw_box(Rect rect, Style style) {
    draw_box(rect, style, glyphs_);
}

void Canvas::draw_box(Rect rect, Style style, const BorderGlyphs& glyphs) {
    if (rect.width < 2 || rect.height < 2) {
        return;
    }

    const BorderGlyphs& g = glyphs;

    draw_text({rect.x, rect.y}, g.top_left, style);
    draw_text({rect.right() - 1, rect.y}, g.top_right, style);
    draw_text({rect.x, rect.bottom() - 1}, g.bottom_left, style);
    draw_text({rect.right() - 1, rect.bottom() - 1}, g.bottom_right, style);

    for (int x = rect.x + 1; x < rect.right() - 1; ++x) {
        draw_text({x, rect.y}, g.horizontal, style);
        draw_text({x, rect.bottom() - 1}, g.horizontal, style);
    }

    for (int y = rect.y + 1; y < rect.bottom() - 1; ++y) {
        draw_text({rect.x, y}, g.vertical, style);
        draw_text({rect.right() - 1, y}, g.vertical, style);
    }
}

void Canvas::draw_hline(int x, int y, int length, Style style) {
    if (length <= 0) {
        return;
    }

    for (int i = 0; i < length; ++i) {
        draw_text({x + i, y}, glyphs_.horizontal, style);
    }
}

void Canvas::draw_vline(int x, int y, int length, Style style) {
    if (length <= 0) {
        return;
    }

    for (int i = 0; i < length; ++i) {
        draw_text({x, y + i}, glyphs_.vertical, style);
    }
}

void Canvas::fill_rect(Rect rect, char ch, Style style) {
    if (rect.width <= 0 || rect.height <= 0) {
        return;
    }

    const std::string row(static_cast<std::size_t>(rect.width), ch);
    for (int y = rect.y; y < rect.bottom(); ++y) {
        draw_text({rect.x, y}, row, style);
    }
}

void Canvas::draw_image(Point position, Size cell_size, const TerminalImage& image) {
    if (image.empty() || cell_size.width <= 0 || cell_size.height <= 0) {
        return;
    }

    const Point terminal = to_terminal(position);
    const Rect visible = intersect(clip_, Rect{terminal, cell_size});
    if (visible.width <= 0 || visible.height <= 0) {
        return;
    }

    backend_.draw_image(visible.x, visible.y, visible.size(), image);
}

void Canvas::with_clip(Rect rect, const std::function<void(Canvas&)>& draw) const {
    const Point child_origin{origin_.x + rect.x, origin_.y + rect.y};
    const Rect child_clip =
        intersect(clip_, {child_origin.x, child_origin.y, rect.width, rect.height});
    if (child_clip.width <= 0 || child_clip.height <= 0) {
        return;
    }

    Canvas child(backend_, child_origin, rect.size(), child_clip, glyphs_);
    draw(child);
}

Canvas Canvas::scrolled(int dx, int dy) const {
    return Canvas(backend_, {origin_.x + dx, origin_.y + dy}, size_, clip_, glyphs_);
}

} // namespace tuinator
