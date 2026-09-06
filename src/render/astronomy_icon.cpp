#include <tuinator/render/astronomy_icon.hpp>
#include <tuinator/render/glyphs.hpp>

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

constexpr AstronomyIconDescriptor make(AstronomyIcon kind, AstronomyIconCategory category, const char* path,
                                       const char* nerd_suffix, char32_t codepoint, char ascii) {
    return AstronomyIconDescriptor{kind, category, path, nerd_suffix, codepoint, ascii};
}

constexpr std::array<AstronomyIconDescriptor, 11> kDescriptors{{
    make(AstronomyIcon::Moonrise, AstronomyIconCategory::Celestial, "astronomy-moonrise", "moonrise", 0xE3C1, 'n'),
    make(AstronomyIcon::Moonset, AstronomyIconCategory::Celestial, "astronomy-moonset", "moonset", 0xE3C2, 'n'),
    make(AstronomyIcon::Sunrise, AstronomyIconCategory::Celestial, "astronomy-sunrise", "sunrise", 0xE34C, 'o'),
    make(AstronomyIcon::Sunset, AstronomyIconCategory::Celestial, "astronomy-sunset", "sunset", 0xE34D, 'o'),
    make(AstronomyIcon::LunarEclipse, AstronomyIconCategory::Eclipse, "astronomy-lunar-eclipse", "lunar_eclipse",
         0xE369, '*'),
    make(AstronomyIcon::SolarEclipse, AstronomyIconCategory::Eclipse, "astronomy-solar-eclipse", "solar_eclipse",
         0xE368, '*'),
    make(AstronomyIcon::Horizon, AstronomyIconCategory::Sky, "astronomy-horizon", "horizon", 0xE343, '*'),
    make(AstronomyIcon::HorizonAlt, AstronomyIconCategory::Sky, "astronomy-horizon-alt", "horizon_alt", 0xE342, '*'),
    make(AstronomyIcon::Stars, AstronomyIconCategory::Sky, "astronomy-stars", "stars", 0xE370, '*'),
    make(AstronomyIcon::Alien, AstronomyIconCategory::Misc, "astronomy-alien", "alien", 0xE36E, '*'),
    make(AstronomyIcon::Aliens, AstronomyIconCategory::Misc, "astronomy-aliens", "aliens", 0xE345, '*'),
}};

static_assert(kDescriptors.size() == 11, "descriptor table out of sync");

const AstronomyIconDescriptor& descriptor_or_default(AstronomyIcon icon) {
    const auto index = static_cast<std::size_t>(icon);
    if (index < kDescriptors.size()) {
        return kDescriptors[index];
    }
    return kDescriptors[0];
}

} // namespace

const AstronomyIconDescriptor& astronomy_icon_descriptor(AstronomyIcon icon) { return descriptor_or_default(icon); }

AstronomyIconCategory astronomy_icon_category(AstronomyIcon icon) { return astronomy_icon_descriptor(icon).category; }

const char* astronomy_icon_path(AstronomyIcon icon) { return astronomy_icon_descriptor(icon).path; }

const char* astronomy_icon_nerd_suffix(AstronomyIcon icon) { return astronomy_icon_descriptor(icon).nerd_suffix; }

const char* astronomy_icon_category_path(AstronomyIconCategory category) {
    switch (category) {
    case AstronomyIconCategory::Celestial: return "astronomy-celestial";
    case AstronomyIconCategory::Eclipse: return "astronomy-eclipse";
    case AstronomyIconCategory::Sky: return "astronomy-sky";
    case AstronomyIconCategory::Misc: return "astronomy-misc";
    }
    return "astronomy";
}

const char* astronomy_icon_category_label(AstronomyIconCategory category) {
    switch (category) {
    case AstronomyIconCategory::Celestial: return "Celestial";
    case AstronomyIconCategory::Eclipse: return "Eclipse";
    case AstronomyIconCategory::Sky: return "Sky";
    case AstronomyIconCategory::Misc: return "Misc";
    }
    return "Other";
}

std::vector<std::string_view> astronomy_icon_path_segments(AstronomyIcon icon) {
    const char* path = astronomy_icon_path(icon);
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

std::optional<AstronomyIcon> astronomy_icon_from_path(std::string_view query) {
    for (const AstronomyIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.path) {
            return descriptor.kind;
        }
    }
    for (const AstronomyIconDescriptor& descriptor : kDescriptors) {
        const std::string legacy = std::string("astronomy-") + descriptor.nerd_suffix;
        if (query == legacy) {
            return descriptor.kind;
        }
    }
    for (const AstronomyIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.nerd_suffix) {
            return descriptor.kind;
        }
    }
    return std::nullopt;
}

std::vector<AstronomyIcon> astronomy_icons_in_category(AstronomyIconCategory category) {
    std::vector<AstronomyIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const AstronomyIconDescriptor& descriptor : kDescriptors) {
        if (descriptor.category == category) {
            icons.push_back(descriptor.kind);
        }
    }
    return icons;
}

std::vector<AstronomyIconCategory> all_astronomy_icon_categories() {
    std::vector<AstronomyIconCategory> categories;
    categories.reserve(4);
    categories.push_back(AstronomyIconCategory::Celestial);
    categories.push_back(AstronomyIconCategory::Eclipse);
    categories.push_back(AstronomyIconCategory::Sky);
    categories.push_back(AstronomyIconCategory::Misc);
    return categories;
}

std::vector<AstronomyIcon> all_astronomy_icons() {
    std::vector<AstronomyIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const AstronomyIconDescriptor& descriptor : kDescriptors) {
        icons.push_back(descriptor.kind);
    }
    return icons;
}

std::string astronomy_icon_glyph(AstronomyIcon icon, GlyphSet glyphs) {
    const AstronomyIconDescriptor& descriptor = astronomy_icon_descriptor(icon);
    if (glyphs == GlyphSet::Auto) {
        glyphs = detect_file_icon_glyph_set();
    }
    if (glyphs == GlyphSet::Ascii) {
        return std::string(1, descriptor.ascii_fallback);
    }
    return utf8_from(descriptor.codepoint);
}

} // namespace tuinator
