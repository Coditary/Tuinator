#pragma once

#include <tuinator/render/glyphs.hpp>

#include <string>
#include <vector>

namespace tuinator {

/// Unicode box-drawing characters (U+2500 block).
enum class BoxDrawing {
    Horizontal,
    Vertical,
    HorizontalHeavy,
    VerticalHeavy,
    HorizontalDash,
    VerticalDash,
    HorizontalDashQuad,
    VerticalDashQuad,
    DownRight,
    DownLeft,
    UpRight,
    UpLeft,
    HeavyDownRight,
    HeavyDownLeft,
    HeavyUpRight,
    HeavyUpLeft,
    HorizontalDouble,
    VerticalDouble,
    DoubleDownRight,
    DoubleDownLeft,
    DoubleUpRight,
    DoubleUpLeft,
    RoundedDownRight,
    RoundedDownLeft,
    RoundedUpRight,
    RoundedUpLeft,
    VerticalLightAndHeavyRight,
    VerticalLightAndHeavyLeft,
    VerticalAndRight,
    VerticalAndLeft,
    HorizontalLightAndHeavyDown,
    HorizontalLightAndHeavyUp,
    DownAndHorizontal,
    UpAndHorizontal,
    Cross,
};

struct BoxDrawingDescriptor {
    BoxDrawing kind = BoxDrawing::Horizontal;
    char32_t codepoint = 0;
    char ascii_fallback = '?';
};

const BoxDrawingDescriptor& box_drawing_descriptor(BoxDrawing kind);

std::string box_drawing_glyph(BoxDrawing kind, GlyphSet glyphs = GlyphSet::Auto);

std::vector<BoxDrawing> all_box_drawings();

/// Build border glyphs from individual box-drawing parts.
BorderGlyphs border_glyphs_from_box_drawing(BoxDrawing corner_tl, BoxDrawing corner_tr, BoxDrawing corner_bl,
                                            BoxDrawing corner_br, BoxDrawing horizontal, BoxDrawing vertical);

} // namespace tuinator
