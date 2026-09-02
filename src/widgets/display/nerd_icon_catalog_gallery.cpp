#include <tuinator/widgets/display/nerd_icon_catalog_gallery.hpp>

#include <tuinator/render/action_icon.hpp>
#include <tuinator/render/air_quality_icon.hpp>
#include <tuinator/render/astronomy_icon.hpp>
#include <tuinator/render/clock_icon.hpp>
#include <tuinator/render/compass_icon.hpp>
#include <tuinator/render/hazard_icon.hpp>
#include <tuinator/render/moon_icon.hpp>
#include <tuinator/render/marine_alert_icon.hpp>
#include <tuinator/render/measurement_icon.hpp>
#include <tuinator/render/text.hpp>
#include <tuinator/render/weather_icon.hpp>

#include <cstdio>
#include <string>

namespace tuinator {

namespace {

struct CatalogRow {
    std::string path;
    std::string glyph;
};

template <typename Icon, typename DescriptorFn, typename GlyphFn, typename AllFn>
std::vector<CatalogRow> rows_for_set(DescriptorFn descriptor, GlyphFn glyph, AllFn all_icons) {
    std::vector<CatalogRow> rows;
    for (Icon icon : all_icons()) {
        rows.push_back({descriptor(icon).path, glyph(icon, GlyphSet::Unicode)});
    }
    return rows;
}

std::vector<CatalogRow> catalog_rows_for(NerdIconSet set) {
    switch (set) {
    case NerdIconSet::Weather:
        return rows_for_set<WeatherIcon>(weather_icon_descriptor, weather_icon_glyph, all_weather_icons);
    case NerdIconSet::Moon:
        return rows_for_set<MoonIcon>(
            moon_icon_descriptor, moon_icon_glyph, all_moon_icons);
    case NerdIconSet::Clock:
        return rows_for_set<ClockIcon>(clock_icon_descriptor, clock_icon_glyph, all_clock_icons);
    case NerdIconSet::Compass:
        return rows_for_set<CompassIcon>(compass_icon_descriptor, compass_icon_glyph, all_compass_icons);
    case NerdIconSet::Measurement:
        return rows_for_set<MeasurementIcon>(
            measurement_icon_descriptor, measurement_icon_glyph, all_measurement_icons);
    case NerdIconSet::Astronomy:
        return rows_for_set<AstronomyIcon>(
            astronomy_icon_descriptor, astronomy_icon_glyph, all_astronomy_icons);
    case NerdIconSet::AirQuality:
        return rows_for_set<AirQualityIcon>(
            air_quality_icon_descriptor, air_quality_icon_glyph, all_air_quality_icons);
    case NerdIconSet::Hazard:
        return rows_for_set<HazardIcon>(hazard_icon_descriptor, hazard_icon_glyph, all_hazard_icons);
    case NerdIconSet::MarineAlert:
        return rows_for_set<MarineAlertIcon>(
            marine_alert_icon_descriptor, marine_alert_icon_glyph, all_marine_alert_icons);
    case NerdIconSet::Action:
        return rows_for_set<ActionIcon>(action_icon_descriptor, action_icon_glyph, all_action_icons);
    }
    return {};
}

int row_count_for(NerdIconSet set) {
    return static_cast<int>(catalog_rows_for(set).size());
}

} // namespace

NerdIconCatalogGallery::NerdIconCatalogGallery(NerdIconCatalogGalleryStyle style) : style_(std::move(style)) {}

Size NerdIconCatalogGallery::preferred_size() const {
    int rows = 1;
    for (NerdIconSet set : {
             NerdIconSet::Weather,
             NerdIconSet::Moon,
             NerdIconSet::Clock,
             NerdIconSet::Compass,
             NerdIconSet::Measurement,
             NerdIconSet::Astronomy,
             NerdIconSet::AirQuality,
             NerdIconSet::Hazard,
             NerdIconSet::MarineAlert,
             NerdIconSet::Action,
         }) {
        const int count = row_count_for(set);
        if (count == 0) {
            continue;
        }
        rows += 1; // domain header
        rows += count;
    }
    return {76, std::max(1, rows)};
}

void NerdIconCatalogGallery::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    canvas.fill_rect(bounds_, ' ', style_.background);

    int y = bounds_.y;
    const int x = bounds_.x;

    auto draw_line = [&](const std::string& text, const Style& style) {
        if (y >= bounds_.y + bounds_.height) {
            return;
        }
        canvas.draw_text({x, y}, text, style);
        ++y;
    };

    draw_line("Nerd Icon Catalog (228)", style_.title);

    for (NerdIconSet set : {
             NerdIconSet::Weather,
             NerdIconSet::Moon,
             NerdIconSet::Clock,
             NerdIconSet::Compass,
             NerdIconSet::Measurement,
             NerdIconSet::Astronomy,
             NerdIconSet::AirQuality,
             NerdIconSet::Hazard,
             NerdIconSet::MarineAlert,
             NerdIconSet::Action,
         }) {
        const auto rows = catalog_rows_for(set);
        if (rows.empty()) {
            continue;
        }

        char header[96];
        std::snprintf(
            header,
            sizeof(header),
            "[[ %s ]] (%zu)",
            nerd_icon_set_label(set),
            rows.size());
        draw_line(header, style_.domain_header);

        for (const CatalogRow& row : rows) {
            if (y >= bounds_.y + bounds_.height) {
                return;
            }
            canvas.draw_text({x, y}, row.glyph, style_.icon);
            const int glyph_width = text_display_width(row.glyph);
            canvas.draw_text({x + glyph_width + 1, y}, row.path, style_.row_text);
            ++y;
        }
    }
}

} // namespace tuinator
