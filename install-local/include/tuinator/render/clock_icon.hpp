#pragma once

#include <tuinator/render/glyphs.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

enum class ClockIconCategory {
    Clock,
};

/// 12 icons from Nerd Fonts weather set (Clock, v3.4.0).
enum class ClockIcon {
    // Clock
    Time1,
    Time10,
    Time11,
    Time12,
    Time2,
    Time3,
    Time4,
    Time5,
    Time6,
    Time7,
    Time8,
    Time9,
};

struct ClockIconDescriptor {
    ClockIcon kind = ClockIcon::Time1;
    ClockIconCategory category = ClockIconCategory::Clock;
    const char* path = "";
    const char* nerd_suffix = "";
    char32_t codepoint = 0;
    char ascii_fallback = '?';
};

const ClockIconDescriptor& clock_icon_descriptor(ClockIcon icon);
ClockIconCategory clock_icon_category(ClockIcon icon);
const char* clock_icon_path(ClockIcon icon);
const char* clock_icon_nerd_suffix(ClockIcon icon);
const char* clock_icon_category_path(ClockIconCategory category);
const char* clock_icon_category_label(ClockIconCategory category);
std::vector<std::string_view> clock_icon_path_segments(ClockIcon icon);
std::optional<ClockIcon> clock_icon_from_path(std::string_view path);
std::vector<ClockIcon> clock_icons_in_category(ClockIconCategory category);
std::vector<ClockIconCategory> all_clock_icon_categories();
std::vector<ClockIcon> all_clock_icons();
std::string clock_icon_glyph(ClockIcon icon, GlyphSet glyphs = GlyphSet::Auto);

} // namespace tuinator
