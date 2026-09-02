#pragma once

#include <tuinator/render/glyphs.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

enum class MarineAlertIconCategory {
    MarineAlert,
};

/// 2 icons from Nerd Fonts weather set (MarineAlert, v3.4.0).
enum class MarineAlertIcon {
    // MarineAlert
    GaleWarning,
    SmallCraftAdvisory,
};

struct MarineAlertIconDescriptor {
    MarineAlertIcon kind = MarineAlertIcon::GaleWarning;
    MarineAlertIconCategory category = MarineAlertIconCategory::MarineAlert;
    const char* path = "";
    const char* nerd_suffix = "";
    char32_t codepoint = 0;
    char ascii_fallback = '?';
};

const MarineAlertIconDescriptor& marine_alert_icon_descriptor(MarineAlertIcon icon);
MarineAlertIconCategory marine_alert_icon_category(MarineAlertIcon icon);
const char* marine_alert_icon_path(MarineAlertIcon icon);
const char* marine_alert_icon_nerd_suffix(MarineAlertIcon icon);
const char* marine_alert_icon_category_path(MarineAlertIconCategory category);
const char* marine_alert_icon_category_label(MarineAlertIconCategory category);
std::vector<std::string_view> marine_alert_icon_path_segments(MarineAlertIcon icon);
std::optional<MarineAlertIcon> marine_alert_icon_from_path(std::string_view path);
std::vector<MarineAlertIcon> marine_alert_icons_in_category(MarineAlertIconCategory category);
std::vector<MarineAlertIconCategory> all_marine_alert_icon_categories();
std::vector<MarineAlertIcon> all_marine_alert_icons();
std::string marine_alert_icon_glyph(MarineAlertIcon icon, GlyphSet glyphs = GlyphSet::Auto);

} // namespace tuinator
