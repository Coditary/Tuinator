#include <tuinator/render/air_quality_icon.hpp>

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

constexpr AirQualityIconDescriptor make(
    AirQualityIcon kind,
    AirQualityIconCategory category,
    const char* path,
    const char* nerd_suffix,
    char32_t codepoint,
    char ascii) {
    return AirQualityIconDescriptor{kind, category, path, nerd_suffix, codepoint, ascii};
}

constexpr std::array<AirQualityIconDescriptor, 3> kDescriptors{{
    make(AirQualityIcon::Dust, AirQualityIconCategory::AirQuality, "air-quality-dust", "dust", 0xE35D, '*'),
    make(AirQualityIcon::Smog, AirQualityIconCategory::AirQuality, "air-quality-smog", "smog", 0xE36D, '*'),
    make(AirQualityIcon::Smoke, AirQualityIconCategory::AirQuality, "air-quality-smoke", "smoke", 0xE35C, '*'),
}};

static_assert(kDescriptors.size() == 3, "descriptor table out of sync");

const AirQualityIconDescriptor& descriptor_or_default(AirQualityIcon icon) {
    const auto index = static_cast<std::size_t>(icon);
    if (index < kDescriptors.size()) {
        return kDescriptors[index];
    }
    return kDescriptors[0];
}

} // namespace

const AirQualityIconDescriptor& air_quality_icon_descriptor(AirQualityIcon icon) {
    return descriptor_or_default(icon);
}

AirQualityIconCategory air_quality_icon_category(AirQualityIcon icon) {
    return air_quality_icon_descriptor(icon).category;
}

const char* air_quality_icon_path(AirQualityIcon icon) {
    return air_quality_icon_descriptor(icon).path;
}

const char* air_quality_icon_nerd_suffix(AirQualityIcon icon) {
    return air_quality_icon_descriptor(icon).nerd_suffix;
}

const char* air_quality_icon_category_path(AirQualityIconCategory category) {
    switch (category) {
    case AirQualityIconCategory::AirQuality:
        return "air-quality";
    }
    return "air-quality";
}

const char* air_quality_icon_category_label(AirQualityIconCategory category) {
    switch (category) {
    case AirQualityIconCategory::AirQuality:
        return "Air Quality";
    }
    return "Other";
}

std::vector<std::string_view> air_quality_icon_path_segments(AirQualityIcon icon) {
    const char* path = air_quality_icon_path(icon);
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

std::optional<AirQualityIcon> air_quality_icon_from_path(std::string_view query) {
    for (const AirQualityIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.path) {
            return descriptor.kind;
        }
    }
    for (const AirQualityIconDescriptor& descriptor : kDescriptors) {
        const std::string legacy = std::string("air-quality-") + descriptor.nerd_suffix;
        if (query == legacy) {
            return descriptor.kind;
        }
    }
    for (const AirQualityIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.nerd_suffix) {
            return descriptor.kind;
        }
    }
    return std::nullopt;
}

std::vector<AirQualityIcon> air_quality_icons_in_category(AirQualityIconCategory category) {
    std::vector<AirQualityIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const AirQualityIconDescriptor& descriptor : kDescriptors) {
        if (descriptor.category == category) {
            icons.push_back(descriptor.kind);
        }
    }
    return icons;
}

std::vector<AirQualityIconCategory> all_air_quality_icon_categories() {
    std::vector<AirQualityIconCategory> categories;
    categories.reserve(1);
    categories.push_back(AirQualityIconCategory::AirQuality);
    return categories;
}

std::vector<AirQualityIcon> all_air_quality_icons() {
    std::vector<AirQualityIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const AirQualityIconDescriptor& descriptor : kDescriptors) {
        icons.push_back(descriptor.kind);
    }
    return icons;
}

std::string air_quality_icon_glyph(AirQualityIcon icon, GlyphSet glyphs) {
    const AirQualityIconDescriptor& descriptor = air_quality_icon_descriptor(icon);
    if (glyphs == GlyphSet::Auto) {
        glyphs = detect_file_icon_glyph_set();
    }
    if (glyphs == GlyphSet::Ascii) {
        return std::string(1, descriptor.ascii_fallback);
    }
    return utf8_from(descriptor.codepoint);
}

} // namespace tuinator
