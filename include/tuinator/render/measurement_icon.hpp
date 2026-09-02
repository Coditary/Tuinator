#pragma once

#include <tuinator/render/glyphs.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

enum class MeasurementIconCategory {
    Measurement,
};

/// 9 icons from Nerd Fonts weather set (Measurement, v3.4.0).
enum class MeasurementIcon {
    // Measurement
    Barometer,
    Celsius,
    Degrees,
    Fahrenheit,
    Hot,
    Humidity,
    Thermometer,
    ThermometerExterior,
    ThermometerInternal,
};

struct MeasurementIconDescriptor {
    MeasurementIcon kind = MeasurementIcon::Barometer;
    MeasurementIconCategory category = MeasurementIconCategory::Measurement;
    const char* path = "";
    const char* nerd_suffix = "";
    char32_t codepoint = 0;
    char ascii_fallback = '?';
};

const MeasurementIconDescriptor& measurement_icon_descriptor(MeasurementIcon icon);
MeasurementIconCategory measurement_icon_category(MeasurementIcon icon);
const char* measurement_icon_path(MeasurementIcon icon);
const char* measurement_icon_nerd_suffix(MeasurementIcon icon);
const char* measurement_icon_category_path(MeasurementIconCategory category);
const char* measurement_icon_category_label(MeasurementIconCategory category);
std::vector<std::string_view> measurement_icon_path_segments(MeasurementIcon icon);
std::optional<MeasurementIcon> measurement_icon_from_path(std::string_view path);
std::vector<MeasurementIcon> measurement_icons_in_category(MeasurementIconCategory category);
std::vector<MeasurementIconCategory> all_measurement_icon_categories();
std::vector<MeasurementIcon> all_measurement_icons();
std::string measurement_icon_glyph(MeasurementIcon icon, GlyphSet glyphs = GlyphSet::Auto);

} // namespace tuinator
