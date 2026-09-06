#pragma once

namespace tuinator {

/// Semantic domains for glyphs sourced from Nerd Fonts `i_weather.sh` (v3.4.0).
enum class NerdIconSet {
    Weather,
    Moon,
    Clock,
    Compass,
    Measurement,
    Astronomy,
    AirQuality,
    Hazard,
    MarineAlert,
    Action,
};

const char* nerd_icon_set_label(NerdIconSet set);
const char* nerd_icon_set_path_prefix(NerdIconSet set);

} // namespace tuinator
