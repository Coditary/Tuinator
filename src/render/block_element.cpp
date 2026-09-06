#include <tuinator/render/block_element.hpp>
#include <tuinator/render/glyphs.hpp>

#include <array>
#include <string>

namespace tuinator {

namespace {

std::string utf8_from(char32_t cp) {
    std::string out;
    if (cp < 0x80) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    return out;
}

constexpr BlockElementDescriptor make(BlockElement kind, char32_t codepoint, char ascii) {
    return BlockElementDescriptor{
        kind,
        codepoint,
        ascii,
    };
}

// data/block_elements.tsv (Unicode block elements).
constexpr std::array<BlockElementDescriptor, 16> kDescriptors{{
    make(BlockElement::Full, 0x2588, '#'),
    make(BlockElement::UpperHalf, 0x2580, '#'),
    make(BlockElement::LowerHalf, 0x2584, '#'),
    make(BlockElement::LeftHalf, 0x258C, '#'),
    make(BlockElement::RightHalf, 0x2590, '#'),
    make(BlockElement::LightShade, 0x2591, '.'),
    make(BlockElement::MediumShade, 0x2592, ':'),
    make(BlockElement::DarkShade, 0x2593, '#'),
    make(BlockElement::LeftOneEighth, 0x258E, '|'),
    make(BlockElement::RightOneEighth, 0x2595, '|'),
    make(BlockElement::LowerOneEighth, 0x2581, '_'),
    make(BlockElement::UpperOneEighth, 0x258F, '^'),
    make(BlockElement::LeftThreeEighths, 0x258D, '|'),
    make(BlockElement::RightThreeEighths, 0x2596, '|'),
    make(BlockElement::UpperOneQuarter, 0x2586, '#'),
    make(BlockElement::LowerOneQuarter, 0x2582, '_'),
}};

static_assert(kDescriptors.size() == 16, "descriptor table out of sync with BlockElement enum");

const BlockElementDescriptor& descriptor_or_default(BlockElement element) {
    const auto index = static_cast<std::size_t>(element);
    if (index < kDescriptors.size()) {
        return kDescriptors[index];
    }
    return kDescriptors[static_cast<std::size_t>(BlockElement::Full)];
}

GlyphSet resolve_glyph_set(GlyphSet glyphs) {
    if (glyphs != GlyphSet::Auto) {
        return glyphs;
    }
    return supports_unicode_text() ? GlyphSet::Unicode : GlyphSet::Ascii;
}

} // namespace

const BlockElementDescriptor& block_element_descriptor(BlockElement element) { return descriptor_or_default(element); }

std::vector<BlockElement> all_block_elements() {
    std::vector<BlockElement> elements;
    elements.reserve(kDescriptors.size());
    for (const BlockElementDescriptor& descriptor : kDescriptors) {
        elements.push_back(descriptor.kind);
    }
    return elements;
}

std::string block_element_glyph(BlockElement element, GlyphSet glyphs) {
    const BlockElementDescriptor& descriptor = block_element_descriptor(element);
    glyphs = resolve_glyph_set(glyphs);

    if (glyphs == GlyphSet::Ascii) {
        return std::string(1, descriptor.ascii_fallback);
    }

    return utf8_from(descriptor.codepoint);
}

} // namespace tuinator
