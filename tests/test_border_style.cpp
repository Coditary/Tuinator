#include "test_harness.hpp"

#include <tuinator/render/border_style.hpp>
#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/theme.hpp>

namespace {

void check_corners_distinct(
    const tuinator::BorderGlyphs& a,
    const tuinator::BorderGlyphs& b) {
    TUINATOR_CHECK(a.top_left != b.top_left);
    TUINATOR_CHECK(a.top_right != b.top_right);
    TUINATOR_CHECK(a.bottom_left != b.bottom_left);
    TUINATOR_CHECK(a.bottom_right != b.bottom_right);
}

void check_edges_distinct(
    const tuinator::BorderGlyphs& a,
    const tuinator::BorderGlyphs& b) {
    TUINATOR_CHECK(a.horizontal != b.horizontal);
    TUINATOR_CHECK(a.vertical != b.vertical);
}

} // namespace

TUINATOR_TEST(border_style_each_set_has_distinct_corners) {
    const tuinator::BorderGlyphs ascii = tuinator::border_glyphs_for(tuinator::BorderStyle::Ascii);
    const tuinator::BorderGlyphs light = tuinator::border_glyphs_for(tuinator::BorderStyle::Light);
    const tuinator::BorderGlyphs heavy = tuinator::border_glyphs_for(tuinator::BorderStyle::Heavy);
    const tuinator::BorderGlyphs doubl = tuinator::border_glyphs_for(tuinator::BorderStyle::Double);
    const tuinator::BorderGlyphs rounded = tuinator::border_glyphs_for(tuinator::BorderStyle::Rounded);

    check_corners_distinct(ascii, light);
    check_corners_distinct(light, heavy);
    check_corners_distinct(heavy, doubl);
    check_corners_distinct(doubl, rounded);
    check_corners_distinct(rounded, ascii);
}

TUINATOR_TEST(border_style_each_set_has_distinct_edges) {
    const tuinator::BorderGlyphs ascii = tuinator::border_glyphs_for(tuinator::BorderStyle::Ascii);
    const tuinator::BorderGlyphs light = tuinator::border_glyphs_for(tuinator::BorderStyle::Light);
    const tuinator::BorderGlyphs heavy = tuinator::border_glyphs_for(tuinator::BorderStyle::Heavy);
    const tuinator::BorderGlyphs doubl = tuinator::border_glyphs_for(tuinator::BorderStyle::Double);
    const tuinator::BorderGlyphs rounded = tuinator::border_glyphs_for(tuinator::BorderStyle::Rounded);

    check_edges_distinct(ascii, light);
    check_edges_distinct(light, heavy);
    check_edges_distinct(heavy, doubl);
    check_edges_distinct(doubl, rounded);
    TUINATOR_CHECK(rounded.horizontal == light.horizontal);
    TUINATOR_CHECK(rounded.vertical == light.vertical);
    TUINATOR_CHECK(rounded.horizontal != ascii.horizontal);
}

TUINATOR_TEST(border_style_unicode_codepoints) {
    const tuinator::BorderGlyphs light = tuinator::border_glyphs_for(tuinator::BorderStyle::Light);
    const tuinator::BorderGlyphs heavy = tuinator::border_glyphs_for(tuinator::BorderStyle::Heavy);
    const tuinator::BorderGlyphs doubl = tuinator::border_glyphs_for(tuinator::BorderStyle::Double);
    const tuinator::BorderGlyphs rounded = tuinator::border_glyphs_for(tuinator::BorderStyle::Rounded);

    TUINATOR_CHECK_EQ(light.top_left, "\xe2\x94\x8c");     // ┌
    TUINATOR_CHECK_EQ(heavy.horizontal, "\xe2\x94\x81");    // ━
    TUINATOR_CHECK_EQ(heavy.vertical, "\xe2\x94\x83");     // ┃
    TUINATOR_CHECK_EQ(doubl.top_left, "\xe2\x95\x94");      // ╔
    TUINATOR_CHECK_EQ(rounded.top_left, "\xe2\x95\xad");    // ╭
}

TUINATOR_TEST(border_style_from_glyph_set_mapping) {
    TUINATOR_CHECK_EQ(
        tuinator::border_style_from_glyph_set(tuinator::GlyphSet::Ascii),
        tuinator::BorderStyle::Ascii);
    TUINATOR_CHECK_EQ(
        tuinator::border_style_from_glyph_set(tuinator::GlyphSet::Unicode),
        tuinator::BorderStyle::Light);
    TUINATOR_CHECK_EQ(
        tuinator::border_style_from_glyph_set(tuinator::GlyphSet::UnicodeRounded),
        tuinator::BorderStyle::Rounded);
}

TUINATOR_TEST(glyph_set_backward_compat_maps_to_border_style) {
    TUINATOR_CHECK_EQ(
        tuinator::border_glyphs_for(tuinator::GlyphSet::Unicode).top_left,
        tuinator::border_glyphs_for(tuinator::BorderStyle::Light).top_left);
    TUINATOR_CHECK_EQ(
        tuinator::border_glyphs_for(tuinator::GlyphSet::UnicodeRounded).top_left,
        tuinator::border_glyphs_for(tuinator::BorderStyle::Rounded).top_left);
}

TUINATOR_TEST(theme_options_border_style_overrides_glyph_set) {
    const tuinator::Theme themed = tuinator::dark_theme({
        .glyphs = tuinator::GlyphSet::Ascii,
        .border_style = tuinator::BorderStyle::Double,
    });

    TUINATOR_CHECK_EQ(themed.glyphs.top_left, "\xe2\x95\x94"); // ╔
}
