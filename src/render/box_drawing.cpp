#include <tuinator/render/box_drawing.hpp>
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

constexpr BoxDrawingDescriptor make(BoxDrawing kind, char32_t codepoint, char ascii) {
    return BoxDrawingDescriptor{
        kind,
        codepoint,
        ascii,
    };
}

// data/box_drawing.tsv
constexpr std::array<BoxDrawingDescriptor, 35> kDescriptors{{
    make(BoxDrawing::Horizontal, 0x2500, '-'),
    make(BoxDrawing::Vertical, 0x2502, '|'),
    make(BoxDrawing::HorizontalHeavy, 0x2501, '-'),
    make(BoxDrawing::VerticalHeavy, 0x2503, '|'),
    make(BoxDrawing::HorizontalDash, 0x2504, '-'),
    make(BoxDrawing::VerticalDash, 0x2506, '|'),
    make(BoxDrawing::HorizontalDashQuad, 0x2508, '-'),
    make(BoxDrawing::VerticalDashQuad, 0x250A, '|'),
    make(BoxDrawing::DownRight, 0x250C, '+'),
    make(BoxDrawing::DownLeft, 0x2510, '+'),
    make(BoxDrawing::UpRight, 0x2514, '+'),
    make(BoxDrawing::UpLeft, 0x2518, '+'),
    make(BoxDrawing::HeavyDownRight, 0x250F, '+'),
    make(BoxDrawing::HeavyDownLeft, 0x2513, '+'),
    make(BoxDrawing::HeavyUpRight, 0x2517, '+'),
    make(BoxDrawing::HeavyUpLeft, 0x251B, '+'),
    make(BoxDrawing::HorizontalDouble, 0x2550, '='),
    make(BoxDrawing::VerticalDouble, 0x2551, '|'),
    make(BoxDrawing::DoubleDownRight, 0x2554, '+'),
    make(BoxDrawing::DoubleDownLeft, 0x2557, '+'),
    make(BoxDrawing::DoubleUpRight, 0x255A, '+'),
    make(BoxDrawing::DoubleUpLeft, 0x255D, '+'),
    make(BoxDrawing::RoundedDownRight, 0x256D, '+'),
    make(BoxDrawing::RoundedDownLeft, 0x256E, '+'),
    make(BoxDrawing::RoundedUpRight, 0x256F, '+'),
    make(BoxDrawing::RoundedUpLeft, 0x2570, '+'),
    make(BoxDrawing::VerticalLightAndHeavyRight, 0x251D, '+'),
    make(BoxDrawing::VerticalLightAndHeavyLeft, 0x2525, '+'),
    make(BoxDrawing::VerticalAndRight, 0x251C, '+'),
    make(BoxDrawing::VerticalAndLeft, 0x2524, '+'),
    make(BoxDrawing::HorizontalLightAndHeavyDown, 0x252F, '+'),
    make(BoxDrawing::HorizontalLightAndHeavyUp, 0x2537, '+'),
    make(BoxDrawing::DownAndHorizontal, 0x252C, '+'),
    make(BoxDrawing::UpAndHorizontal, 0x2534, '+'),
    make(BoxDrawing::Cross, 0x253C, '+'),
}};

static_assert(kDescriptors.size() == 35, "descriptor table out of sync with BoxDrawing enum");

const BoxDrawingDescriptor& descriptor_or_default(BoxDrawing kind) {
    const auto index = static_cast<std::size_t>(kind);
    if (index < kDescriptors.size()) {
        return kDescriptors[index];
    }
    return kDescriptors[static_cast<std::size_t>(BoxDrawing::Horizontal)];
}

GlyphSet resolve_glyph_set(GlyphSet glyphs) {
    if (glyphs != GlyphSet::Auto) {
        return glyphs;
    }
    return supports_unicode_text() ? GlyphSet::Unicode : GlyphSet::Ascii;
}

std::string resize_handle_glyph(GlyphSet glyphs) {
    glyphs = resolve_glyph_set(glyphs);
    if (glyphs == GlyphSet::Ascii) {
        return "#";
    }
    return "\xe2\x97\xa2"; // ◢
}

} // namespace

const BoxDrawingDescriptor& box_drawing_descriptor(BoxDrawing kind) { return descriptor_or_default(kind); }

std::string box_drawing_glyph(BoxDrawing kind, GlyphSet glyphs) {
    const BoxDrawingDescriptor& descriptor = box_drawing_descriptor(kind);
    glyphs = resolve_glyph_set(glyphs);

    if (glyphs == GlyphSet::Ascii) {
        return std::string(1, descriptor.ascii_fallback);
    }

    return utf8_from(descriptor.codepoint);
}

std::vector<BoxDrawing> all_box_drawings() {
    std::vector<BoxDrawing> kinds;
    kinds.reserve(kDescriptors.size());
    for (const BoxDrawingDescriptor& descriptor : kDescriptors) {
        kinds.push_back(descriptor.kind);
    }
    return kinds;
}

BorderGlyphs border_glyphs_from_box_drawing(BoxDrawing corner_tl, BoxDrawing corner_tr, BoxDrawing corner_bl,
                                            BoxDrawing corner_br, BoxDrawing horizontal, BoxDrawing vertical) {
    const GlyphSet glyphs = GlyphSet::Auto;
    return BorderGlyphs{
        box_drawing_glyph(corner_tl, glyphs),  box_drawing_glyph(corner_tr, glyphs),
        box_drawing_glyph(corner_bl, glyphs),  box_drawing_glyph(corner_br, glyphs),
        box_drawing_glyph(horizontal, glyphs), box_drawing_glyph(vertical, glyphs),
        resize_handle_glyph(glyphs),
    };
}

} // namespace tuinator
