#include <tuinator/render/compass_icon.hpp>

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

constexpr CompassIconDescriptor make(
    CompassIcon kind,
    CompassIconCategory category,
    const char* path,
    const char* nerd_suffix,
    char32_t codepoint,
    char ascii) {
    return CompassIconDescriptor{kind, category, path, nerd_suffix, codepoint, ascii};
}

constexpr std::array<CompassIconDescriptor, 8> kDescriptors{{
    make(CompassIcon::DirectionDown, CompassIconCategory::Direction, "compass-direction-down", "direction_down", 0xE340, '>'),
    make(CompassIcon::DirectionDownLeft, CompassIconCategory::Direction, "compass-direction-down-left", "direction_down_left", 0xE33F, '>'),
    make(CompassIcon::DirectionDownRight, CompassIconCategory::Direction, "compass-direction-down-right", "direction_down_right", 0xE380, '>'),
    make(CompassIcon::DirectionLeft, CompassIconCategory::Direction, "compass-direction-left", "direction_left", 0xE344, '>'),
    make(CompassIcon::DirectionRight, CompassIconCategory::Direction, "compass-direction-right", "direction_right", 0xE349, '>'),
    make(CompassIcon::DirectionUp, CompassIconCategory::Direction, "compass-direction-up", "direction_up", 0xE353, '>'),
    make(CompassIcon::DirectionUpLeft, CompassIconCategory::Direction, "compass-direction-up-left", "direction_up_left", 0xE37F, '>'),
    make(CompassIcon::DirectionUpRight, CompassIconCategory::Direction, "compass-direction-up-right", "direction_up_right", 0xE352, '>'),
}};

static_assert(kDescriptors.size() == 8, "descriptor table out of sync");

const CompassIconDescriptor& descriptor_or_default(CompassIcon icon) {
    const auto index = static_cast<std::size_t>(icon);
    if (index < kDescriptors.size()) {
        return kDescriptors[index];
    }
    return kDescriptors[0];
}

} // namespace

const CompassIconDescriptor& compass_icon_descriptor(CompassIcon icon) {
    return descriptor_or_default(icon);
}

CompassIconCategory compass_icon_category(CompassIcon icon) {
    return compass_icon_descriptor(icon).category;
}

const char* compass_icon_path(CompassIcon icon) {
    return compass_icon_descriptor(icon).path;
}

const char* compass_icon_nerd_suffix(CompassIcon icon) {
    return compass_icon_descriptor(icon).nerd_suffix;
}

const char* compass_icon_category_path(CompassIconCategory category) {
    switch (category) {
    case CompassIconCategory::Direction:
        return "compass-direction";
    }
    return "compass";
}

const char* compass_icon_category_label(CompassIconCategory category) {
    switch (category) {
    case CompassIconCategory::Direction:
        return "Direction";
    }
    return "Other";
}

std::vector<std::string_view> compass_icon_path_segments(CompassIcon icon) {
    const char* path = compass_icon_path(icon);
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

std::optional<CompassIcon> compass_icon_from_path(std::string_view query) {
    for (const CompassIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.path) {
            return descriptor.kind;
        }
    }
    for (const CompassIconDescriptor& descriptor : kDescriptors) {
        const std::string legacy = std::string("compass-") + descriptor.nerd_suffix;
        if (query == legacy) {
            return descriptor.kind;
        }
    }
    for (const CompassIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.nerd_suffix) {
            return descriptor.kind;
        }
    }
    return std::nullopt;
}

std::vector<CompassIcon> compass_icons_in_category(CompassIconCategory category) {
    std::vector<CompassIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const CompassIconDescriptor& descriptor : kDescriptors) {
        if (descriptor.category == category) {
            icons.push_back(descriptor.kind);
        }
    }
    return icons;
}

std::vector<CompassIconCategory> all_compass_icon_categories() {
    std::vector<CompassIconCategory> categories;
    categories.reserve(1);
    categories.push_back(CompassIconCategory::Direction);
    return categories;
}

std::vector<CompassIcon> all_compass_icons() {
    std::vector<CompassIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const CompassIconDescriptor& descriptor : kDescriptors) {
        icons.push_back(descriptor.kind);
    }
    return icons;
}

std::string compass_icon_glyph(CompassIcon icon, GlyphSet glyphs) {
    const CompassIconDescriptor& descriptor = compass_icon_descriptor(icon);
    if (glyphs == GlyphSet::Auto) {
        glyphs = detect_file_icon_glyph_set();
    }
    if (glyphs == GlyphSet::Ascii) {
        return std::string(1, descriptor.ascii_fallback);
    }
    return utf8_from(descriptor.codepoint);
}

} // namespace tuinator
