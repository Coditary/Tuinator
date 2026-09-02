#include "test_harness.hpp"

#include <tuinator/render/block_element.hpp>
#include <tuinator/render/glyphs.hpp>

TUINATOR_TEST(block_element_descriptor_matches_enum_order) {
    TUINATOR_CHECK_EQ(tuinator::block_element_descriptor(tuinator::BlockElement::Full).codepoint, 0x2588);
    TUINATOR_CHECK_EQ(tuinator::block_element_descriptor(tuinator::BlockElement::UpperHalf).codepoint, 0x2580);
    TUINATOR_CHECK_EQ(tuinator::block_element_descriptor(tuinator::BlockElement::LowerHalf).codepoint, 0x2584);
    TUINATOR_CHECK_EQ(tuinator::block_element_descriptor(tuinator::BlockElement::LeftThreeEighths).codepoint, 0x258D);
    TUINATOR_CHECK_EQ(tuinator::block_element_descriptor(tuinator::BlockElement::RightThreeEighths).codepoint, 0x2596);
    TUINATOR_CHECK_EQ(tuinator::block_element_descriptor(tuinator::BlockElement::UpperOneQuarter).codepoint, 0x2586);
    TUINATOR_CHECK_EQ(tuinator::block_element_descriptor(tuinator::BlockElement::LowerOneQuarter).codepoint, 0x2582);
}

TUINATOR_TEST(block_element_glyph_ascii_fallback) {
    TUINATOR_CHECK_EQ(tuinator::block_element_glyph(tuinator::BlockElement::Full, tuinator::GlyphSet::Ascii), "#");
    TUINATOR_CHECK_EQ(tuinator::block_element_glyph(tuinator::BlockElement::LightShade, tuinator::GlyphSet::Ascii), ".");
    TUINATOR_CHECK_EQ(tuinator::block_element_glyph(tuinator::BlockElement::MediumShade, tuinator::GlyphSet::Ascii), ":");
    TUINATOR_CHECK_EQ(tuinator::block_element_glyph(tuinator::BlockElement::LowerOneEighth, tuinator::GlyphSet::Ascii), "_");
    TUINATOR_CHECK_EQ(tuinator::block_element_glyph(tuinator::BlockElement::UpperOneEighth, tuinator::GlyphSet::Ascii), "^");
}

TUINATOR_TEST(block_element_glyph_unicode_differs_from_ascii) {
    const std::string full = tuinator::block_element_glyph(tuinator::BlockElement::Full, tuinator::GlyphSet::Unicode);
    TUINATOR_CHECK(full != "#");
    TUINATOR_CHECK(!full.empty());
}

TUINATOR_TEST(all_block_elements_lists_every_kind) {
    const auto elements = tuinator::all_block_elements();
    TUINATOR_CHECK_EQ(elements.size(), 16U);
    TUINATOR_CHECK_EQ(elements.front(), tuinator::BlockElement::Full);
    TUINATOR_CHECK_EQ(elements.back(), tuinator::BlockElement::LowerOneQuarter);
}
