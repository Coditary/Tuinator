#pragma once

#include <tuinator/render/canvas.hpp>
#include <tuinator/render/scrollbar/behavior.hpp>
#include <tuinator/render/scrollbar/core.hpp>
#include <tuinator/render/scrollbar/style.hpp>

namespace tuinator {

struct Theme;

enum class ScrollbarPreset {
    Classic,
    Ascii,
    Minimal,
    Bold,
    Thin,
};

struct ScrollbarOptions {
    ScrollbarConfig config{};
    ScrollbarBehavior behavior = ScrollbarBehavior::classic();
    ScrollbarStyle style = ScrollbarStyle::classic();

    static ScrollbarOptions classic();
    static ScrollbarOptions from_parts(ScrollbarConfig config, ScrollbarBehavior behavior, ScrollbarStyle style);

    ScrollbarOptions with_config(ScrollbarConfig config) const;
    ScrollbarOptions with_behavior(ScrollbarBehavior behavior) const;
    ScrollbarOptions with_style(ScrollbarStyle style) const;
    ScrollbarOptions with_vertical(bool enabled) const;
    ScrollbarOptions with_horizontal(bool enabled) const;
    ScrollbarOptions with_thumb_style(Style style) const;
    ScrollbarOptions with_track_style(Style style) const;
    ScrollbarOptions with_arrow_style(Style style) const;
};

ScrollbarOptions scrollbar_options(ScrollbarPreset preset);
ScrollbarOptions scrollbar_options(const Theme& theme, ScrollbarPreset preset);

void paint_scrollbars(Canvas& canvas, const ScrollbarOptions& options, const ScrollbarLayout& layout);

void paint_scrollbars(Canvas& canvas, const ScrollbarOptions& options, int scroll_x, int scroll_y, int content_width,
                      int content_height);

// Legacy aliases
using ScrollbarStylePalette = ScrollbarPalette;
using ScrollbarSkin = ScrollbarStyle;
namespace ScrollbarSkins {
using ScrollbarStyles::ascii;
using ScrollbarStyles::bold;
using ScrollbarStyles::classic;
using ScrollbarStyles::from_kind;
using ScrollbarStyles::minimal;
inline ScrollbarStyle from_preset(ScrollbarStyleKind kind) { return ScrollbarStyles::from_kind(kind); }
using ScrollbarStyles::themed;
using ScrollbarStyles::thin;
} // namespace ScrollbarSkins

} // namespace tuinator
