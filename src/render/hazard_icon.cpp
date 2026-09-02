#include <tuinator/render/hazard_icon.hpp>

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

constexpr HazardIconDescriptor make(
    HazardIcon kind,
    HazardIconCategory category,
    const char* path,
    const char* nerd_suffix,
    char32_t codepoint,
    char ascii) {
    return HazardIconDescriptor{kind, category, path, nerd_suffix, codepoint, ascii};
}

constexpr std::array<HazardIconDescriptor, 6> kDescriptors{{
    make(HazardIcon::Earthquake, HazardIconCategory::Hazard, "hazard-earthquake", "earthquake", 0xE3BE, '*'),
    make(HazardIcon::Fire, HazardIconCategory::Hazard, "hazard-fire", "fire", 0xE3BF, '*'),
    make(HazardIcon::Flood, HazardIconCategory::Hazard, "hazard-flood", "flood", 0xE375, '*'),
    make(HazardIcon::Meteor, HazardIconCategory::Hazard, "hazard-meteor", "meteor", 0xE36A, '*'),
    make(HazardIcon::Tsunami, HazardIconCategory::Hazard, "hazard-tsunami", "tsunami", 0xE3BD, 'o'),
    make(HazardIcon::Volcano, HazardIconCategory::Hazard, "hazard-volcano", "volcano", 0xE3C0, '*'),
}};

static_assert(kDescriptors.size() == 6, "descriptor table out of sync");

const HazardIconDescriptor& descriptor_or_default(HazardIcon icon) {
    const auto index = static_cast<std::size_t>(icon);
    if (index < kDescriptors.size()) {
        return kDescriptors[index];
    }
    return kDescriptors[0];
}

} // namespace

const HazardIconDescriptor& hazard_icon_descriptor(HazardIcon icon) {
    return descriptor_or_default(icon);
}

HazardIconCategory hazard_icon_category(HazardIcon icon) {
    return hazard_icon_descriptor(icon).category;
}

const char* hazard_icon_path(HazardIcon icon) {
    return hazard_icon_descriptor(icon).path;
}

const char* hazard_icon_nerd_suffix(HazardIcon icon) {
    return hazard_icon_descriptor(icon).nerd_suffix;
}

const char* hazard_icon_category_path(HazardIconCategory category) {
    switch (category) {
    case HazardIconCategory::Hazard:
        return "hazard";
    }
    return "hazard";
}

const char* hazard_icon_category_label(HazardIconCategory category) {
    switch (category) {
    case HazardIconCategory::Hazard:
        return "Hazard";
    }
    return "Other";
}

std::vector<std::string_view> hazard_icon_path_segments(HazardIcon icon) {
    const char* path = hazard_icon_path(icon);
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

std::optional<HazardIcon> hazard_icon_from_path(std::string_view query) {
    for (const HazardIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.path) {
            return descriptor.kind;
        }
    }
    for (const HazardIconDescriptor& descriptor : kDescriptors) {
        const std::string legacy = std::string("hazard-") + descriptor.nerd_suffix;
        if (query == legacy) {
            return descriptor.kind;
        }
    }
    for (const HazardIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.nerd_suffix) {
            return descriptor.kind;
        }
    }
    return std::nullopt;
}

std::vector<HazardIcon> hazard_icons_in_category(HazardIconCategory category) {
    std::vector<HazardIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const HazardIconDescriptor& descriptor : kDescriptors) {
        if (descriptor.category == category) {
            icons.push_back(descriptor.kind);
        }
    }
    return icons;
}

std::vector<HazardIconCategory> all_hazard_icon_categories() {
    std::vector<HazardIconCategory> categories;
    categories.reserve(1);
    categories.push_back(HazardIconCategory::Hazard);
    return categories;
}

std::vector<HazardIcon> all_hazard_icons() {
    std::vector<HazardIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const HazardIconDescriptor& descriptor : kDescriptors) {
        icons.push_back(descriptor.kind);
    }
    return icons;
}

std::string hazard_icon_glyph(HazardIcon icon, GlyphSet glyphs) {
    const HazardIconDescriptor& descriptor = hazard_icon_descriptor(icon);
    if (glyphs == GlyphSet::Auto) {
        glyphs = detect_file_icon_glyph_set();
    }
    if (glyphs == GlyphSet::Ascii) {
        return std::string(1, descriptor.ascii_fallback);
    }
    return utf8_from(descriptor.codepoint);
}

} // namespace tuinator
