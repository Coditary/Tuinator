#include <tuinator/render/clock_icon.hpp>
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

constexpr ClockIconDescriptor make(ClockIcon kind, ClockIconCategory category, const char* path,
                                   const char* nerd_suffix, char32_t codepoint, char ascii) {
    return ClockIconDescriptor{kind, category, path, nerd_suffix, codepoint, ascii};
}

constexpr std::array<ClockIconDescriptor, 12> kDescriptors{{
    make(ClockIcon::Time1, ClockIconCategory::Clock, "clock-time-1", "time_1", 0xE382, 't'),
    make(ClockIcon::Time10, ClockIconCategory::Clock, "clock-time-10", "time_10", 0xE38B, 't'),
    make(ClockIcon::Time11, ClockIconCategory::Clock, "clock-time-11", "time_11", 0xE38C, 't'),
    make(ClockIcon::Time12, ClockIconCategory::Clock, "clock-time-12", "time_12", 0xE381, 't'),
    make(ClockIcon::Time2, ClockIconCategory::Clock, "clock-time-2", "time_2", 0xE383, 't'),
    make(ClockIcon::Time3, ClockIconCategory::Clock, "clock-time-3", "time_3", 0xE384, 't'),
    make(ClockIcon::Time4, ClockIconCategory::Clock, "clock-time-4", "time_4", 0xE385, 't'),
    make(ClockIcon::Time5, ClockIconCategory::Clock, "clock-time-5", "time_5", 0xE386, 't'),
    make(ClockIcon::Time6, ClockIconCategory::Clock, "clock-time-6", "time_6", 0xE387, 't'),
    make(ClockIcon::Time7, ClockIconCategory::Clock, "clock-time-7", "time_7", 0xE388, 't'),
    make(ClockIcon::Time8, ClockIconCategory::Clock, "clock-time-8", "time_8", 0xE389, 't'),
    make(ClockIcon::Time9, ClockIconCategory::Clock, "clock-time-9", "time_9", 0xE38A, 't'),
}};

static_assert(kDescriptors.size() == 12, "descriptor table out of sync");

const ClockIconDescriptor& descriptor_or_default(ClockIcon icon) {
    const auto index = static_cast<std::size_t>(icon);
    if (index < kDescriptors.size()) {
        return kDescriptors[index];
    }
    return kDescriptors[0];
}

} // namespace

const ClockIconDescriptor& clock_icon_descriptor(ClockIcon icon) { return descriptor_or_default(icon); }

ClockIconCategory clock_icon_category(ClockIcon icon) { return clock_icon_descriptor(icon).category; }

const char* clock_icon_path(ClockIcon icon) { return clock_icon_descriptor(icon).path; }

const char* clock_icon_nerd_suffix(ClockIcon icon) { return clock_icon_descriptor(icon).nerd_suffix; }

const char* clock_icon_category_path(ClockIconCategory category) {
    switch (category) {
    case ClockIconCategory::Clock: return "clock";
    }
    return "clock";
}

const char* clock_icon_category_label(ClockIconCategory category) {
    switch (category) {
    case ClockIconCategory::Clock: return "Clock";
    }
    return "Other";
}

std::vector<std::string_view> clock_icon_path_segments(ClockIcon icon) {
    const char* path = clock_icon_path(icon);
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

std::optional<ClockIcon> clock_icon_from_path(std::string_view query) {
    for (const ClockIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.path) {
            return descriptor.kind;
        }
    }
    for (const ClockIconDescriptor& descriptor : kDescriptors) {
        const std::string legacy = std::string("clock-") + descriptor.nerd_suffix;
        if (query == legacy) {
            return descriptor.kind;
        }
    }
    for (const ClockIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.nerd_suffix) {
            return descriptor.kind;
        }
    }
    return std::nullopt;
}

std::vector<ClockIcon> clock_icons_in_category(ClockIconCategory category) {
    std::vector<ClockIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const ClockIconDescriptor& descriptor : kDescriptors) {
        if (descriptor.category == category) {
            icons.push_back(descriptor.kind);
        }
    }
    return icons;
}

std::vector<ClockIconCategory> all_clock_icon_categories() {
    std::vector<ClockIconCategory> categories;
    categories.reserve(1);
    categories.push_back(ClockIconCategory::Clock);
    return categories;
}

std::vector<ClockIcon> all_clock_icons() {
    std::vector<ClockIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const ClockIconDescriptor& descriptor : kDescriptors) {
        icons.push_back(descriptor.kind);
    }
    return icons;
}

std::string clock_icon_glyph(ClockIcon icon, GlyphSet glyphs) {
    const ClockIconDescriptor& descriptor = clock_icon_descriptor(icon);
    if (glyphs == GlyphSet::Auto) {
        glyphs = detect_file_icon_glyph_set();
    }
    if (glyphs == GlyphSet::Ascii) {
        return std::string(1, descriptor.ascii_fallback);
    }
    return utf8_from(descriptor.codepoint);
}

} // namespace tuinator
