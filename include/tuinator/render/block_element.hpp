#pragma once

#include <tuinator/render/glyphs.hpp>

#include <string>
#include <vector>

namespace tuinator {

/// Unicode block elements: full/half blocks, shades, and partial fills.
enum class BlockElement {
    Full,
    UpperHalf,
    LowerHalf,
    LeftHalf,
    RightHalf,
    LightShade,
    MediumShade,
    DarkShade,
    LeftOneEighth,
    RightOneEighth,
    LowerOneEighth,
    UpperOneEighth,
    LeftThreeEighths,
    RightThreeEighths,
    UpperOneQuarter,
    LowerOneQuarter,
};

struct BlockElementDescriptor {
    BlockElement kind = BlockElement::Full;
    char32_t codepoint = 0;
    char ascii_fallback = '?';
};

const BlockElementDescriptor& block_element_descriptor(BlockElement element);

std::string block_element_glyph(BlockElement element, GlyphSet glyphs = GlyphSet::Auto);

/// All block elements in enum order.
std::vector<BlockElement> all_block_elements();

} // namespace tuinator
