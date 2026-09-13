#include <tuinator/render/box_drawing.hpp>
#include <tuinator/render/glyphs.hpp>

#include "test_harness.hpp"

TUINATOR_TEST(box_drawing_descriptor_matches_enum_order) {
    TUINATOR_CHECK_EQ(tuinator::box_drawing_descriptor(tuinator::BoxDrawing::Horizontal).codepoint, 0x2500);
    TUINATOR_CHECK_EQ(tuinator::box_drawing_descriptor(tuinator::BoxDrawing::Vertical).codepoint, 0x2502);
    TUINATOR_CHECK_EQ(tuinator::box_drawing_descriptor(tuinator::BoxDrawing::Cross).codepoint, 0x253C);
    TUINATOR_CHECK_EQ(tuinator::box_drawing_descriptor(tuinator::BoxDrawing::RoundedDownRight).codepoint, 0x256D);
}

TUINATOR_TEST(box_drawing_glyph_ascii_fallback) {
    TUINATOR_CHECK_EQ(tuinator::box_drawing_glyph(tuinator::BoxDrawing::Horizontal, tuinator::GlyphSet::Ascii), "-");
    TUINATOR_CHECK_EQ(tuinator::box_drawing_glyph(tuinator::BoxDrawing::Vertical, tuinator::GlyphSet::Ascii), "|");
    TUINATOR_CHECK_EQ(tuinator::box_drawing_glyph(tuinator::BoxDrawing::HorizontalDouble, tuinator::GlyphSet::Ascii),
                      "=");
    TUINATOR_CHECK_EQ(tuinator::box_drawing_glyph(tuinator::BoxDrawing::DownRight, tuinator::GlyphSet::Ascii), "+");
}

TUINATOR_TEST(box_drawing_glyph_unicode_differs_from_ascii) {
    const std::string horizontal =
        tuinator::box_drawing_glyph(tuinator::BoxDrawing::Horizontal, tuinator::GlyphSet::Unicode);
    TUINATOR_CHECK(horizontal != "-");
    TUINATOR_CHECK_EQ(horizontal, "\xe2\x94\x80");
}

TUINATOR_TEST(all_box_drawings_lists_every_kind) {
    const auto kinds = tuinator::all_box_drawings();
    TUINATOR_CHECK_EQ(kinds.size(), 35U);
    TUINATOR_CHECK_EQ(kinds.front(), tuinator::BoxDrawing::Horizontal);
    TUINATOR_CHECK_EQ(kinds.back(), tuinator::BoxDrawing::Cross);
}

TUINATOR_TEST(border_glyphs_from_box_drawing_builds_border) {
    const tuinator::BorderGlyphs border = tuinator::border_glyphs_from_box_drawing(
        tuinator::BoxDrawing::DownRight, tuinator::BoxDrawing::DownLeft, tuinator::BoxDrawing::UpRight,
        tuinator::BoxDrawing::UpLeft, tuinator::BoxDrawing::Horizontal, tuinator::BoxDrawing::Vertical);

    const tuinator::GlyphSet glyphs = tuinator::GlyphSet::Auto;
    TUINATOR_CHECK_EQ(border.top_left, tuinator::box_drawing_glyph(tuinator::BoxDrawing::DownRight, glyphs));
    TUINATOR_CHECK_EQ(border.top_right, tuinator::box_drawing_glyph(tuinator::BoxDrawing::DownLeft, glyphs));
    TUINATOR_CHECK_EQ(border.bottom_left, tuinator::box_drawing_glyph(tuinator::BoxDrawing::UpRight, glyphs));
    TUINATOR_CHECK_EQ(border.bottom_right, tuinator::box_drawing_glyph(tuinator::BoxDrawing::UpLeft, glyphs));
    TUINATOR_CHECK_EQ(border.horizontal, tuinator::box_drawing_glyph(tuinator::BoxDrawing::Horizontal, glyphs));
    TUINATOR_CHECK_EQ(border.vertical, tuinator::box_drawing_glyph(tuinator::BoxDrawing::Vertical, glyphs));
    TUINATOR_CHECK(!border.resize_handle.empty());
}

TUINATOR_TEST(border_glyphs_from_box_drawing_rounded_style) {
    const tuinator::BorderGlyphs border = tuinator::border_glyphs_from_box_drawing(
        tuinator::BoxDrawing::RoundedDownRight, tuinator::BoxDrawing::RoundedDownLeft,
        tuinator::BoxDrawing::RoundedUpRight, tuinator::BoxDrawing::RoundedUpLeft, tuinator::BoxDrawing::Horizontal,
        tuinator::BoxDrawing::Vertical);

    const tuinator::GlyphSet glyphs = tuinator::GlyphSet::Auto;
    TUINATOR_CHECK_EQ(border.top_left, tuinator::box_drawing_glyph(tuinator::BoxDrawing::RoundedDownRight, glyphs));
    TUINATOR_CHECK_EQ(border.bottom_right, tuinator::box_drawing_glyph(tuinator::BoxDrawing::RoundedUpLeft, glyphs));
}
