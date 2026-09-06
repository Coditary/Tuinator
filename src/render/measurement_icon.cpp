#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/measurement_icon.hpp>

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

constexpr MeasurementIconDescriptor make(MeasurementIcon kind, MeasurementIconCategory category, const char* path,
                                         const char* nerd_suffix, char32_t codepoint, char ascii) {
    return MeasurementIconDescriptor{kind, category, path, nerd_suffix, codepoint, ascii};
}

constexpr std::array<MeasurementIconDescriptor, 9> kDescriptors{{
    make(MeasurementIcon::Barometer, MeasurementIconCategory::Measurement, "measurement-barometer", "barometer", 0xE372,
         '*'),
    make(MeasurementIcon::Celsius, MeasurementIconCategory::Measurement, "measurement-celsius", "celsius", 0xE339, 'T'),
    make(MeasurementIcon::Degrees, MeasurementIconCategory::Measurement, "measurement-degrees", "degrees", 0xE33E, '*'),
    make(MeasurementIcon::Fahrenheit, MeasurementIconCategory::Measurement, "measurement-fahrenheit", "fahrenheit",
         0xE341, 'T'),
    make(MeasurementIcon::Hot, MeasurementIconCategory::Measurement, "measurement-hot", "hot", 0xE36B, '*'),
    make(MeasurementIcon::Humidity, MeasurementIconCategory::Measurement, "measurement-humidity", "humidity", 0xE373,
         '*'),
    make(MeasurementIcon::Thermometer, MeasurementIconCategory::Measurement, "measurement-thermometer", "thermometer",
         0xE350, 'T'),
    make(MeasurementIcon::ThermometerExterior, MeasurementIconCategory::Measurement, "measurement-thermometer-exterior",
         "thermometer_exterior", 0xE34E, 'T'),
    make(MeasurementIcon::ThermometerInternal, MeasurementIconCategory::Measurement, "measurement-thermometer-internal",
         "thermometer_internal", 0xE34F, 'T'),
}};

static_assert(kDescriptors.size() == 9, "descriptor table out of sync");

const MeasurementIconDescriptor& descriptor_or_default(MeasurementIcon icon) {
    const auto index = static_cast<std::size_t>(icon);
    if (index < kDescriptors.size()) {
        return kDescriptors[index];
    }
    return kDescriptors[0];
}

} // namespace

const MeasurementIconDescriptor& measurement_icon_descriptor(MeasurementIcon icon) {
    return descriptor_or_default(icon);
}

MeasurementIconCategory measurement_icon_category(MeasurementIcon icon) {
    return measurement_icon_descriptor(icon).category;
}

const char* measurement_icon_path(MeasurementIcon icon) { return measurement_icon_descriptor(icon).path; }

const char* measurement_icon_nerd_suffix(MeasurementIcon icon) { return measurement_icon_descriptor(icon).nerd_suffix; }

const char* measurement_icon_category_path(MeasurementIconCategory category) {
    switch (category) {
    case MeasurementIconCategory::Measurement: return "measurement";
    }
    return "measurement";
}

const char* measurement_icon_category_label(MeasurementIconCategory category) {
    switch (category) {
    case MeasurementIconCategory::Measurement: return "Measurement";
    }
    return "Other";
}

std::vector<std::string_view> measurement_icon_path_segments(MeasurementIcon icon) {
    const char* path = measurement_icon_path(icon);
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

std::optional<MeasurementIcon> measurement_icon_from_path(std::string_view query) {
    for (const MeasurementIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.path) {
            return descriptor.kind;
        }
    }
    for (const MeasurementIconDescriptor& descriptor : kDescriptors) {
        const std::string legacy = std::string("measurement-") + descriptor.nerd_suffix;
        if (query == legacy) {
            return descriptor.kind;
        }
    }
    for (const MeasurementIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.nerd_suffix) {
            return descriptor.kind;
        }
    }
    return std::nullopt;
}

std::vector<MeasurementIcon> measurement_icons_in_category(MeasurementIconCategory category) {
    std::vector<MeasurementIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const MeasurementIconDescriptor& descriptor : kDescriptors) {
        if (descriptor.category == category) {
            icons.push_back(descriptor.kind);
        }
    }
    return icons;
}

std::vector<MeasurementIconCategory> all_measurement_icon_categories() {
    std::vector<MeasurementIconCategory> categories;
    categories.reserve(1);
    categories.push_back(MeasurementIconCategory::Measurement);
    return categories;
}

std::vector<MeasurementIcon> all_measurement_icons() {
    std::vector<MeasurementIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const MeasurementIconDescriptor& descriptor : kDescriptors) {
        icons.push_back(descriptor.kind);
    }
    return icons;
}

std::string measurement_icon_glyph(MeasurementIcon icon, GlyphSet glyphs) {
    const MeasurementIconDescriptor& descriptor = measurement_icon_descriptor(icon);
    if (glyphs == GlyphSet::Auto) {
        glyphs = detect_file_icon_glyph_set();
    }
    if (glyphs == GlyphSet::Ascii) {
        return std::string(1, descriptor.ascii_fallback);
    }
    return utf8_from(descriptor.codepoint);
}

} // namespace tuinator
