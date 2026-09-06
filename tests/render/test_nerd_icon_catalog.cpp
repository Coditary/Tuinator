#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/render/action_icon.hpp>
#include <tuinator/render/air_quality_icon.hpp>
#include <tuinator/render/astronomy_icon.hpp>
#include <tuinator/render/clock_icon.hpp>
#include <tuinator/render/compass_icon.hpp>
#include <tuinator/render/hazard_icon.hpp>
#include <tuinator/render/marine_alert_icon.hpp>
#include <tuinator/render/measurement_icon.hpp>
#include <tuinator/render/moon_icon.hpp>
#include <tuinator/render/weather_icon.hpp>
#include <tuinator/widgets/display/nerd_icon_catalog_gallery.hpp>

#include <cstring>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(nerd_icon_domains_cover_all_weather_font_glyphs) {
    TUINATOR_CHECK_EQ(tuinator::all_weather_icons().size(), 119u);
    TUINATOR_CHECK_EQ(tuinator::all_moon_icons().size(), 56u);
    TUINATOR_CHECK_EQ(tuinator::all_clock_icons().size(), 12u);
    TUINATOR_CHECK_EQ(tuinator::all_compass_icons().size(), 8u);
    TUINATOR_CHECK_EQ(tuinator::all_measurement_icons().size(), 9u);
    TUINATOR_CHECK_EQ(tuinator::all_astronomy_icons().size(), 11u);
    TUINATOR_CHECK_EQ(tuinator::all_air_quality_icons().size(), 3u);
    TUINATOR_CHECK_EQ(tuinator::all_hazard_icons().size(), 6u);
    TUINATOR_CHECK_EQ(tuinator::all_marine_alert_icons().size(), 2u);
    TUINATOR_CHECK_EQ(tuinator::all_action_icons().size(), 2u);

    const std::size_t total = tuinator::all_weather_icons().size() + tuinator::all_moon_icons().size() +
                              tuinator::all_clock_icons().size() + tuinator::all_compass_icons().size() +
                              tuinator::all_measurement_icons().size() + tuinator::all_astronomy_icons().size() +
                              tuinator::all_air_quality_icons().size() + tuinator::all_hazard_icons().size() +
                              tuinator::all_marine_alert_icons().size() + tuinator::all_action_icons().size();
    TUINATOR_CHECK_EQ(total, 228u);
}

TUINATOR_TEST(nerd_icon_nested_paths_use_domain_prefixes) {
    TUINATOR_CHECK(std::strcmp(tuinator::weather_icon_path(tuinator::WeatherIcon::DaySunny), "weather-day-sunny") == 0);
    TUINATOR_CHECK(std::strcmp(tuinator::clock_icon_path(tuinator::ClockIcon::Time1), "clock-time-1") == 0);
    TUINATOR_CHECK(std::strcmp(tuinator::moon_icon_path(tuinator::MoonIcon::MoonFull), "moon-full") == 0);
    TUINATOR_CHECK(
        std::strcmp(tuinator::compass_icon_path(tuinator::CompassIcon::DirectionUp), "compass-direction-up") == 0);
    TUINATOR_CHECK(std::strcmp(tuinator::weather_icon_path(tuinator::WeatherIcon::WindEast), "weather-wind-east") == 0);
    TUINATOR_CHECK(
        std::strcmp(tuinator::measurement_icon_path(tuinator::MeasurementIcon::Celsius), "measurement-celsius") == 0);
    TUINATOR_CHECK(std::strcmp(tuinator::action_icon_path(tuinator::ActionIcon::Refresh), "action-refresh") == 0);
    TUINATOR_CHECK(std::strcmp(tuinator::astronomy_icon_path(tuinator::AstronomyIcon::Horizon), "astronomy-horizon") ==
                   0);
}

TUINATOR_TEST(nerd_icon_from_path_works_per_domain) {
    TUINATOR_CHECK(tuinator::clock_icon_from_path("clock-time-12").has_value());
    TUINATOR_CHECK(tuinator::moon_icon_from_path("moon-waxing-crescent-1").has_value());
    TUINATOR_CHECK(tuinator::weather_icon_from_path("weather-wind-east").has_value());
    TUINATOR_CHECK(tuinator::weather_icon_from_path("weather-umbrella").has_value());
    TUINATOR_CHECK(tuinator::hazard_icon_from_path("hazard-volcano").has_value());
}

TUINATOR_TEST(nerd_icon_catalog_gallery_renders_all_domains) {
    tuinator::MemoryTerminalBackend backend({100, 300});
    backend.init();

    tuinator::NerdIconCatalogGallery gallery;
    gallery.layout({0, 0, 100, 300});
    tuinator::test::render_root(gallery, backend);

    TUINATOR_CHECK(tuinator::test::row_has(backend, "Nerd Icon Catalog (228)"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "[[ Weather ]]"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "[[ Clock ]]"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "[[ Moon ]]"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "weather-day-sunny"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "clock-time-1"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "moon-full"));
}
