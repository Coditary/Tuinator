#pragma once

#include <tuinator/render/glyphs.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

enum class WeatherIconCategory {
    Day,
    NightAlt,
    Night,
    Cloud,
    Precipitation,
    Storm,
    Wind,
    Unavailable,
    Equipment,
    Misc,
};

/// 119 icons from Nerd Fonts weather set (Weather, v3.4.0).
enum class WeatherIcon {
    // Day
    DayCloudy,
    DayCloudyGusts,
    DayCloudyHigh,
    DayCloudyWindy,
    DayFog,
    DayHail,
    DayHaze,
    DayLightWind,
    DayLightning,
    DayRain,
    DayRainMix,
    DayRainWind,
    DayShowers,
    DaySleet,
    DaySleetStorm,
    DaySnow,
    DaySnowThunderstorm,
    DaySnowWind,
    DaySprinkle,
    DayStormShowers,
    DaySunny,
    DaySunnyOvercast,
    DayThunderstorm,
    DayWindy,
    // NightAlt
    NightAltCloudy,
    NightAltCloudyGusts,
    NightAltCloudyHigh,
    NightAltCloudyWindy,
    NightAltHail,
    NightAltLightning,
    NightAltPartlyCloudy,
    NightAltRain,
    NightAltRainMix,
    NightAltRainWind,
    NightAltShowers,
    NightAltSleet,
    NightAltSleetStorm,
    NightAltSnow,
    NightAltSnowThunderstorm,
    NightAltSnowWind,
    NightAltSprinkle,
    NightAltStormShowers,
    NightAltThunderstorm,
    // Night
    NightClear,
    NightCloudy,
    NightCloudyGusts,
    NightCloudyHigh,
    NightCloudyWindy,
    NightFog,
    NightHail,
    NightLightning,
    NightPartlyCloudy,
    NightRain,
    NightRainMix,
    NightRainWind,
    NightShowers,
    NightSleet,
    NightSleetStorm,
    NightSnow,
    NightSnowThunderstorm,
    NightSnowWind,
    NightSprinkle,
    NightStormShowers,
    NightThunderstorm,
    // Cloud
    Cloud,
    CloudDown,
    CloudRefresh,
    CloudUp,
    Cloudy,
    CloudyGusts,
    CloudyWindy,
    Fog,
    Hail,
    // Precipitation
    Rain,
    RainMix,
    RainWind,
    Raindrop,
    Raindrops,
    Showers,
    Sleet,
    Snow,
    SnowWind,
    SnowflakeCold,
    Sprinkle,
    // Storm
    Hurricane,
    HurricaneWarning,
    Lightning,
    Sandstorm,
    StormShowers,
    StormWarning,
    Thunderstorm,
    Tornado,
    // Wind
    StrongWind,
    WindBeaufort0,
    WindBeaufort1,
    WindBeaufort10,
    WindBeaufort11,
    WindBeaufort12,
    WindBeaufort2,
    WindBeaufort3,
    WindBeaufort4,
    WindBeaufort5,
    WindBeaufort6,
    WindBeaufort7,
    WindBeaufort8,
    WindBeaufort9,
    WindDirection,
    WindEast,
    WindNorth,
    WindNorthEast,
    WindNorthWest,
    WindSouth,
    WindSouthEast,
    WindSouthWest,
    WindWest,
    Windy,
    // Unavailable
    Na,
    // Equipment
    Umbrella,
    // Misc
    Train,
};

struct WeatherIconDescriptor {
    WeatherIcon kind = WeatherIcon::DayCloudy;
    WeatherIconCategory category = WeatherIconCategory::Day;
    const char* path = "";
    const char* nerd_suffix = "";
    char32_t codepoint = 0;
    char ascii_fallback = '?';
};

const WeatherIconDescriptor& weather_icon_descriptor(WeatherIcon icon);
WeatherIconCategory weather_icon_category(WeatherIcon icon);
const char* weather_icon_path(WeatherIcon icon);
const char* weather_icon_nerd_suffix(WeatherIcon icon);
const char* weather_icon_category_path(WeatherIconCategory category);
const char* weather_icon_category_label(WeatherIconCategory category);
std::vector<std::string_view> weather_icon_path_segments(WeatherIcon icon);
std::optional<WeatherIcon> weather_icon_from_path(std::string_view path);
std::vector<WeatherIcon> weather_icons_in_category(WeatherIconCategory category);
std::vector<WeatherIconCategory> all_weather_icon_categories();
std::vector<WeatherIcon> all_weather_icons();
std::string weather_icon_glyph(WeatherIcon icon, GlyphSet glyphs = GlyphSet::Auto);

} // namespace tuinator
