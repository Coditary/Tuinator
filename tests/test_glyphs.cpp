#include "render_helper.hpp"
#include "test_harness.hpp"

#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/containers/panel.hpp>

TUINATOR_TEST(diff_fill_glyph_matches_neovim_fillchar) {
    TUINATOR_CHECK_EQ(tuinator::diff_fill_glyph(tuinator::GlyphSet::Ascii), "/");
    TUINATOR_CHECK_EQ(tuinator::diff_fill_glyph(tuinator::GlyphSet::Unicode), "\xe2\x95\xb1");
}

TUINATOR_TEST(diff_sign_glyph_matches_gitsigns_bar) {
    TUINATOR_CHECK_EQ(tuinator::diff_sign_glyph(tuinator::GlyphSet::Ascii), "|");
    TUINATOR_CHECK_EQ(tuinator::diff_sign_glyph(tuinator::GlyphSet::Unicode), "\xe2\x96\x8e");
}

TUINATOR_TEST(border_glyphs_for_each_set) {
    TUINATOR_CHECK_EQ(tuinator::border_glyphs_for(tuinator::GlyphSet::Ascii).horizontal, "-");
    TUINATOR_CHECK_EQ(tuinator::border_glyphs_for(tuinator::GlyphSet::Ascii).vertical, "|");
    TUINATOR_CHECK(tuinator::border_glyphs_for(tuinator::GlyphSet::Unicode).horizontal != "-");
    TUINATOR_CHECK(tuinator::border_glyphs_for(tuinator::GlyphSet::UnicodeRounded).top_left != "+");
}

TUINATOR_TEST(theme_carries_glyph_set) {
    const tuinator::Theme ascii = tuinator::dark_theme({.glyphs = tuinator::GlyphSet::Ascii});
    const tuinator::Theme unicode = tuinator::dark_theme({.glyphs = tuinator::GlyphSet::Unicode});
    TUINATOR_CHECK_EQ(ascii.glyphs.top_left, "+");
    TUINATOR_CHECK(unicode.glyphs.top_left != "+");
}

TUINATOR_TEST(canvas_draw_box_ascii_corners) {
    tuinator::MemoryTerminalBackend backend({12, 6});
    backend.init();

    tuinator::Panel panel("Box", tuinator::Theme{}.border, tuinator::Theme{}.heading, tuinator::ascii_border_glyphs());
    panel.layout({0, 0, 12, 6});

    backend.begin_frame();
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    ctx.with_clip({0, 0, 12, 6}, [&](tuinator::PaintContext& clipped) { panel.paint(clipped); });
    backend.end_frame();

    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 0, 0), '+');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 11, 0), '+');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 0, 5), '+');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 5, 5), '-');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 0, 3), '|');
}
