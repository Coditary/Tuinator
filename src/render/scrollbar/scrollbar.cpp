#include <tuinator/render/scrollbar/scrollbar.hpp>
#include <tuinator/render/theme.hpp>

#include <algorithm>

namespace tuinator {

namespace {

void draw_glyph(Canvas& canvas, Point position, const std::string& glyph, Style style) {
    if (glyph.empty()) {
        return;
    }

    if (glyph.size() == 1) {
        canvas.draw_char(position, glyph[0], style);
        return;
    }

    canvas.draw_text(position, glyph, style);
}

void paint_vertical_scrollbar(Canvas& canvas, int x, int y, int height, const ScrollbarThumb& thumb,
                              const ScrollbarGlyphs& glyphs, const ScrollbarPalette& palette, bool show_arrows) {
    if (height <= 0) {
        return;
    }

    if (!show_arrows) {
        for (int row = 0; row < height; ++row) {
            const bool is_thumb = row >= thumb.start && row < thumb.start + thumb.size;
            draw_glyph(canvas, {x, y + row}, is_thumb ? glyphs.thumb : glyphs.vertical_track,
                       is_thumb ? palette.thumb : palette.track);
        }
        return;
    }

    if (height == 1) {
        draw_glyph(canvas, {x, y}, glyphs.vertical_track, palette.track);
        return;
    }

    draw_glyph(canvas, {x, y}, glyphs.arrow_up, palette.arrow);
    draw_glyph(canvas, {x, y + height - 1}, glyphs.arrow_down, palette.arrow);

    const int track_len = std::max(0, height - 2);
    for (int row = 0; row < track_len; ++row) {
        const bool is_thumb = row >= thumb.start && row < thumb.start + thumb.size;
        draw_glyph(canvas, {x, y + 1 + row}, is_thumb ? glyphs.thumb : glyphs.vertical_track,
                   is_thumb ? palette.thumb : palette.track);
    }
}

void paint_horizontal_scrollbar(Canvas& canvas, int x, int y, int width, const ScrollbarThumb& thumb,
                                const ScrollbarGlyphs& glyphs, const ScrollbarPalette& palette, bool show_arrows) {
    if (width <= 0) {
        return;
    }

    if (!show_arrows) {
        for (int col = 0; col < width; ++col) {
            const bool is_thumb = col >= thumb.start && col < thumb.start + thumb.size;
            draw_glyph(canvas, {x + col, y}, is_thumb ? glyphs.thumb : glyphs.horizontal_track,
                       is_thumb ? palette.thumb : palette.track);
        }
        return;
    }

    if (width == 1) {
        draw_glyph(canvas, {x, y}, glyphs.horizontal_track, palette.track);
        return;
    }

    draw_glyph(canvas, {x, y}, glyphs.arrow_left, palette.arrow);
    draw_glyph(canvas, {x + width - 1, y}, glyphs.arrow_right, palette.arrow);

    const int track_len = std::max(0, width - 2);
    for (int col = 0; col < track_len; ++col) {
        const bool is_thumb = col >= thumb.start && col < thumb.start + thumb.size;
        draw_glyph(canvas, {x + 1 + col, y}, is_thumb ? glyphs.thumb : glyphs.horizontal_track,
                   is_thumb ? palette.thumb : palette.track);
    }
}

ScrollbarBehavior behavior_for_preset(ScrollbarPreset preset) {
    switch (preset) {
    case ScrollbarPreset::Minimal: return ScrollbarBehavior::minimal();
    default: return ScrollbarBehavior::classic();
    }
}

ScrollbarStyle style_for_preset(ScrollbarPreset preset) {
    switch (preset) {
    case ScrollbarPreset::Ascii: return ScrollbarStyle::ascii();
    case ScrollbarPreset::Minimal: return ScrollbarStyle::minimal();
    case ScrollbarPreset::Bold: return ScrollbarStyle::bold();
    case ScrollbarPreset::Thin: return ScrollbarStyle::thin();
    case ScrollbarPreset::Classic:
    default: return ScrollbarStyle::classic();
    }
}

} // namespace

ScrollbarOptions ScrollbarOptions::classic() { return {}; }

ScrollbarOptions ScrollbarOptions::from_parts(ScrollbarConfig config, ScrollbarBehavior behavior,
                                              ScrollbarStyle style) {
    ScrollbarOptions options;
    options.config = std::move(config);
    options.behavior = std::move(behavior);
    options.style = std::move(style);
    return options;
}

ScrollbarOptions ScrollbarOptions::with_config(ScrollbarConfig config) const {
    ScrollbarOptions copy = *this;
    copy.config = std::move(config);
    return copy;
}

ScrollbarOptions ScrollbarOptions::with_behavior(ScrollbarBehavior behavior) const {
    ScrollbarOptions copy = *this;
    copy.behavior = std::move(behavior);
    return copy;
}

ScrollbarOptions ScrollbarOptions::with_style(ScrollbarStyle style) const {
    ScrollbarOptions copy = *this;
    copy.style = std::move(style);
    return copy;
}

ScrollbarOptions ScrollbarOptions::with_vertical(bool enabled) const {
    ScrollbarOptions copy = *this;
    copy.config.vertical = enabled;
    return copy;
}

ScrollbarOptions ScrollbarOptions::with_horizontal(bool enabled) const {
    ScrollbarOptions copy = *this;
    copy.config.horizontal = enabled;
    return copy;
}

ScrollbarOptions ScrollbarOptions::with_thumb_style(Style style) const {
    return with_style(this->style.with_thumb_style(std::move(style)));
}

ScrollbarOptions ScrollbarOptions::with_track_style(Style style) const {
    return with_style(this->style.with_track_style(std::move(style)));
}

ScrollbarOptions ScrollbarOptions::with_arrow_style(Style style) const {
    return with_style(this->style.with_arrow_style(std::move(style)));
}

ScrollbarOptions scrollbar_options(ScrollbarPreset preset) {
    return ScrollbarOptions::from_parts({}, behavior_for_preset(preset), style_for_preset(preset));
}

ScrollbarOptions scrollbar_options(const Theme& theme, ScrollbarPreset preset) {
    return scrollbar_options(preset).with_style(ScrollbarStyles::themed(theme, style_for_preset(preset)));
}

void paint_scrollbars(Canvas& canvas, const ScrollbarOptions& options, const ScrollbarLayout& layout) {
    if (!options.config.enabled) {
        return;
    }

    const int width = canvas.size().width;
    const int height = canvas.size().height;
    if (width <= 0 || height <= 0) {
        return;
    }

    const ScrollbarGlyphs glyphs = options.style.glyphs();
    const ScrollbarPalette palette = options.style.palette();
    const bool show_arrows = options.behavior.show_arrows;

    const int vertical_x = width - 1;
    const int horizontal_y = height - 1;

    if (layout.metrics.show_vertical) {
        paint_vertical_scrollbar(canvas, vertical_x, layout.vertical_bar_y, layout.vertical_bar_height,
                                 layout.vertical_thumb, glyphs, palette, show_arrows);
    }

    if (layout.metrics.show_horizontal) {
        paint_horizontal_scrollbar(canvas, layout.horizontal_bar_x, horizontal_y, layout.horizontal_bar_width,
                                   layout.horizontal_thumb, glyphs, palette, show_arrows);
    }

    if (layout.metrics.show_vertical && layout.metrics.show_horizontal) {
        draw_glyph(canvas, {vertical_x, horizontal_y}, glyphs.corner, palette.track);
    }
}

void paint_scrollbars(Canvas& canvas, const ScrollbarOptions& options, int scroll_x, int scroll_y, int content_width,
                      int content_height) {
    const ScrollbarLayout layout =
        compute_scrollbar_layout(canvas.size().width, canvas.size().height, content_width, content_height, scroll_x,
                                 scroll_y, options.config, options.behavior.show_arrows);

    paint_scrollbars(canvas, options, layout);
}

} // namespace tuinator
