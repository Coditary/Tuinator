#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/weather_icon.hpp>

#include <cstring>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(weather_icon_total_count) { TUINATOR_CHECK_EQ(tuinator::all_weather_icons().size(), 119u); }

TUINATOR_TEST(weather_icons_in_category_day) {
    const auto icons = tuinator::weather_icons_in_category(tuinator::WeatherIconCategory::Day);
    TUINATOR_CHECK_EQ(icons.size(), 24u);
}

TUINATOR_TEST(weather_icon_category_nesting) {
    TUINATOR_CHECK_EQ(tuinator::weather_icon_category(tuinator::WeatherIcon::DaySunny),
                      tuinator::WeatherIconCategory::Day);
}

TUINATOR_TEST(weather_icon_from_path_nested_lookup) {
    const auto icon = tuinator::weather_icon_from_path("weather-day-sunny");
    TUINATOR_CHECK(icon.has_value());
    TUINATOR_CHECK_EQ(*icon, tuinator::WeatherIcon::DaySunny);
}

TUINATOR_TEST(weather_icon_path_segments_are_nested) {
    const auto segments = tuinator::weather_icon_path_segments(tuinator::WeatherIcon::DaySunny);
    TUINATOR_CHECK_EQ(segments.size(), 3u);
    TUINATOR_CHECK(segments[0] == "weather");
    TUINATOR_CHECK(segments[1] == "day");
    TUINATOR_CHECK(segments[2] == "sunny");
}

TUINATOR_TEST(weather_icon_no_longer_contains_clock_or_moon) {
    TUINATOR_CHECK(!tuinator::weather_icon_from_path("clock-time-1").has_value());
    TUINATOR_CHECK(!tuinator::weather_icon_from_path("moon-full").has_value());
    TUINATOR_CHECK_EQ(tuinator::all_weather_icon_categories().size(), 10u);
}

TUINATOR_TEST(weather_icon_every_glyph_renders_non_empty) {
    for (tuinator::WeatherIcon icon : tuinator::all_weather_icons()) {
        const std::string glyph = tuinator::weather_icon_glyph(icon, tuinator::GlyphSet::Unicode);
        TUINATOR_CHECK(!glyph.empty());
    }
}
