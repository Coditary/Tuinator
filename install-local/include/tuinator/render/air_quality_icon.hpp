#pragma once

#include <tuinator/render/glyphs.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

enum class AirQualityIconCategory {
    AirQuality,
};

/// 3 icons from Nerd Fonts weather set (AirQuality, v3.4.0).
enum class AirQualityIcon {
    // AirQuality
    Dust,
    Smog,
    Smoke,
};

struct AirQualityIconDescriptor {
    AirQualityIcon kind = AirQualityIcon::Dust;
    AirQualityIconCategory category = AirQualityIconCategory::AirQuality;
    const char* path = "";
    const char* nerd_suffix = "";
    char32_t codepoint = 0;
    char ascii_fallback = '?';
};

const AirQualityIconDescriptor& air_quality_icon_descriptor(AirQualityIcon icon);
AirQualityIconCategory air_quality_icon_category(AirQualityIcon icon);
const char* air_quality_icon_path(AirQualityIcon icon);
const char* air_quality_icon_nerd_suffix(AirQualityIcon icon);
const char* air_quality_icon_category_path(AirQualityIconCategory category);
const char* air_quality_icon_category_label(AirQualityIconCategory category);
std::vector<std::string_view> air_quality_icon_path_segments(AirQualityIcon icon);
std::optional<AirQualityIcon> air_quality_icon_from_path(std::string_view path);
std::vector<AirQualityIcon> air_quality_icons_in_category(AirQualityIconCategory category);
std::vector<AirQualityIconCategory> all_air_quality_icon_categories();
std::vector<AirQualityIcon> all_air_quality_icons();
std::string air_quality_icon_glyph(AirQualityIcon icon, GlyphSet glyphs = GlyphSet::Auto);

} // namespace tuinator
