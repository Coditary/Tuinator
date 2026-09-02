#pragma once

#include <tuinator/render/glyphs.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

enum class CompassIconCategory {
    Direction,
};

/// 8 icons from Nerd Fonts weather set (Compass, v3.4.0).
enum class CompassIcon {
    // Direction
    DirectionDown,
    DirectionDownLeft,
    DirectionDownRight,
    DirectionLeft,
    DirectionRight,
    DirectionUp,
    DirectionUpLeft,
    DirectionUpRight,
};

struct CompassIconDescriptor {
    CompassIcon kind = CompassIcon::DirectionDown;
    CompassIconCategory category = CompassIconCategory::Direction;
    const char* path = "";
    const char* nerd_suffix = "";
    char32_t codepoint = 0;
    char ascii_fallback = '?';
};

const CompassIconDescriptor& compass_icon_descriptor(CompassIcon icon);
CompassIconCategory compass_icon_category(CompassIcon icon);
const char* compass_icon_path(CompassIcon icon);
const char* compass_icon_nerd_suffix(CompassIcon icon);
const char* compass_icon_category_path(CompassIconCategory category);
const char* compass_icon_category_label(CompassIconCategory category);
std::vector<std::string_view> compass_icon_path_segments(CompassIcon icon);
std::optional<CompassIcon> compass_icon_from_path(std::string_view path);
std::vector<CompassIcon> compass_icons_in_category(CompassIconCategory category);
std::vector<CompassIconCategory> all_compass_icon_categories();
std::vector<CompassIcon> all_compass_icons();
std::string compass_icon_glyph(CompassIcon icon, GlyphSet glyphs = GlyphSet::Auto);

} // namespace tuinator
