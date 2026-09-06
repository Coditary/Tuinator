#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/text.hpp>
#include <tuinator/widgets/chrome/terminal_frame.hpp>

#include <algorithm>
#include <variant>

namespace tuinator {

namespace {

constexpr int kMinWidth = 16;
constexpr int kMinHeight = 6;

} // namespace

TerminalFrame::TerminalFrame(std::string title, std::unique_ptr<Widget> content, TerminalFrameOptions options)
    : title_(std::move(title)), options_(std::move(options)), status_line_(options_.style.status_line),
      surface_(std::make_unique<MemoryTerminalBackend>(Size{80, 24})) {
    set_content(std::move(content));
    surface_->init();
}

void TerminalFrame::set_title(std::string title) {
    title_ = std::move(title);
    mark_dirty();
}

void TerminalFrame::set_content(std::unique_ptr<Widget> content) {
    content_ = std::move(content);
    bind_content_callbacks();
    surface_dirty_ = true;
    mark_dirty();
}

void TerminalFrame::set_status_left(std::vector<StatusSegment> segments) {
    status_line_.set_left(std::move(segments));
    mark_dirty();
}

void TerminalFrame::set_status_center(std::vector<StatusSegment> segments) {
    status_line_.set_center(std::move(segments));
    mark_dirty();
}

void TerminalFrame::set_status_right(std::vector<StatusSegment> segments) {
    status_line_.set_right(std::move(segments));
    mark_dirty();
}

void TerminalFrame::refresh_surface(const Theme& theme) {
    surface_dirty_ = true;
    render_surface(theme);
    mark_dirty();
}

void TerminalFrame::set_on_dirty(std::function<void(Rect)> callback) {
    Widget::set_on_dirty(std::move(callback));
    bind_content_callbacks();
    status_line_.set_on_dirty(on_dirty_);
}

void TerminalFrame::bind_content_callbacks() {
    if (content_) {
        content_->set_on_dirty([this](Rect) {
            surface_dirty_ = true;
            mark_dirty();
        });
        content_->set_on_layout(on_layout_);
    }
}

BorderGlyphs TerminalFrame::border_glyphs() const { return border_glyphs_for(options_.style.border_style); }

Rect TerminalFrame::content_bounds() const {
    const int status_rows = options_.show_status_line ? 1 : 0;
    return {
        bounds_.x + 1,
        bounds_.y + 1,
        std::max(0, bounds_.width - 2),
        std::max(0, bounds_.height - 2 - status_rows),
    };
}

Rect TerminalFrame::content_area_local() const {
    const Rect area = content_bounds();
    return {
        0,
        0,
        area.width,
        area.height,
    };
}

Rect TerminalFrame::status_bounds() const {
    if (!options_.show_status_line) {
        return {};
    }

    return {
        bounds_.x + 1,
        bounds_.y + bounds_.height - 2,
        std::max(0, bounds_.width - 2),
        1,
    };
}

Size TerminalFrame::preferred_size() const {
    Size content_size{};
    if (content_) {
        content_size = content_->preferred_size();
    }

    const int status_rows = options_.show_status_line ? 1 : 0;
    const int title_width = text_display_width(title_) + 6;
    return {
        std::max(kMinWidth, std::max(content_size.width + 2, title_width)),
        std::max(kMinHeight, content_size.height + 2 + status_rows),
    };
}

void TerminalFrame::layout(Rect bounds) {
    bounds_ = bounds;
    surface_dirty_ = true;

    if (content_) {
        content_->layout(content_area_local());
    }

    if (options_.show_status_line) {
        status_line_.layout(status_bounds());
    }
}

void TerminalFrame::ensure_surface_size() const {
    const Rect area = content_bounds();
    const Size target{std::max(1, area.width), std::max(1, area.height)};
    const Size current = surface_->terminal_size();
    if (current.width != target.width || current.height != target.height) {
        surface_->resize(target);
        surface_dirty_ = true;
    }
}

void TerminalFrame::render_surface(const Theme& theme) const {
    if (!content_) {
        return;
    }

    ensure_surface_size();
    const Rect area = content_area_local();
    content_->layout(area);

    surface_->begin_frame(BeginFrameOptions{true, true});
    Canvas canvas(*surface_);
    canvas.set_glyphs(theme.glyphs);
    canvas.fill_rect(area, ' ', options_.style.content_background);

    PaintContext child_ctx{canvas, theme};
    content_->paint(child_ctx);
    surface_->end_frame();
    surface_dirty_ = false;
}

void TerminalFrame::blit_surface(Canvas& canvas, Point origin) const {
    const auto& cells = surface_->cells();
    for (std::size_t y = 0; y < cells.size(); ++y) {
        for (std::size_t x = 0; x < cells[y].size(); ++x) {
            const MemoryTerminalBackend::Cell& cell = cells[y][x];
            Style style = cell.style;
            if (cell.ch == ' ' && !style.foreground_rgb.has_value() && !style.background_rgb.has_value() &&
                style.foreground == Color::Default && style.background == Color::Default) {
                style = options_.style.content_background;
            }
            canvas.draw_char({origin.x + static_cast<int>(x), origin.y + static_cast<int>(y)}, cell.ch, style);
        }
    }
}

void TerminalFrame::paint_title_bar(Canvas& canvas) const {
    const int width = bounds_.width;
    if (width <= 0) {
        return;
    }

    const BorderGlyphs glyphs = border_glyphs();
    const Style border = options_.style.border;
    const Style title = options_.style.title;

    canvas.draw_text({0, 0}, glyphs.top_left, border);
    if (width > 1) {
        canvas.draw_text({width - 1, 0}, glyphs.top_right, border);
    }

    const std::string label = " " + title_ + " ";
    int x = 1;
    for (const char ch : label) {
        if (x >= width - 1) {
            break;
        }
        canvas.draw_char({x, 0}, ch, title);
        ++x;
    }

    for (int fill_x = x; fill_x < width - 1; ++fill_x) {
        canvas.draw_char({fill_x, 0}, ' ', options_.style.title_background);
    }

    if (options_.style.show_controls && width >= 8) {
        const std::string controls = " - □ x ";
        int control_x = width - static_cast<int>(text_display_width(controls)) - 1;
        control_x = std::max(1, control_x);
        canvas.draw_text({control_x, 0}, controls, title);
    } else {
        for (int fill_x = 1; fill_x < width - 1; ++fill_x) {
            if (fill_x < static_cast<int>(text_display_width(label)) + 1) {
                continue;
            }
            canvas.draw_text({fill_x, 0}, glyphs.horizontal, border);
        }
    }
}

void TerminalFrame::paint(PaintContext& ctx) const {
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    Canvas& canvas = ctx.canvas;
    const BorderGlyphs glyphs = border_glyphs();
    const Style border = options_.style.border;

    canvas.draw_box({{0, 0}, bounds_.size()}, border, glyphs);
    paint_title_bar(canvas);

    const Rect area = content_bounds();
    if (area.width > 0 && area.height > 0 && content_) {
        if (options_.live_content) {
            const Rect local{
                area.x - bounds_.x,
                area.y - bounds_.y,
                area.width,
                area.height,
            };
            ctx.with_clip(local, [&](PaintContext& child_ctx) { content_->paint(child_ctx); });
        } else {
            if (surface_dirty_) {
                render_surface(ctx.theme);
            }

            const Point local_origin{
                area.x - bounds_.x,
                area.y - bounds_.y,
            };
            blit_surface(canvas, local_origin);
        }
    }

    if (options_.show_status_line) {
        const Rect status = status_bounds();
        const Rect local_status{
            status.x - bounds_.x,
            status.y - bounds_.y,
            status.width,
            status.height,
        };
        ctx.with_clip(local_status, [&](PaintContext& status_ctx) { status_line_.paint(status_ctx); });
    }
}

void TerminalFrame::on_idle() {
    if (content_ && options_.live_content) {
        content_->on_idle();
    }
}

bool TerminalFrame::needs_periodic_idle() const {
    return options_.live_content && content_ != nullptr && content_->needs_periodic_idle();
}

void TerminalFrame::for_each_descendant(const std::function<void(Widget*)>& visitor) {
    visitor(this);
    if (content_) {
        content_->for_each_descendant(visitor);
    }
    if (options_.show_status_line) {
        visitor(&status_line_);
    }
}

bool TerminalFrame::handle_event(const Event& event) {
    if (!content_) {
        return false;
    }

    if (options_.live_content) {
        if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
            if (content_->pointer_active()) {
                return content_->handle_event(event);
            }

            const Rect area = content_bounds();
            if (!area.contains(mouse->position)) {
                return false;
            }

            MouseEvent local = *mouse;
            local.position = {
                mouse->position.x - area.x,
                mouse->position.y - area.y,
            };
            return content_->handle_event(Event{local});
        }

        return content_->handle_event(event);
    }

    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (content_->pointer_active()) {
            return content_->handle_event(event);
        }

        const Rect area = content_bounds();
        if (!area.contains(mouse->position)) {
            return false;
        }

        MouseEvent local = *mouse;
        local.position = {
            mouse->position.x - area.x,
            mouse->position.y - area.y,
        };
        return content_->handle_event(Event{local});
    }

    if (content_->has_focused_descendant() && content_->handle_event(event)) {
        return true;
    }

    return false;
}

Widget* TerminalFrame::hit_test(Point point) {
    if (!bounds_.contains(point)) {
        return nullptr;
    }

    if (content_) {
        const Rect area = content_bounds();
        if (area.contains(point)) {
            const Point local{point.x - area.x, point.y - area.y};
            if (Widget* hit = content_->hit_test(local)) {
                return hit;
            }
        }
    }

    return this;
}

Widget* TerminalFrame::hit_test_focusable(Point point) {
    if (!bounds_.contains(point)) {
        return nullptr;
    }

    if (content_) {
        const Rect area = content_bounds();
        if (area.contains(point)) {
            const Point local{point.x - area.x, point.y - area.y};
            return content_->hit_test_focusable(local);
        }
    }

    return nullptr;
}

bool TerminalFrame::has_focused_descendant() const { return content_ && content_->has_focused_descendant(); }

void TerminalFrame::collect_focusable(std::vector<Widget*>& out) {
    if (content_) {
        content_->collect_focusable(out);
    }
}

void TerminalFrame::for_each_child(const std::function<void(Widget*)>& visitor) {
    if (content_) {
        visitor(content_.get());
    }
    if (options_.show_status_line) {
        visitor(&status_line_);
    }
}

} // namespace tuinator
