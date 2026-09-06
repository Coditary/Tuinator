#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/weather_icon.hpp>

#include <array>
#include <string>

namespace tuinator {

namespace {

std::string utf8_from(char32_t cp) {
    std::string out;
    if (cp < 0x80) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    return out;
}

constexpr WeatherIconDescriptor make(WeatherIcon kind, WeatherIconCategory category, const char* path,
                                     const char* nerd_suffix, char32_t codepoint, char ascii) {
    return WeatherIconDescriptor{kind, category, path, nerd_suffix, codepoint, ascii};
}

constexpr std::array<WeatherIconDescriptor, 119> kDescriptors{{
    make(WeatherIcon::DayCloudy, WeatherIconCategory::Day, "weather-day-cloudy", "day_cloudy", 0xE302, 'o'),
    make(WeatherIcon::DayCloudyGusts, WeatherIconCategory::Day, "weather-day-cloudy-gusts", "day_cloudy_gusts", 0xE300,
         'o'),
    make(WeatherIcon::DayCloudyHigh, WeatherIconCategory::Day, "weather-day-cloudy-high", "day_cloudy_high", 0xE376,
         'o'),
    make(WeatherIcon::DayCloudyWindy, WeatherIconCategory::Day, "weather-day-cloudy-windy", "day_cloudy_windy", 0xE301,
         'o'),
    make(WeatherIcon::DayFog, WeatherIconCategory::Day, "weather-day-fog", "day_fog", 0xE303, 'o'),
    make(WeatherIcon::DayHail, WeatherIconCategory::Day, "weather-day-hail", "day_hail", 0xE304, 'o'),
    make(WeatherIcon::DayHaze, WeatherIconCategory::Day, "weather-day-haze", "day_haze", 0xE3AE, 'o'),
    make(WeatherIcon::DayLightWind, WeatherIconCategory::Day, "weather-day-light-wind", "day_light_wind", 0xE3BC, 'o'),
    make(WeatherIcon::DayLightning, WeatherIconCategory::Day, "weather-day-lightning", "day_lightning", 0xE305, 'o'),
    make(WeatherIcon::DayRain, WeatherIconCategory::Day, "weather-day-rain", "day_rain", 0xE308, 'r'),
    make(WeatherIcon::DayRainMix, WeatherIconCategory::Day, "weather-day-rain-mix", "day_rain_mix", 0xE306, 'r'),
    make(WeatherIcon::DayRainWind, WeatherIconCategory::Day, "weather-day-rain-wind", "day_rain_wind", 0xE307, 'r'),
    make(WeatherIcon::DayShowers, WeatherIconCategory::Day, "weather-day-showers", "day_showers", 0xE309, 'r'),
    make(WeatherIcon::DaySleet, WeatherIconCategory::Day, "weather-day-sleet", "day_sleet", 0xE3AA, '*'),
    make(WeatherIcon::DaySleetStorm, WeatherIconCategory::Day, "weather-day-sleet-storm", "day_sleet_storm", 0xE362,
         '*'),
    make(WeatherIcon::DaySnow, WeatherIconCategory::Day, "weather-day-snow", "day_snow", 0xE30A, '*'),
    make(WeatherIcon::DaySnowThunderstorm, WeatherIconCategory::Day, "weather-day-snow-thunderstorm",
         "day_snow_thunderstorm", 0xE365, '*'),
    make(WeatherIcon::DaySnowWind, WeatherIconCategory::Day, "weather-day-snow-wind", "day_snow_wind", 0xE35F, '*'),
    make(WeatherIcon::DaySprinkle, WeatherIconCategory::Day, "weather-day-sprinkle", "day_sprinkle", 0xE30B, 'o'),
    make(WeatherIcon::DayStormShowers, WeatherIconCategory::Day, "weather-day-storm-showers", "day_storm_showers",
         0xE30E, 'r'),
    make(WeatherIcon::DaySunny, WeatherIconCategory::Day, "weather-day-sunny", "day_sunny", 0xE30D, 'o'),
    make(WeatherIcon::DaySunnyOvercast, WeatherIconCategory::Day, "weather-day-sunny-overcast", "day_sunny_overcast",
         0xE30C, 'o'),
    make(WeatherIcon::DayThunderstorm, WeatherIconCategory::Day, "weather-day-thunderstorm", "day_thunderstorm", 0xE30F,
         'o'),
    make(WeatherIcon::DayWindy, WeatherIconCategory::Day, "weather-day-windy", "day_windy", 0xE37D, 'o'),
    make(WeatherIcon::NightAltCloudy, WeatherIconCategory::NightAlt, "weather-night-alt-cloudy", "night_alt_cloudy",
         0xE37E, 'n'),
    make(WeatherIcon::NightAltCloudyGusts, WeatherIconCategory::NightAlt, "weather-night-alt-cloudy-gusts",
         "night_alt_cloudy_gusts", 0xE31F, 'n'),
    make(WeatherIcon::NightAltCloudyHigh, WeatherIconCategory::NightAlt, "weather-night-alt-cloudy-high",
         "night_alt_cloudy_high", 0xE377, 'n'),
    make(WeatherIcon::NightAltCloudyWindy, WeatherIconCategory::NightAlt, "weather-night-alt-cloudy-windy",
         "night_alt_cloudy_windy", 0xE320, 'n'),
    make(WeatherIcon::NightAltHail, WeatherIconCategory::NightAlt, "weather-night-alt-hail", "night_alt_hail", 0xE321,
         'n'),
    make(WeatherIcon::NightAltLightning, WeatherIconCategory::NightAlt, "weather-night-alt-lightning",
         "night_alt_lightning", 0xE322, 'n'),
    make(WeatherIcon::NightAltPartlyCloudy, WeatherIconCategory::NightAlt, "weather-night-alt-partly-cloudy",
         "night_alt_partly_cloudy", 0xE379, 'n'),
    make(WeatherIcon::NightAltRain, WeatherIconCategory::NightAlt, "weather-night-alt-rain", "night_alt_rain", 0xE325,
         'r'),
    make(WeatherIcon::NightAltRainMix, WeatherIconCategory::NightAlt, "weather-night-alt-rain-mix",
         "night_alt_rain_mix", 0xE323, 'r'),
    make(WeatherIcon::NightAltRainWind, WeatherIconCategory::NightAlt, "weather-night-alt-rain-wind",
         "night_alt_rain_wind", 0xE324, 'r'),
    make(WeatherIcon::NightAltShowers, WeatherIconCategory::NightAlt, "weather-night-alt-showers", "night_alt_showers",
         0xE326, 'r'),
    make(WeatherIcon::NightAltSleet, WeatherIconCategory::NightAlt, "weather-night-alt-sleet", "night_alt_sleet",
         0xE3AC, '*'),
    make(WeatherIcon::NightAltSleetStorm, WeatherIconCategory::NightAlt, "weather-night-alt-sleet-storm",
         "night_alt_sleet_storm", 0xE364, '*'),
    make(WeatherIcon::NightAltSnow, WeatherIconCategory::NightAlt, "weather-night-alt-snow", "night_alt_snow", 0xE327,
         '*'),
    make(WeatherIcon::NightAltSnowThunderstorm, WeatherIconCategory::NightAlt, "weather-night-alt-snow-thunderstorm",
         "night_alt_snow_thunderstorm", 0xE367, '*'),
    make(WeatherIcon::NightAltSnowWind, WeatherIconCategory::NightAlt, "weather-night-alt-snow-wind",
         "night_alt_snow_wind", 0xE361, '*'),
    make(WeatherIcon::NightAltSprinkle, WeatherIconCategory::NightAlt, "weather-night-alt-sprinkle",
         "night_alt_sprinkle", 0xE328, 'n'),
    make(WeatherIcon::NightAltStormShowers, WeatherIconCategory::NightAlt, "weather-night-alt-storm-showers",
         "night_alt_storm_showers", 0xE329, 'r'),
    make(WeatherIcon::NightAltThunderstorm, WeatherIconCategory::NightAlt, "weather-night-alt-thunderstorm",
         "night_alt_thunderstorm", 0xE32A, 'n'),
    make(WeatherIcon::NightClear, WeatherIconCategory::Night, "weather-night-clear", "night_clear", 0xE32B, 'n'),
    make(WeatherIcon::NightCloudy, WeatherIconCategory::Night, "weather-night-cloudy", "night_cloudy", 0xE32E, 'n'),
    make(WeatherIcon::NightCloudyGusts, WeatherIconCategory::Night, "weather-night-cloudy-gusts", "night_cloudy_gusts",
         0xE32C, 'n'),
    make(WeatherIcon::NightCloudyHigh, WeatherIconCategory::Night, "weather-night-cloudy-high", "night_cloudy_high",
         0xE378, 'n'),
    make(WeatherIcon::NightCloudyWindy, WeatherIconCategory::Night, "weather-night-cloudy-windy", "night_cloudy_windy",
         0xE32D, 'n'),
    make(WeatherIcon::NightFog, WeatherIconCategory::Night, "weather-night-fog", "night_fog", 0xE346, 'n'),
    make(WeatherIcon::NightHail, WeatherIconCategory::Night, "weather-night-hail", "night_hail", 0xE32F, 'n'),
    make(WeatherIcon::NightLightning, WeatherIconCategory::Night, "weather-night-lightning", "night_lightning", 0xE330,
         'n'),
    make(WeatherIcon::NightPartlyCloudy, WeatherIconCategory::Night, "weather-night-partly-cloudy",
         "night_partly_cloudy", 0xE37B, 'n'),
    make(WeatherIcon::NightRain, WeatherIconCategory::Night, "weather-night-rain", "night_rain", 0xE333, 'r'),
    make(WeatherIcon::NightRainMix, WeatherIconCategory::Night, "weather-night-rain-mix", "night_rain_mix", 0xE331,
         'r'),
    make(WeatherIcon::NightRainWind, WeatherIconCategory::Night, "weather-night-rain-wind", "night_rain_wind", 0xE332,
         'r'),
    make(WeatherIcon::NightShowers, WeatherIconCategory::Night, "weather-night-showers", "night_showers", 0xE334, 'r'),
    make(WeatherIcon::NightSleet, WeatherIconCategory::Night, "weather-night-sleet", "night_sleet", 0xE3AB, '*'),
    make(WeatherIcon::NightSleetStorm, WeatherIconCategory::Night, "weather-night-sleet-storm", "night_sleet_storm",
         0xE363, '*'),
    make(WeatherIcon::NightSnow, WeatherIconCategory::Night, "weather-night-snow", "night_snow", 0xE335, '*'),
    make(WeatherIcon::NightSnowThunderstorm, WeatherIconCategory::Night, "weather-night-snow-thunderstorm",
         "night_snow_thunderstorm", 0xE366, '*'),
    make(WeatherIcon::NightSnowWind, WeatherIconCategory::Night, "weather-night-snow-wind", "night_snow_wind", 0xE360,
         '*'),
    make(WeatherIcon::NightSprinkle, WeatherIconCategory::Night, "weather-night-sprinkle", "night_sprinkle", 0xE336,
         'n'),
    make(WeatherIcon::NightStormShowers, WeatherIconCategory::Night, "weather-night-storm-showers",
         "night_storm_showers", 0xE337, 'r'),
    make(WeatherIcon::NightThunderstorm, WeatherIconCategory::Night, "weather-night-thunderstorm", "night_thunderstorm",
         0xE338, 'n'),
    make(WeatherIcon::Cloud, WeatherIconCategory::Cloud, "weather-cloud", "cloud", 0xE33D, 'c'),
    make(WeatherIcon::CloudDown, WeatherIconCategory::Cloud, "weather-cloud-down", "cloud_down", 0xE33A, 'c'),
    make(WeatherIcon::CloudRefresh, WeatherIconCategory::Cloud, "weather-cloud-refresh", "cloud_refresh", 0xE33B, 'c'),
    make(WeatherIcon::CloudUp, WeatherIconCategory::Cloud, "weather-cloud-up", "cloud_up", 0xE33C, 'c'),
    make(WeatherIcon::Cloudy, WeatherIconCategory::Cloud, "weather-cloudy", "cloudy", 0xE312, 'c'),
    make(WeatherIcon::CloudyGusts, WeatherIconCategory::Cloud, "weather-cloudy-gusts", "cloudy_gusts", 0xE310, 'c'),
    make(WeatherIcon::CloudyWindy, WeatherIconCategory::Cloud, "weather-cloudy-windy", "cloudy_windy", 0xE311, '~'),
    make(WeatherIcon::Fog, WeatherIconCategory::Cloud, "weather-fog", "fog", 0xE313, 'c'),
    make(WeatherIcon::Hail, WeatherIconCategory::Cloud, "weather-hail", "hail", 0xE314, '*'),
    make(WeatherIcon::Rain, WeatherIconCategory::Precipitation, "weather-rain", "rain", 0xE318, 'r'),
    make(WeatherIcon::RainMix, WeatherIconCategory::Precipitation, "weather-rain-mix", "rain_mix", 0xE316, 'r'),
    make(WeatherIcon::RainWind, WeatherIconCategory::Precipitation, "weather-rain-wind", "rain_wind", 0xE317, 'r'),
    make(WeatherIcon::Raindrop, WeatherIconCategory::Precipitation, "weather-raindrop", "raindrop", 0xE371, 'r'),
    make(WeatherIcon::Raindrops, WeatherIconCategory::Precipitation, "weather-raindrops", "raindrops", 0xE34A, 'r'),
    make(WeatherIcon::Showers, WeatherIconCategory::Precipitation, "weather-showers", "showers", 0xE319, 'r'),
    make(WeatherIcon::Sleet, WeatherIconCategory::Precipitation, "weather-sleet", "sleet", 0xE3AD, '*'),
    make(WeatherIcon::Snow, WeatherIconCategory::Precipitation, "weather-snow", "snow", 0xE31A, '*'),
    make(WeatherIcon::SnowWind, WeatherIconCategory::Precipitation, "weather-snow-wind", "snow_wind", 0xE35E, '*'),
    make(WeatherIcon::SnowflakeCold, WeatherIconCategory::Precipitation, "weather-snowflake-cold", "snowflake_cold",
         0xE36F, '*'),
    make(WeatherIcon::Sprinkle, WeatherIconCategory::Precipitation, "weather-sprinkle", "sprinkle", 0xE31B, '*'),
    make(WeatherIcon::Hurricane, WeatherIconCategory::Storm, "weather-hurricane", "hurricane", 0xE36C, '*'),
    make(WeatherIcon::HurricaneWarning, WeatherIconCategory::Storm, "weather-hurricane-warning", "hurricane_warning",
         0xE3C7, '*'),
    make(WeatherIcon::Lightning, WeatherIconCategory::Storm, "weather-lightning", "lightning", 0xE315, '!'),
    make(WeatherIcon::Sandstorm, WeatherIconCategory::Storm, "weather-sandstorm", "sandstorm", 0xE37A, '!'),
    make(WeatherIcon::StormShowers, WeatherIconCategory::Storm, "weather-storm-showers", "storm_showers", 0xE31C, 'r'),
    make(WeatherIcon::StormWarning, WeatherIconCategory::Storm, "weather-storm-warning", "storm_warning", 0xE3C6, '!'),
    make(WeatherIcon::Thunderstorm, WeatherIconCategory::Storm, "weather-thunderstorm", "thunderstorm", 0xE31D, '!'),
    make(WeatherIcon::Tornado, WeatherIconCategory::Storm, "weather-tornado", "tornado", 0xE351, '~'),
    make(WeatherIcon::StrongWind, WeatherIconCategory::Wind, "weather-strong-wind", "strong_wind", 0xE34B, '~'),
    make(WeatherIcon::WindBeaufort0, WeatherIconCategory::Wind, "weather-wind-beaufort-0", "wind_beaufort_0", 0xE3AF,
         '~'),
    make(WeatherIcon::WindBeaufort1, WeatherIconCategory::Wind, "weather-wind-beaufort-1", "wind_beaufort_1", 0xE3B0,
         '~'),
    make(WeatherIcon::WindBeaufort10, WeatherIconCategory::Wind, "weather-wind-beaufort-10", "wind_beaufort_10", 0xE3B9,
         '~'),
    make(WeatherIcon::WindBeaufort11, WeatherIconCategory::Wind, "weather-wind-beaufort-11", "wind_beaufort_11", 0xE3BA,
         '~'),
    make(WeatherIcon::WindBeaufort12, WeatherIconCategory::Wind, "weather-wind-beaufort-12", "wind_beaufort_12", 0xE3BB,
         '~'),
    make(WeatherIcon::WindBeaufort2, WeatherIconCategory::Wind, "weather-wind-beaufort-2", "wind_beaufort_2", 0xE3B1,
         '~'),
    make(WeatherIcon::WindBeaufort3, WeatherIconCategory::Wind, "weather-wind-beaufort-3", "wind_beaufort_3", 0xE3B2,
         '~'),
    make(WeatherIcon::WindBeaufort4, WeatherIconCategory::Wind, "weather-wind-beaufort-4", "wind_beaufort_4", 0xE3B3,
         '~'),
    make(WeatherIcon::WindBeaufort5, WeatherIconCategory::Wind, "weather-wind-beaufort-5", "wind_beaufort_5", 0xE3B4,
         '~'),
    make(WeatherIcon::WindBeaufort6, WeatherIconCategory::Wind, "weather-wind-beaufort-6", "wind_beaufort_6", 0xE3B5,
         '~'),
    make(WeatherIcon::WindBeaufort7, WeatherIconCategory::Wind, "weather-wind-beaufort-7", "wind_beaufort_7", 0xE3B6,
         '~'),
    make(WeatherIcon::WindBeaufort8, WeatherIconCategory::Wind, "weather-wind-beaufort-8", "wind_beaufort_8", 0xE3B7,
         '~'),
    make(WeatherIcon::WindBeaufort9, WeatherIconCategory::Wind, "weather-wind-beaufort-9", "wind_beaufort_9", 0xE3B8,
         '~'),
    make(WeatherIcon::WindDirection, WeatherIconCategory::Wind, "weather-wind-direction", "wind_direction", 0xE3A9,
         '~'),
    make(WeatherIcon::WindEast, WeatherIconCategory::Wind, "weather-wind-east", "wind_east", 0xE35B, '~'),
    make(WeatherIcon::WindNorth, WeatherIconCategory::Wind, "weather-wind-north", "wind_north", 0xE35A, '~'),
    make(WeatherIcon::WindNorthEast, WeatherIconCategory::Wind, "weather-wind-north-east", "wind_north_east", 0xE359,
         '~'),
    make(WeatherIcon::WindNorthWest, WeatherIconCategory::Wind, "weather-wind-north-west", "wind_north_west", 0xE358,
         '~'),
    make(WeatherIcon::WindSouth, WeatherIconCategory::Wind, "weather-wind-south", "wind_south", 0xE357, '~'),
    make(WeatherIcon::WindSouthEast, WeatherIconCategory::Wind, "weather-wind-south-east", "wind_south_east", 0xE356,
         '~'),
    make(WeatherIcon::WindSouthWest, WeatherIconCategory::Wind, "weather-wind-south-west", "wind_south_west", 0xE355,
         '~'),
    make(WeatherIcon::WindWest, WeatherIconCategory::Wind, "weather-wind-west", "wind_west", 0xE354, '~'),
    make(WeatherIcon::Windy, WeatherIconCategory::Wind, "weather-windy", "windy", 0xE31E, '~'),
    make(WeatherIcon::Na, WeatherIconCategory::Unavailable, "weather-na", "na", 0xE374, '*'),
    make(WeatherIcon::Umbrella, WeatherIconCategory::Equipment, "weather-umbrella", "umbrella", 0xE37C, 'u'),
    make(WeatherIcon::Train, WeatherIconCategory::Misc, "weather-train", "train", 0xE3C3, 'r'),
}};

static_assert(kDescriptors.size() == 119, "descriptor table out of sync");

const WeatherIconDescriptor& descriptor_or_default(WeatherIcon icon) {
    const auto index = static_cast<std::size_t>(icon);
    if (index < kDescriptors.size()) {
        return kDescriptors[index];
    }
    return kDescriptors[0];
}

} // namespace

const WeatherIconDescriptor& weather_icon_descriptor(WeatherIcon icon) { return descriptor_or_default(icon); }

WeatherIconCategory weather_icon_category(WeatherIcon icon) { return weather_icon_descriptor(icon).category; }

const char* weather_icon_path(WeatherIcon icon) { return weather_icon_descriptor(icon).path; }

const char* weather_icon_nerd_suffix(WeatherIcon icon) { return weather_icon_descriptor(icon).nerd_suffix; }

const char* weather_icon_category_path(WeatherIconCategory category) {
    switch (category) {
    case WeatherIconCategory::Day: return "weather-day";
    case WeatherIconCategory::NightAlt: return "weather-night-alt";
    case WeatherIconCategory::Night: return "weather-night";
    case WeatherIconCategory::Cloud: return "weather-cloud";
    case WeatherIconCategory::Precipitation: return "weather-precipitation";
    case WeatherIconCategory::Storm: return "weather-storm";
    case WeatherIconCategory::Wind: return "weather-wind";
    case WeatherIconCategory::Unavailable: return "weather-unavailable";
    case WeatherIconCategory::Equipment: return "weather-equipment";
    case WeatherIconCategory::Misc: return "weather-misc";
    }
    return "weather";
}

const char* weather_icon_category_label(WeatherIconCategory category) {
    switch (category) {
    case WeatherIconCategory::Day: return "Day";
    case WeatherIconCategory::NightAlt: return "Night Alt";
    case WeatherIconCategory::Night: return "Night";
    case WeatherIconCategory::Cloud: return "Cloud";
    case WeatherIconCategory::Precipitation: return "Precipitation";
    case WeatherIconCategory::Storm: return "Storm";
    case WeatherIconCategory::Wind: return "Wind";
    case WeatherIconCategory::Unavailable: return "Unavailable";
    case WeatherIconCategory::Equipment: return "Equipment";
    case WeatherIconCategory::Misc: return "Misc";
    }
    return "Other";
}

std::vector<std::string_view> weather_icon_path_segments(WeatherIcon icon) {
    const char* path = weather_icon_path(icon);
    std::vector<std::string_view> segments;
    if (path == nullptr || path[0] == '\0') {
        return segments;
    }
    const char* start = path;
    for (const char* cursor = path; *cursor != '\0'; ++cursor) {
        if (*cursor != '-') {
            continue;
        }
        segments.emplace_back(start, static_cast<std::size_t>(cursor - start));
        start = cursor + 1;
    }
    segments.emplace_back(start);
    return segments;
}

std::optional<WeatherIcon> weather_icon_from_path(std::string_view query) {
    for (const WeatherIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.path) {
            return descriptor.kind;
        }
    }
    for (const WeatherIconDescriptor& descriptor : kDescriptors) {
        const std::string legacy = std::string("weather-") + descriptor.nerd_suffix;
        if (query == legacy) {
            return descriptor.kind;
        }
    }
    for (const WeatherIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.nerd_suffix) {
            return descriptor.kind;
        }
    }
    return std::nullopt;
}

std::vector<WeatherIcon> weather_icons_in_category(WeatherIconCategory category) {
    std::vector<WeatherIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const WeatherIconDescriptor& descriptor : kDescriptors) {
        if (descriptor.category == category) {
            icons.push_back(descriptor.kind);
        }
    }
    return icons;
}

std::vector<WeatherIconCategory> all_weather_icon_categories() {
    std::vector<WeatherIconCategory> categories;
    categories.reserve(10);
    categories.push_back(WeatherIconCategory::Day);
    categories.push_back(WeatherIconCategory::NightAlt);
    categories.push_back(WeatherIconCategory::Night);
    categories.push_back(WeatherIconCategory::Cloud);
    categories.push_back(WeatherIconCategory::Precipitation);
    categories.push_back(WeatherIconCategory::Storm);
    categories.push_back(WeatherIconCategory::Wind);
    categories.push_back(WeatherIconCategory::Unavailable);
    categories.push_back(WeatherIconCategory::Equipment);
    categories.push_back(WeatherIconCategory::Misc);
    return categories;
}

std::vector<WeatherIcon> all_weather_icons() {
    std::vector<WeatherIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const WeatherIconDescriptor& descriptor : kDescriptors) {
        icons.push_back(descriptor.kind);
    }
    return icons;
}

std::string weather_icon_glyph(WeatherIcon icon, GlyphSet glyphs) {
    const WeatherIconDescriptor& descriptor = weather_icon_descriptor(icon);
    if (glyphs == GlyphSet::Auto) {
        glyphs = detect_file_icon_glyph_set();
    }
    if (glyphs == GlyphSet::Ascii) {
        return std::string(1, descriptor.ascii_fallback);
    }
    return utf8_from(descriptor.codepoint);
}

} // namespace tuinator
