#pragma once

#include <tuinator/render/glyphs.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

/// Glyph source for LineIcon.
enum class LineIconSet {
    Powerline,
    Unicode,
};

/// Semantic category for line / box / block / diff glyphs.
enum class LineIconCategory {
    Powerline,
    PowerlineExtra,
    BoxDrawing,
    BlockElement,
    Diff,
};

/// Typed line glyphs: Powerline (4 Nerd Fonts PLE),
/// box drawing, block shades, and diff helpers (v3.4.0 + Unicode).
enum class LineIcon {
    // Powerline
    Branch,
    LineNumber,
    Hostname,
    ColumnNumber,
    // PowerlineExtra
    LeftHardDivider,
    LeftSoftDivider,
    RightHardDivider,
    RightSoftDivider,
    RightHalfCircleThick,
    RightHalfCircleThin,
    LeftHalfCircleThick,
    LeftHalfCircleThin,
    LowerLeftTriangle,
    BackslashSeparator,
    LowerRightTriangle,
    ForwardslashSeparator,
    UpperLeftTriangle,
    ForwardslashSeparatorRedundant,
    UpperRightTriangle,
    BackslashSeparatorRedundant,
    FlameThick,
    FlameThin,
    FlameThickMirrored,
    FlameThinMirrored,
    PixelatedSquaresSmall,
    PixelatedSquaresSmallMirrored,
    PixelatedSquaresBig,
    PixelatedSquaresBigMirrored,
    IceWaveform,
    IceWaveformMirrored,
    Honeycomb,
    HoneycombOutline,
    LegoSeparator,
    LegoSeparatorThin,
    LegoBlockFacing,
    LegoBlockSideways,
    TrapezoidTopBottom,
    TrapezoidTopBottomMirrored,
    RightHardDividerInverse,
    LeftHardDividerInverse,
    // BoxDrawing
    BoxHorizontal,
    BoxVertical,
    BoxHorizontalHeavy,
    BoxVerticalHeavy,
    BoxHorizontalDash,
    BoxVerticalDash,
    BoxHorizontalDashQuad,
    BoxVerticalDashQuad,
    BoxDownRight,
    BoxDownLeft,
    BoxUpRight,
    BoxUpLeft,
    BoxHorizontalDouble,
    BoxVerticalDouble,
    BoxDoubleDownRight,
    BoxDoubleDownLeft,
    BoxDoubleUpRight,
    BoxDoubleUpLeft,
    BoxRoundedDownRight,
    BoxRoundedDownLeft,
    BoxRoundedUpRight,
    BoxRoundedUpLeft,
    // BlockElement
    BlockFull,
    BlockUpperHalf,
    BlockLowerHalf,
    BlockLeftHalf,
    BlockRightHalf,
    BlockLightShade,
    BlockMediumShade,
    BlockDarkShade,
    BlockLeftOneEighth,
    BlockRightOneEighth,
    BlockLowerOneEighth,
    BlockUpperOneEighth,
    // Diff
    DiffFill,
    DiffSign,
};

struct LineIconDescriptor {
    LineIcon kind = LineIcon::Branch;
    LineIconCategory category = LineIconCategory::Powerline;
    LineIconSet set = LineIconSet::Powerline;
    const char* nerd_name = "";
    char32_t codepoint = 0;
    char ascii_fallback = '?';
};

const LineIconDescriptor& line_icon_descriptor(LineIcon icon);

LineIconCategory line_icon_category(LineIcon icon);

LineIconSet line_icon_set(LineIcon icon);

/// Cheat-sheet name, e.g. `ple-right_half_circle_thin` or `uni-horizontal`.
const char* line_icon_nerd_name(LineIcon icon);

std::optional<LineIcon> line_icon_from_nerd_name(std::string_view nerd_name);

/// All icons in a category (enum order within the category).
std::vector<LineIcon> line_icons_in_category(LineIconCategory category);

std::string line_icon_glyph(LineIcon icon, GlyphSet glyphs = GlyphSet::Auto);

} // namespace tuinator
