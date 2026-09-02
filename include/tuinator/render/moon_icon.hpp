#pragma once

#include <tuinator/render/glyphs.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

enum class MoonIconCategory {
    Moon,
    MoonAlt,
};

/// 56 icons from Nerd Fonts weather set (Moon, v3.4.0).
enum class MoonIcon {
    // Moon
    MoonFirstQuarter,
    MoonFull,
    MoonNew,
    MoonThirdQuarter,
    MoonWaningCrescent1,
    MoonWaningCrescent2,
    MoonWaningCrescent3,
    MoonWaningCrescent4,
    MoonWaningCrescent5,
    MoonWaningCrescent6,
    MoonWaningGibbous1,
    MoonWaningGibbous2,
    MoonWaningGibbous3,
    MoonWaningGibbous4,
    MoonWaningGibbous5,
    MoonWaningGibbous6,
    MoonWaxingCrescent1,
    MoonWaxingCrescent2,
    MoonWaxingCrescent3,
    MoonWaxingCrescent4,
    MoonWaxingCrescent5,
    MoonWaxingCrescent6,
    MoonWaxingGibbous1,
    MoonWaxingGibbous2,
    MoonWaxingGibbous3,
    MoonWaxingGibbous4,
    MoonWaxingGibbous5,
    MoonWaxingGibbous6,
    // MoonAlt
    MoonAltFirstQuarter,
    MoonAltFull,
    MoonAltNew,
    MoonAltThirdQuarter,
    MoonAltWaningCrescent1,
    MoonAltWaningCrescent2,
    MoonAltWaningCrescent3,
    MoonAltWaningCrescent4,
    MoonAltWaningCrescent5,
    MoonAltWaningCrescent6,
    MoonAltWaningGibbous1,
    MoonAltWaningGibbous2,
    MoonAltWaningGibbous3,
    MoonAltWaningGibbous4,
    MoonAltWaningGibbous5,
    MoonAltWaningGibbous6,
    MoonAltWaxingCrescent1,
    MoonAltWaxingCrescent2,
    MoonAltWaxingCrescent3,
    MoonAltWaxingCrescent4,
    MoonAltWaxingCrescent5,
    MoonAltWaxingCrescent6,
    MoonAltWaxingGibbous1,
    MoonAltWaxingGibbous2,
    MoonAltWaxingGibbous3,
    MoonAltWaxingGibbous4,
    MoonAltWaxingGibbous5,
    MoonAltWaxingGibbous6,
};

struct MoonIconDescriptor {
    MoonIcon kind = MoonIcon::MoonFirstQuarter;
    MoonIconCategory category = MoonIconCategory::Moon;
    const char* path = "";
    const char* nerd_suffix = "";
    char32_t codepoint = 0;
    char ascii_fallback = '?';
};

const MoonIconDescriptor& moon_icon_descriptor(MoonIcon icon);
MoonIconCategory moon_icon_category(MoonIcon icon);
const char* moon_icon_path(MoonIcon icon);
const char* moon_icon_nerd_suffix(MoonIcon icon);
const char* moon_icon_category_path(MoonIconCategory category);
const char* moon_icon_category_label(MoonIconCategory category);
std::vector<std::string_view> moon_icon_path_segments(MoonIcon icon);
std::optional<MoonIcon> moon_icon_from_path(std::string_view path);
std::vector<MoonIcon> moon_icons_in_category(MoonIconCategory category);
std::vector<MoonIconCategory> all_moon_icon_categories();
std::vector<MoonIcon> all_moon_icons();
std::string moon_icon_glyph(MoonIcon icon, GlyphSet glyphs = GlyphSet::Auto);

} // namespace tuinator
