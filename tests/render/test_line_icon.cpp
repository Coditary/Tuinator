#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/line_icon.hpp>

#include <cstring>

#include "test_harness.hpp"

TUINATOR_TEST(line_icon_categories_are_grouped) {
    TUINATOR_CHECK_EQ(tuinator::line_icon_category(tuinator::LineIcon::Branch), tuinator::LineIconCategory::Powerline);
    TUINATOR_CHECK_EQ(tuinator::line_icon_category(tuinator::LineIcon::RightHardDivider),
                      tuinator::LineIconCategory::PowerlineExtra);
    TUINATOR_CHECK_EQ(tuinator::line_icon_category(tuinator::LineIcon::BoxHorizontalDash),
                      tuinator::LineIconCategory::BoxDrawing);
    TUINATOR_CHECK_EQ(tuinator::line_icon_category(tuinator::LineIcon::BlockMediumShade),
                      tuinator::LineIconCategory::BlockElement);
    TUINATOR_CHECK_EQ(tuinator::line_icon_category(tuinator::LineIcon::DiffFill), tuinator::LineIconCategory::Diff);
}

TUINATOR_TEST(line_icon_powerline_uses_nerd_fonts_codepoints) {
    TUINATOR_CHECK_EQ(tuinator::line_icon_descriptor(tuinator::LineIcon::RightHardDivider).codepoint, 0xE0B2);
    TUINATOR_CHECK_EQ(tuinator::line_icon_descriptor(tuinator::LineIcon::RightHalfCircleThin).codepoint, 0xE0B5);
    TUINATOR_CHECK(std::strcmp(tuinator::line_icon_nerd_name(tuinator::LineIcon::RightHardDivider),
                               "ple-right_hard_divider") == 0);
}

TUINATOR_TEST(line_icon_box_and_block_use_unicode_codepoints) {
    TUINATOR_CHECK_EQ(tuinator::line_icon_descriptor(tuinator::LineIcon::BoxHorizontalDashQuad).codepoint, 0x2508);
    TUINATOR_CHECK_EQ(tuinator::line_icon_descriptor(tuinator::LineIcon::BlockMediumShade).codepoint, 0x2592);
    TUINATOR_CHECK_EQ(tuinator::line_icon_descriptor(tuinator::LineIcon::DiffFill).codepoint, 0x2571);
}

TUINATOR_TEST(line_icon_glyph_ascii_fallback) {
    TUINATOR_CHECK_EQ(tuinator::line_icon_glyph(tuinator::LineIcon::RightHardDivider, tuinator::GlyphSet::Ascii), "<");
    TUINATOR_CHECK_EQ(tuinator::line_icon_glyph(tuinator::LineIcon::BlockMediumShade, tuinator::GlyphSet::Ascii), ":");
    TUINATOR_CHECK_EQ(tuinator::line_icon_glyph(tuinator::LineIcon::DiffFill, tuinator::GlyphSet::Ascii), "/");
}

TUINATOR_TEST(line_icon_from_nerd_name_lookup) {
    const auto icon = tuinator::line_icon_from_nerd_name("ple-left_half_circle_thin");
    TUINATOR_CHECK(icon.has_value());
    TUINATOR_CHECK_EQ(*icon, tuinator::LineIcon::LeftHalfCircleThin);
    TUINATOR_CHECK(!tuinator::line_icon_from_nerd_name("ple-does_not_exist").has_value());
}

TUINATOR_TEST(line_icons_in_category_lists_powerline_sets) {
    const auto powerline = tuinator::line_icons_in_category(tuinator::LineIconCategory::Powerline);
    TUINATOR_CHECK_EQ(powerline.size(), 4U);
    TUINATOR_CHECK_EQ(powerline.front(), tuinator::LineIcon::Branch);
    const auto extra = tuinator::line_icons_in_category(tuinator::LineIconCategory::PowerlineExtra);
    TUINATOR_CHECK_EQ(extra.size(), 36U);
    TUINATOR_CHECK_EQ(extra.front(), tuinator::LineIcon::LeftHardDivider);
}

TUINATOR_TEST(diff_glyph_helpers_delegate_to_line_icon) {
    TUINATOR_CHECK_EQ(tuinator::diff_fill_glyph(tuinator::GlyphSet::Ascii),
                      tuinator::line_icon_glyph(tuinator::LineIcon::DiffFill, tuinator::GlyphSet::Ascii));
    TUINATOR_CHECK_EQ(tuinator::diff_sign_glyph(tuinator::GlyphSet::Unicode),
                      tuinator::line_icon_glyph(tuinator::LineIcon::DiffSign, tuinator::GlyphSet::Unicode));
}
