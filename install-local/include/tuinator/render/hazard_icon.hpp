#pragma once

#include <tuinator/render/glyphs.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

enum class HazardIconCategory {
    Hazard,
};

/// 6 icons from Nerd Fonts weather set (Hazard, v3.4.0).
enum class HazardIcon {
    // Hazard
    Earthquake,
    Fire,
    Flood,
    Meteor,
    Tsunami,
    Volcano,
};

struct HazardIconDescriptor {
    HazardIcon kind = HazardIcon::Earthquake;
    HazardIconCategory category = HazardIconCategory::Hazard;
    const char* path = "";
    const char* nerd_suffix = "";
    char32_t codepoint = 0;
    char ascii_fallback = '?';
};

const HazardIconDescriptor& hazard_icon_descriptor(HazardIcon icon);
HazardIconCategory hazard_icon_category(HazardIcon icon);
const char* hazard_icon_path(HazardIcon icon);
const char* hazard_icon_nerd_suffix(HazardIcon icon);
const char* hazard_icon_category_path(HazardIconCategory category);
const char* hazard_icon_category_label(HazardIconCategory category);
std::vector<std::string_view> hazard_icon_path_segments(HazardIcon icon);
std::optional<HazardIcon> hazard_icon_from_path(std::string_view path);
std::vector<HazardIcon> hazard_icons_in_category(HazardIconCategory category);
std::vector<HazardIconCategory> all_hazard_icon_categories();
std::vector<HazardIcon> all_hazard_icons();
std::string hazard_icon_glyph(HazardIcon icon, GlyphSet glyphs = GlyphSet::Auto);

} // namespace tuinator
