#pragma once

#include <tuinator/render/glyphs.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

enum class AstronomyIconCategory {
    Celestial,
    Eclipse,
    Sky,
    Misc,
};

/// 11 icons from Nerd Fonts weather set (Astronomy, v3.4.0).
enum class AstronomyIcon {
    // Celestial
    Moonrise,
    Moonset,
    Sunrise,
    Sunset,
    // Eclipse
    LunarEclipse,
    SolarEclipse,
    // Sky
    Horizon,
    HorizonAlt,
    Stars,
    // Misc
    Alien,
    Aliens,
};

struct AstronomyIconDescriptor {
    AstronomyIcon kind = AstronomyIcon::Moonrise;
    AstronomyIconCategory category = AstronomyIconCategory::Celestial;
    const char* path = "";
    const char* nerd_suffix = "";
    char32_t codepoint = 0;
    char ascii_fallback = '?';
};

const AstronomyIconDescriptor& astronomy_icon_descriptor(AstronomyIcon icon);
AstronomyIconCategory astronomy_icon_category(AstronomyIcon icon);
const char* astronomy_icon_path(AstronomyIcon icon);
const char* astronomy_icon_nerd_suffix(AstronomyIcon icon);
const char* astronomy_icon_category_path(AstronomyIconCategory category);
const char* astronomy_icon_category_label(AstronomyIconCategory category);
std::vector<std::string_view> astronomy_icon_path_segments(AstronomyIcon icon);
std::optional<AstronomyIcon> astronomy_icon_from_path(std::string_view path);
std::vector<AstronomyIcon> astronomy_icons_in_category(AstronomyIconCategory category);
std::vector<AstronomyIconCategory> all_astronomy_icon_categories();
std::vector<AstronomyIcon> all_astronomy_icons();
std::string astronomy_icon_glyph(AstronomyIcon icon, GlyphSet glyphs = GlyphSet::Auto);

} // namespace tuinator
