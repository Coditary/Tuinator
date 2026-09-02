#include <tuinator/widgets/chrome/status_line.hpp>

#include <tuinator/render/file_icon.hpp>
#include <tuinator/render/line_icon.hpp>
#include <tuinator/render/ui_icon.hpp>
#include <tuinator/render/text.hpp>

#include <algorithm>
#include <string>

namespace tuinator {

namespace {

Style apply_segment_colors(const Style& base, const StatusSegment& segment) {
    Style style = base;
    if (segment.foreground_rgb.has_value()) {
        style.foreground_rgb = segment.foreground_rgb;
    }
    if (segment.background_rgb.has_value()) {
        style.background_rgb = segment.background_rgb;
    }
    style.bold = segment.bold;
    style.dim = segment.dim;
    return style;
}

} // namespace

StatusLine::StatusLine(StatusLineStyle style) : style_(std::move(style)) {}

void StatusLine::set_left(std::vector<StatusSegment> segments) {
    left_ = std::move(segments);
    mark_dirty();
}

void StatusLine::set_center(std::vector<StatusSegment> segments) {
    center_ = std::move(segments);
    mark_dirty();
}

void StatusLine::set_right(std::vector<StatusSegment> segments) {
    right_ = std::move(segments);
    mark_dirty();
}

Size StatusLine::preferred_size() const {
    int width = 1;
    for (const StatusSegment& segment : left_) {
        width += segment_width(segment) + 1;
    }
    for (const StatusSegment& segment : right_) {
        width += segment_width(segment) + 1;
    }
    return {std::max(1, width), 1};
}

int StatusLine::segment_width(const StatusSegment& segment) const {
    if (segment.kind == StatusSegmentKind::Separator) {
        if (segment.line_icon.has_value()) {
            return text_display_width(line_icon_glyph(*segment.line_icon));
        }
        return 1;
    }

    int width = text_display_width(segment.text);
    if (segment.line_icon.has_value()) {
        width += text_display_width(line_icon_glyph(*segment.line_icon));
        if (!segment.text.empty()) {
            width += 1;
        }
    } else if (segment.ui_icon.has_value()) {
        width += text_display_width(ui_icon_glyph(*segment.ui_icon));
        if (!segment.text.empty()) {
            width += 1;
        }
    } else if (segment.icon.has_value()) {
        width += text_display_width(file_icon_glyph(*segment.icon));
        if (!segment.text.empty()) {
            width += 1;
        }
    }
    return width;
}

Style StatusLine::segment_style(const StatusSegment& segment) const {
    Style style = apply_segment_colors(style_.background, segment);
    if ((segment.kind == StatusSegmentKind::Pill || segment.kind == StatusSegmentKind::Box)
        && segment.background_rgb.has_value()) {
        style.background_rgb = segment.background_rgb;
    }
    return style;
}

void StatusLine::paint_segment(Canvas& canvas, int x, int y, const StatusSegment& segment) const {
    if (segment.kind == StatusSegmentKind::Separator) {
        if (segment.line_icon.has_value()) {
            const Rgb color = segment.foreground_rgb.value_or(Rgb{0x7d, 0xcf, 0xff});
            canvas.draw_text({x, y}, line_icon_glyph(*segment.line_icon), style_fg(color));
            return;
        }
        const Rgb color = segment.foreground_rgb.value_or(Rgb{0x7d, 0xcf, 0xff});
        canvas.fill_rect({{x, y}, {1, 1}}, ' ', style_fg_bg(color, color));
        return;
    }

    Style style = segment_style(segment);
    int cursor = x;

    if ((segment.kind == StatusSegmentKind::Pill || segment.kind == StatusSegmentKind::Box)
        && segment.background_rgb.has_value()) {
        const int width = segment_width(segment);
        canvas.fill_rect({{cursor, y}, {width, 1}}, ' ', style);
    }

    if (segment.line_icon.has_value()) {
        const std::string line_glyph = line_icon_glyph(*segment.line_icon);
        canvas.draw_text({cursor, y}, line_glyph, style);
        cursor += text_display_width(line_glyph);
        if (!segment.text.empty()) {
            ++cursor;
        }
    } else if (segment.ui_icon.has_value()) {
        Style icon_style = style;
        icon_style.foreground_rgb = segment.foreground_rgb.value_or(ui_icon_color(*segment.ui_icon));
        const std::string icon_glyph = ui_icon_glyph(*segment.ui_icon);
        canvas.draw_text({cursor, y}, icon_glyph, icon_style);
        cursor += text_display_width(icon_glyph);
        if (!segment.text.empty()) {
            ++cursor;
        }
    } else if (segment.icon.has_value()) {
        Style icon_style = style;
        icon_style.foreground_rgb = file_icon_color(*segment.icon);
        const std::string icon_glyph = file_icon_glyph(*segment.icon);
        canvas.draw_text({cursor, y}, icon_glyph, icon_style);
        cursor += text_display_width(icon_glyph);
        if (!segment.text.empty()) {
            ++cursor;
        }
    }

    if (!segment.text.empty()) {
        canvas.draw_text({cursor, y}, segment.text, style);
    }
}

int StatusLine::paint_segments(Canvas& canvas, int x, int y, const std::vector<StatusSegment>& segments) const {
    int cursor = x;
    for (const StatusSegment& segment : segments) {
        paint_segment(canvas, cursor, y, segment);
        cursor += segment_width(segment) + 1;
    }
    return cursor;
}

void StatusLine::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    const int width = bounds_.width;
    if (width <= 0 || bounds_.height <= 0) {
        return;
    }

    canvas.fill_rect({{0, 0}, {width, bounds_.height}}, ' ', style_.background);

    int x = 0;
    const int left_end = paint_segments(canvas, x, 0, left_);

    int center_width = 0;
    for (const StatusSegment& segment : center_) {
        center_width += segment_width(segment) + 1;
    }
    if (!center_.empty() && center_width > 0) {
        const int center_x = std::max(left_end, (width - center_width) / 2);
        paint_segments(canvas, center_x, 0, center_);
    }

    int right_width = 0;
    for (const StatusSegment& segment : right_) {
        right_width += segment_width(segment) + 1;
    }
    if (!right_.empty() && right_width > 0) {
        const int right_x = std::max(left_end + 1, width - right_width);
        paint_segments(canvas, right_x, 0, right_);
    }
}

} // namespace tuinator
