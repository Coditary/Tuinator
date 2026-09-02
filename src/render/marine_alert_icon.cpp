#include <tuinator/render/marine_alert_icon.hpp>

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

constexpr MarineAlertIconDescriptor make(
    MarineAlertIcon kind,
    MarineAlertIconCategory category,
    const char* path,
    const char* nerd_suffix,
    char32_t codepoint,
    char ascii) {
    return MarineAlertIconDescriptor{kind, category, path, nerd_suffix, codepoint, ascii};
}

constexpr std::array<MarineAlertIconDescriptor, 2> kDescriptors{{
    make(MarineAlertIcon::GaleWarning, MarineAlertIconCategory::MarineAlert, "marine-gale-warning", "gale_warning", 0xE3C5, '*'),
    make(MarineAlertIcon::SmallCraftAdvisory, MarineAlertIconCategory::MarineAlert, "marine-small-craft-advisory", "small_craft_advisory", 0xE3C4, '*'),
}};

static_assert(kDescriptors.size() == 2, "descriptor table out of sync");

const MarineAlertIconDescriptor& descriptor_or_default(MarineAlertIcon icon) {
    const auto index = static_cast<std::size_t>(icon);
    if (index < kDescriptors.size()) {
        return kDescriptors[index];
    }
    return kDescriptors[0];
}

} // namespace

const MarineAlertIconDescriptor& marine_alert_icon_descriptor(MarineAlertIcon icon) {
    return descriptor_or_default(icon);
}

MarineAlertIconCategory marine_alert_icon_category(MarineAlertIcon icon) {
    return marine_alert_icon_descriptor(icon).category;
}

const char* marine_alert_icon_path(MarineAlertIcon icon) {
    return marine_alert_icon_descriptor(icon).path;
}

const char* marine_alert_icon_nerd_suffix(MarineAlertIcon icon) {
    return marine_alert_icon_descriptor(icon).nerd_suffix;
}

const char* marine_alert_icon_category_path(MarineAlertIconCategory category) {
    switch (category) {
    case MarineAlertIconCategory::MarineAlert:
        return "marine";
    }
    return "marine";
}

const char* marine_alert_icon_category_label(MarineAlertIconCategory category) {
    switch (category) {
    case MarineAlertIconCategory::MarineAlert:
        return "Marine Alert";
    }
    return "Other";
}

std::vector<std::string_view> marine_alert_icon_path_segments(MarineAlertIcon icon) {
    const char* path = marine_alert_icon_path(icon);
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

std::optional<MarineAlertIcon> marine_alert_icon_from_path(std::string_view query) {
    for (const MarineAlertIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.path) {
            return descriptor.kind;
        }
    }
    for (const MarineAlertIconDescriptor& descriptor : kDescriptors) {
        const std::string legacy = std::string("marine-") + descriptor.nerd_suffix;
        if (query == legacy) {
            return descriptor.kind;
        }
    }
    for (const MarineAlertIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.nerd_suffix) {
            return descriptor.kind;
        }
    }
    return std::nullopt;
}

std::vector<MarineAlertIcon> marine_alert_icons_in_category(MarineAlertIconCategory category) {
    std::vector<MarineAlertIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const MarineAlertIconDescriptor& descriptor : kDescriptors) {
        if (descriptor.category == category) {
            icons.push_back(descriptor.kind);
        }
    }
    return icons;
}

std::vector<MarineAlertIconCategory> all_marine_alert_icon_categories() {
    std::vector<MarineAlertIconCategory> categories;
    categories.reserve(1);
    categories.push_back(MarineAlertIconCategory::MarineAlert);
    return categories;
}

std::vector<MarineAlertIcon> all_marine_alert_icons() {
    std::vector<MarineAlertIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const MarineAlertIconDescriptor& descriptor : kDescriptors) {
        icons.push_back(descriptor.kind);
    }
    return icons;
}

std::string marine_alert_icon_glyph(MarineAlertIcon icon, GlyphSet glyphs) {
    const MarineAlertIconDescriptor& descriptor = marine_alert_icon_descriptor(icon);
    if (glyphs == GlyphSet::Auto) {
        glyphs = detect_file_icon_glyph_set();
    }
    if (glyphs == GlyphSet::Ascii) {
        return std::string(1, descriptor.ascii_fallback);
    }
    return utf8_from(descriptor.codepoint);
}

} // namespace tuinator
