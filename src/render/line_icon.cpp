#include <tuinator/render/line_icon.hpp>

#include <tuinator/render/glyphs.hpp>

#include <array>
#include <cstring>
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

constexpr LineIconDescriptor make(
    LineIcon kind,
    LineIconCategory category,
    LineIconSet set,
    const char* nerd_name,
    char32_t codepoint,
    char ascii) {
    return LineIconDescriptor{
        kind,
        category,
        set,
        nerd_name,
        codepoint,
        ascii,
    };
}

// data/nerd_line_icons.tsv (v3.4.0 PLE + Unicode box/block/diff).
constexpr std::array<LineIconDescriptor, 76> kDescriptors{{
    make(LineIcon::Branch, LineIconCategory::Powerline, LineIconSet::Powerline, "ple-branch", 0xE0A0, 'Y'),
    make(LineIcon::LineNumber, LineIconCategory::Powerline, LineIconSet::Powerline, "ple-line_number", 0xE0A1, '#'),
    make(LineIcon::Hostname, LineIconCategory::Powerline, LineIconSet::Powerline, "ple-hostname", 0xE0A2, 'H'),
    make(LineIcon::ColumnNumber, LineIconCategory::Powerline, LineIconSet::Powerline, "ple-column_number", 0xE0A3, '#'),
    make(LineIcon::LeftHardDivider, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-left_hard_divider", 0xE0B0, '>'),
    make(LineIcon::LeftSoftDivider, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-left_soft_divider", 0xE0B1, '>'),
    make(LineIcon::RightHardDivider, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-right_hard_divider", 0xE0B2, '<'),
    make(LineIcon::RightSoftDivider, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-right_soft_divider", 0xE0B3, '<'),
    make(LineIcon::RightHalfCircleThick, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-right_half_circle_thick", 0xE0B4, ')'),
    make(LineIcon::RightHalfCircleThin, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-right_half_circle_thin", 0xE0B5, ')'),
    make(LineIcon::LeftHalfCircleThick, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-left_half_circle_thick", 0xE0B6, '('),
    make(LineIcon::LeftHalfCircleThin, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-left_half_circle_thin", 0xE0B7, '('),
    make(LineIcon::LowerLeftTriangle, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-lower_left_triangle", 0xE0B8, '/'),
    make(LineIcon::BackslashSeparator, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-backslash_separator", 0xE0B9, '\\'),
    make(LineIcon::LowerRightTriangle, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-lower_right_triangle", 0xE0BA, '\\'),
    make(LineIcon::ForwardslashSeparator, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-forwardslash_separator", 0xE0BB, '/'),
    make(LineIcon::UpperLeftTriangle, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-upper_left_triangle", 0xE0BC, '/'),
    make(LineIcon::ForwardslashSeparatorRedundant, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-forwardslash_separator_redundant", 0xE0BD, '/'),
    make(LineIcon::UpperRightTriangle, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-upper_right_triangle", 0xE0BE, '\\'),
    make(LineIcon::BackslashSeparatorRedundant, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-backslash_separator_redundant", 0xE0BF, '\\'),
    make(LineIcon::FlameThick, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-flame_thick", 0xE0C0, '~'),
    make(LineIcon::FlameThin, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-flame_thin", 0xE0C1, '~'),
    make(LineIcon::FlameThickMirrored, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-flame_thick_mirrored", 0xE0C2, '~'),
    make(LineIcon::FlameThinMirrored, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-flame_thin_mirrored", 0xE0C3, '~'),
    make(LineIcon::PixelatedSquaresSmall, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-pixelated_squares_small", 0xE0C4, '#'),
    make(LineIcon::PixelatedSquaresSmallMirrored, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-pixelated_squares_small_mirrored", 0xE0C5, '#'),
    make(LineIcon::PixelatedSquaresBig, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-pixelated_squares_big", 0xE0C6, '#'),
    make(LineIcon::PixelatedSquaresBigMirrored, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-pixelated_squares_big_mirrored", 0xE0C7, '#'),
    make(LineIcon::IceWaveform, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-ice_waveform", 0xE0C8, '~'),
    make(LineIcon::IceWaveformMirrored, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-ice_waveform_mirrored", 0xE0CA, '~'),
    make(LineIcon::Honeycomb, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-honeycomb", 0xE0CC, '#'),
    make(LineIcon::HoneycombOutline, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-honeycomb_outline", 0xE0CD, '#'),
    make(LineIcon::LegoSeparator, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-lego_separator", 0xE0CE, '-'),
    make(LineIcon::LegoSeparatorThin, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-lego_separator_thin", 0xE0CF, '-'),
    make(LineIcon::LegoBlockFacing, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-lego_block_facing", 0xE0D0, '#'),
    make(LineIcon::LegoBlockSideways, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-lego_block_sideways", 0xE0D1, '#'),
    make(LineIcon::TrapezoidTopBottom, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-trapezoid_top_bottom", 0xE0D2, '/'),
    make(LineIcon::TrapezoidTopBottomMirrored, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-trapezoid_top_bottom_mirrored", 0xE0D4, '\\'),
    make(LineIcon::RightHardDividerInverse, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-right_hard_divider_inverse", 0xE0D6, '<'),
    make(LineIcon::LeftHardDividerInverse, LineIconCategory::PowerlineExtra, LineIconSet::Powerline, "ple-left_hard_divider_inverse", 0xE0D7, '>'),
    make(LineIcon::BoxHorizontal, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-horizontal", 0x2500, '-'),
    make(LineIcon::BoxVertical, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-vertical", 0x2502, '|'),
    make(LineIcon::BoxHorizontalHeavy, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-horizontal_heavy", 0x2501, '-'),
    make(LineIcon::BoxVerticalHeavy, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-vertical_heavy", 0x2503, '|'),
    make(LineIcon::BoxHorizontalDash, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-horizontal_dash", 0x2504, '-'),
    make(LineIcon::BoxVerticalDash, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-vertical_dash", 0x2506, '|'),
    make(LineIcon::BoxHorizontalDashQuad, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-horizontal_dash_quad", 0x2508, '-'),
    make(LineIcon::BoxVerticalDashQuad, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-vertical_dash_quad", 0x250A, '|'),
    make(LineIcon::BoxDownRight, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-down_right", 0x250C, '+'),
    make(LineIcon::BoxDownLeft, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-down_left", 0x2510, '+'),
    make(LineIcon::BoxUpRight, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-up_right", 0x2514, '+'),
    make(LineIcon::BoxUpLeft, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-up_left", 0x2518, '+'),
    make(LineIcon::BoxHorizontalDouble, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-horizontal_double", 0x2550, '='),
    make(LineIcon::BoxVerticalDouble, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-vertical_double", 0x2551, '|'),
    make(LineIcon::BoxDoubleDownRight, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-double_down_right", 0x2554, '+'),
    make(LineIcon::BoxDoubleDownLeft, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-double_down_left", 0x2557, '+'),
    make(LineIcon::BoxDoubleUpRight, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-double_up_right", 0x255A, '+'),
    make(LineIcon::BoxDoubleUpLeft, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-double_up_left", 0x255D, '+'),
    make(LineIcon::BoxRoundedDownRight, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-rounded_down_right", 0x256D, '+'),
    make(LineIcon::BoxRoundedDownLeft, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-rounded_down_left", 0x256E, '+'),
    make(LineIcon::BoxRoundedUpRight, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-rounded_up_right", 0x256F, '+'),
    make(LineIcon::BoxRoundedUpLeft, LineIconCategory::BoxDrawing, LineIconSet::Unicode, "uni-rounded_up_left", 0x2570, '+'),
    make(LineIcon::BlockFull, LineIconCategory::BlockElement, LineIconSet::Unicode, "uni-full", 0x2588, '#'),
    make(LineIcon::BlockUpperHalf, LineIconCategory::BlockElement, LineIconSet::Unicode, "uni-upper_half", 0x2580, '#'),
    make(LineIcon::BlockLowerHalf, LineIconCategory::BlockElement, LineIconSet::Unicode, "uni-lower_half", 0x2584, '#'),
    make(LineIcon::BlockLeftHalf, LineIconCategory::BlockElement, LineIconSet::Unicode, "uni-left_half", 0x258C, '#'),
    make(LineIcon::BlockRightHalf, LineIconCategory::BlockElement, LineIconSet::Unicode, "uni-right_half", 0x2590, '#'),
    make(LineIcon::BlockLightShade, LineIconCategory::BlockElement, LineIconSet::Unicode, "uni-light_shade", 0x2591, '.'),
    make(LineIcon::BlockMediumShade, LineIconCategory::BlockElement, LineIconSet::Unicode, "uni-medium_shade", 0x2592, ':'),
    make(LineIcon::BlockDarkShade, LineIconCategory::BlockElement, LineIconSet::Unicode, "uni-dark_shade", 0x2593, '#'),
    make(LineIcon::BlockLeftOneEighth, LineIconCategory::BlockElement, LineIconSet::Unicode, "uni-left_one_eighth", 0x258E, '|'),
    make(LineIcon::BlockRightOneEighth, LineIconCategory::BlockElement, LineIconSet::Unicode, "uni-right_one_eighth", 0x2595, '|'),
    make(LineIcon::BlockLowerOneEighth, LineIconCategory::BlockElement, LineIconSet::Unicode, "uni-lower_one_eighth", 0x2581, '_'),
    make(LineIcon::BlockUpperOneEighth, LineIconCategory::BlockElement, LineIconSet::Unicode, "uni-upper_one_eighth", 0x258F, '^'),
    make(LineIcon::DiffFill, LineIconCategory::Diff, LineIconSet::Unicode, "uni-fill", 0x2571, '/'),
    make(LineIcon::DiffSign, LineIconCategory::Diff, LineIconSet::Unicode, "uni-sign", 0x258E, '|'),
}};

static_assert(kDescriptors.size() == 76, "descriptor table out of sync with LineIcon enum");

const LineIconDescriptor& descriptor_or_default(LineIcon icon) {
    const auto index = static_cast<std::size_t>(icon);
    if (index < kDescriptors.size()) {
        return kDescriptors[index];
    }
    return kDescriptors[static_cast<std::size_t>(LineIcon::BoxHorizontal)];
}

GlyphSet resolve_glyph_set(const LineIconDescriptor& descriptor, GlyphSet glyphs) {
    if (glyphs != GlyphSet::Auto) {
        return glyphs;
    }

    if (descriptor.set == LineIconSet::Powerline) {
        return detect_file_icon_glyph_set();
    }

    return supports_unicode_text() ? GlyphSet::Unicode : GlyphSet::Ascii;
}

} // namespace

const LineIconDescriptor& line_icon_descriptor(LineIcon icon) {
    return descriptor_or_default(icon);
}

LineIconCategory line_icon_category(LineIcon icon) {
    return line_icon_descriptor(icon).category;
}

LineIconSet line_icon_set(LineIcon icon) {
    return line_icon_descriptor(icon).set;
}

const char* line_icon_nerd_name(LineIcon icon) {
    return line_icon_descriptor(icon).nerd_name;
}

std::optional<LineIcon> line_icon_from_nerd_name(std::string_view nerd_name) {
    for (const LineIconDescriptor& descriptor : kDescriptors) {
        if (nerd_name == descriptor.nerd_name) {
            return descriptor.kind;
        }
    }
    return std::nullopt;
}

std::vector<LineIcon> line_icons_in_category(LineIconCategory category) {
    std::vector<LineIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const LineIconDescriptor& descriptor : kDescriptors) {
        if (descriptor.category == category) {
            icons.push_back(descriptor.kind);
        }
    }
    return icons;
}

std::string line_icon_glyph(LineIcon icon, GlyphSet glyphs) {
    const LineIconDescriptor& descriptor = line_icon_descriptor(icon);
    glyphs = resolve_glyph_set(descriptor, glyphs);

    if (glyphs == GlyphSet::Ascii) {
        return std::string(1, descriptor.ascii_fallback);
    }

    return utf8_from(descriptor.codepoint);
}

} // namespace tuinator
