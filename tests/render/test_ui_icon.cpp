#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/ui_icon.hpp>

#include <cstring>

#include "test_harness.hpp"

TUINATOR_TEST(ui_icon_descriptor_matches_enum_order) {
    TUINATOR_CHECK(std::strcmp(tuinator::ui_icon_descriptor(tuinator::UiIcon::GitBranch).nerd_name, "dev-git_branch") ==
                   0);
    TUINATOR_CHECK(std::strcmp(tuinator::ui_icon_descriptor(tuinator::UiIcon::DiffAdded).nerd_name, "cod-diff_added") ==
                   0);
    TUINATOR_CHECK_EQ(tuinator::ui_icon_set(tuinator::UiIcon::Linux), tuinator::UiIconSet::Devicon);
    TUINATOR_CHECK_EQ(tuinator::ui_icon_set(tuinator::UiIcon::Close), tuinator::UiIconSet::Codicon);
}

TUINATOR_TEST(ui_icon_uses_nerd_fonts_codepoints) {
    TUINATOR_CHECK_EQ(tuinator::ui_icon_descriptor(tuinator::UiIcon::DiffAdded).codepoint, 0xEADC);
    TUINATOR_CHECK_EQ(tuinator::ui_icon_descriptor(tuinator::UiIcon::DiffModified).codepoint, 0xEADE);
    TUINATOR_CHECK_EQ(tuinator::ui_icon_descriptor(tuinator::UiIcon::DiffRemoved).codepoint, 0xEADF);
    TUINATOR_CHECK_EQ(tuinator::ui_icon_descriptor(tuinator::UiIcon::GitBranch).codepoint, 0xE725);
    TUINATOR_CHECK_EQ(tuinator::ui_icon_descriptor(tuinator::UiIcon::Close).codepoint, 0xEA76);
    TUINATOR_CHECK_EQ(tuinator::ui_icon_descriptor(tuinator::UiIcon::Linux).codepoint, 0xE712);
}

TUINATOR_TEST(ui_icon_glyph_ascii_fallback) {
    TUINATOR_CHECK_EQ(tuinator::ui_icon_glyph(tuinator::UiIcon::DiffAdded, tuinator::GlyphSet::Ascii), "+");
    TUINATOR_CHECK_EQ(tuinator::ui_icon_glyph(tuinator::UiIcon::DiffRemoved, tuinator::GlyphSet::Ascii), "-");
    TUINATOR_CHECK_EQ(tuinator::ui_icon_glyph(tuinator::UiIcon::Close, tuinator::GlyphSet::Ascii), "x");
}

TUINATOR_TEST(ui_icon_glyph_unicode_differs_from_ascii) {
    const std::string added = tuinator::ui_icon_glyph(tuinator::UiIcon::DiffAdded, tuinator::GlyphSet::Unicode);
    TUINATOR_CHECK(added != "+");
    TUINATOR_CHECK(!added.empty());
}

TUINATOR_TEST(ui_icon_from_nerd_name_lookup) {
    const auto icon = tuinator::ui_icon_from_nerd_name("cod-diff_modified");
    TUINATOR_CHECK(icon.has_value());
    TUINATOR_CHECK_EQ(*icon, tuinator::UiIcon::DiffModified);
    TUINATOR_CHECK(!tuinator::ui_icon_from_nerd_name("cod-does_not_exist").has_value());
}

TUINATOR_TEST(ui_icon_color_returns_theme_tint) {
    const tuinator::Rgb branch = tuinator::ui_icon_color(tuinator::UiIcon::GitBranch);
    TUINATOR_CHECK_EQ(branch.r, 0xBB);
    TUINATOR_CHECK_EQ(branch.g, 0x9A);
    TUINATOR_CHECK_EQ(branch.b, 0xF7);
}
