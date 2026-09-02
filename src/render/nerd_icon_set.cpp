#include <tuinator/render/nerd_icon_set.hpp>

namespace tuinator {

const char* nerd_icon_set_label(NerdIconSet set) {
    switch (set) {
    case NerdIconSet::Weather:
        return "Weather";
    case NerdIconSet::Moon:
        return "Moon";
    case NerdIconSet::Clock:
        return "Clock";
    case NerdIconSet::Compass:
        return "Compass";
    case NerdIconSet::Measurement:
        return "Measurement";
    case NerdIconSet::Astronomy:
        return "Astronomy";
    case NerdIconSet::AirQuality:
        return "AirQuality";
    case NerdIconSet::Hazard:
        return "Hazard";
    case NerdIconSet::MarineAlert:
        return "MarineAlert";
    case NerdIconSet::Action:
        return "Action";
    }
    return "Unknown";
}

const char* nerd_icon_set_path_prefix(NerdIconSet set) {
    switch (set) {
    case NerdIconSet::Weather:
        return "weather";
    case NerdIconSet::Moon:
        return "moon";
    case NerdIconSet::Clock:
        return "clock";
    case NerdIconSet::Compass:
        return "compass";
    case NerdIconSet::Measurement:
        return "measurement";
    case NerdIconSet::Astronomy:
        return "astronomy";
    case NerdIconSet::AirQuality:
        return "air-quality";
    case NerdIconSet::Hazard:
        return "hazard";
    case NerdIconSet::MarineAlert:
        return "marine";
    case NerdIconSet::Action:
        return "action";
    }
    return "";
}

} // namespace tuinator
