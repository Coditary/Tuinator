#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/moon_icon.hpp>

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

constexpr MoonIconDescriptor make(MoonIcon kind, MoonIconCategory category, const char* path, const char* nerd_suffix,
                                  char32_t codepoint, char ascii) {
    return MoonIconDescriptor{kind, category, path, nerd_suffix, codepoint, ascii};
}

constexpr std::array<MoonIconDescriptor, 56> kDescriptors{{
    make(MoonIcon::MoonFirstQuarter, MoonIconCategory::Moon, "moon-first-quarter", "moon_first_quarter", 0xE394, 'n'),
    make(MoonIcon::MoonFull, MoonIconCategory::Moon, "moon-full", "moon_full", 0xE39B, 'n'),
    make(MoonIcon::MoonNew, MoonIconCategory::Moon, "moon-new", "moon_new", 0xE38D, 'n'),
    make(MoonIcon::MoonThirdQuarter, MoonIconCategory::Moon, "moon-third-quarter", "moon_third_quarter", 0xE3A2, 'n'),
    make(MoonIcon::MoonWaningCrescent1, MoonIconCategory::Moon, "moon-waning-crescent-1", "moon_waning_crescent_1",
         0xE3A3, 'n'),
    make(MoonIcon::MoonWaningCrescent2, MoonIconCategory::Moon, "moon-waning-crescent-2", "moon_waning_crescent_2",
         0xE3A4, 'n'),
    make(MoonIcon::MoonWaningCrescent3, MoonIconCategory::Moon, "moon-waning-crescent-3", "moon_waning_crescent_3",
         0xE3A5, 'n'),
    make(MoonIcon::MoonWaningCrescent4, MoonIconCategory::Moon, "moon-waning-crescent-4", "moon_waning_crescent_4",
         0xE3A6, 'n'),
    make(MoonIcon::MoonWaningCrescent5, MoonIconCategory::Moon, "moon-waning-crescent-5", "moon_waning_crescent_5",
         0xE3A7, 'n'),
    make(MoonIcon::MoonWaningCrescent6, MoonIconCategory::Moon, "moon-waning-crescent-6", "moon_waning_crescent_6",
         0xE3A8, 'n'),
    make(MoonIcon::MoonWaningGibbous1, MoonIconCategory::Moon, "moon-waning-gibbous-1", "moon_waning_gibbous_1", 0xE39C,
         'n'),
    make(MoonIcon::MoonWaningGibbous2, MoonIconCategory::Moon, "moon-waning-gibbous-2", "moon_waning_gibbous_2", 0xE39D,
         'n'),
    make(MoonIcon::MoonWaningGibbous3, MoonIconCategory::Moon, "moon-waning-gibbous-3", "moon_waning_gibbous_3", 0xE39E,
         'n'),
    make(MoonIcon::MoonWaningGibbous4, MoonIconCategory::Moon, "moon-waning-gibbous-4", "moon_waning_gibbous_4", 0xE39F,
         'n'),
    make(MoonIcon::MoonWaningGibbous5, MoonIconCategory::Moon, "moon-waning-gibbous-5", "moon_waning_gibbous_5", 0xE3A0,
         'n'),
    make(MoonIcon::MoonWaningGibbous6, MoonIconCategory::Moon, "moon-waning-gibbous-6", "moon_waning_gibbous_6", 0xE3A1,
         'n'),
    make(MoonIcon::MoonWaxingCrescent1, MoonIconCategory::Moon, "moon-waxing-crescent-1", "moon_waxing_crescent_1",
         0xE38E, 'n'),
    make(MoonIcon::MoonWaxingCrescent2, MoonIconCategory::Moon, "moon-waxing-crescent-2", "moon_waxing_crescent_2",
         0xE38F, 'n'),
    make(MoonIcon::MoonWaxingCrescent3, MoonIconCategory::Moon, "moon-waxing-crescent-3", "moon_waxing_crescent_3",
         0xE390, 'n'),
    make(MoonIcon::MoonWaxingCrescent4, MoonIconCategory::Moon, "moon-waxing-crescent-4", "moon_waxing_crescent_4",
         0xE391, 'n'),
    make(MoonIcon::MoonWaxingCrescent5, MoonIconCategory::Moon, "moon-waxing-crescent-5", "moon_waxing_crescent_5",
         0xE392, 'n'),
    make(MoonIcon::MoonWaxingCrescent6, MoonIconCategory::Moon, "moon-waxing-crescent-6", "moon_waxing_crescent_6",
         0xE393, 'n'),
    make(MoonIcon::MoonWaxingGibbous1, MoonIconCategory::Moon, "moon-waxing-gibbous-1", "moon_waxing_gibbous_1", 0xE395,
         'n'),
    make(MoonIcon::MoonWaxingGibbous2, MoonIconCategory::Moon, "moon-waxing-gibbous-2", "moon_waxing_gibbous_2", 0xE396,
         'n'),
    make(MoonIcon::MoonWaxingGibbous3, MoonIconCategory::Moon, "moon-waxing-gibbous-3", "moon_waxing_gibbous_3", 0xE397,
         'n'),
    make(MoonIcon::MoonWaxingGibbous4, MoonIconCategory::Moon, "moon-waxing-gibbous-4", "moon_waxing_gibbous_4", 0xE398,
         'n'),
    make(MoonIcon::MoonWaxingGibbous5, MoonIconCategory::Moon, "moon-waxing-gibbous-5", "moon_waxing_gibbous_5", 0xE399,
         'n'),
    make(MoonIcon::MoonWaxingGibbous6, MoonIconCategory::Moon, "moon-waxing-gibbous-6", "moon_waxing_gibbous_6", 0xE39A,
         'n'),
    make(MoonIcon::MoonAltFirstQuarter, MoonIconCategory::MoonAlt, "moon-first-quarter", "moon_alt_first_quarter",
         0xE3CE, 'n'),
    make(MoonIcon::MoonAltFull, MoonIconCategory::MoonAlt, "moon-full", "moon_alt_full", 0xE3D5, 'n'),
    make(MoonIcon::MoonAltNew, MoonIconCategory::MoonAlt, "moon-new", "moon_alt_new", 0xE3E3, 'n'),
    make(MoonIcon::MoonAltThirdQuarter, MoonIconCategory::MoonAlt, "moon-third-quarter", "moon_alt_third_quarter",
         0xE3DC, 'n'),
    make(MoonIcon::MoonAltWaningCrescent1, MoonIconCategory::MoonAlt, "moon-waning-crescent-1",
         "moon_alt_waning_crescent_1", 0xE3DD, 'n'),
    make(MoonIcon::MoonAltWaningCrescent2, MoonIconCategory::MoonAlt, "moon-waning-crescent-2",
         "moon_alt_waning_crescent_2", 0xE3DE, 'n'),
    make(MoonIcon::MoonAltWaningCrescent3, MoonIconCategory::MoonAlt, "moon-waning-crescent-3",
         "moon_alt_waning_crescent_3", 0xE3DF, 'n'),
    make(MoonIcon::MoonAltWaningCrescent4, MoonIconCategory::MoonAlt, "moon-waning-crescent-4",
         "moon_alt_waning_crescent_4", 0xE3E0, 'n'),
    make(MoonIcon::MoonAltWaningCrescent5, MoonIconCategory::MoonAlt, "moon-waning-crescent-5",
         "moon_alt_waning_crescent_5", 0xE3E1, 'n'),
    make(MoonIcon::MoonAltWaningCrescent6, MoonIconCategory::MoonAlt, "moon-waning-crescent-6",
         "moon_alt_waning_crescent_6", 0xE3E2, 'n'),
    make(MoonIcon::MoonAltWaningGibbous1, MoonIconCategory::MoonAlt, "moon-waning-gibbous-1",
         "moon_alt_waning_gibbous_1", 0xE3D6, 'n'),
    make(MoonIcon::MoonAltWaningGibbous2, MoonIconCategory::MoonAlt, "moon-waning-gibbous-2",
         "moon_alt_waning_gibbous_2", 0xE3D7, 'n'),
    make(MoonIcon::MoonAltWaningGibbous3, MoonIconCategory::MoonAlt, "moon-waning-gibbous-3",
         "moon_alt_waning_gibbous_3", 0xE3D8, 'n'),
    make(MoonIcon::MoonAltWaningGibbous4, MoonIconCategory::MoonAlt, "moon-waning-gibbous-4",
         "moon_alt_waning_gibbous_4", 0xE3D9, 'n'),
    make(MoonIcon::MoonAltWaningGibbous5, MoonIconCategory::MoonAlt, "moon-waning-gibbous-5",
         "moon_alt_waning_gibbous_5", 0xE3DA, 'n'),
    make(MoonIcon::MoonAltWaningGibbous6, MoonIconCategory::MoonAlt, "moon-waning-gibbous-6",
         "moon_alt_waning_gibbous_6", 0xE3DB, 'n'),
    make(MoonIcon::MoonAltWaxingCrescent1, MoonIconCategory::MoonAlt, "moon-waxing-crescent-1",
         "moon_alt_waxing_crescent_1", 0xE3C8, 'n'),
    make(MoonIcon::MoonAltWaxingCrescent2, MoonIconCategory::MoonAlt, "moon-waxing-crescent-2",
         "moon_alt_waxing_crescent_2", 0xE3C9, 'n'),
    make(MoonIcon::MoonAltWaxingCrescent3, MoonIconCategory::MoonAlt, "moon-waxing-crescent-3",
         "moon_alt_waxing_crescent_3", 0xE3CA, 'n'),
    make(MoonIcon::MoonAltWaxingCrescent4, MoonIconCategory::MoonAlt, "moon-waxing-crescent-4",
         "moon_alt_waxing_crescent_4", 0xE3CB, 'n'),
    make(MoonIcon::MoonAltWaxingCrescent5, MoonIconCategory::MoonAlt, "moon-waxing-crescent-5",
         "moon_alt_waxing_crescent_5", 0xE3CC, 'n'),
    make(MoonIcon::MoonAltWaxingCrescent6, MoonIconCategory::MoonAlt, "moon-waxing-crescent-6",
         "moon_alt_waxing_crescent_6", 0xE3CD, 'n'),
    make(MoonIcon::MoonAltWaxingGibbous1, MoonIconCategory::MoonAlt, "moon-waxing-gibbous-1",
         "moon_alt_waxing_gibbous_1", 0xE3CF, 'n'),
    make(MoonIcon::MoonAltWaxingGibbous2, MoonIconCategory::MoonAlt, "moon-waxing-gibbous-2",
         "moon_alt_waxing_gibbous_2", 0xE3D0, 'n'),
    make(MoonIcon::MoonAltWaxingGibbous3, MoonIconCategory::MoonAlt, "moon-waxing-gibbous-3",
         "moon_alt_waxing_gibbous_3", 0xE3D1, 'n'),
    make(MoonIcon::MoonAltWaxingGibbous4, MoonIconCategory::MoonAlt, "moon-waxing-gibbous-4",
         "moon_alt_waxing_gibbous_4", 0xE3D2, 'n'),
    make(MoonIcon::MoonAltWaxingGibbous5, MoonIconCategory::MoonAlt, "moon-waxing-gibbous-5",
         "moon_alt_waxing_gibbous_5", 0xE3D3, 'n'),
    make(MoonIcon::MoonAltWaxingGibbous6, MoonIconCategory::MoonAlt, "moon-waxing-gibbous-6",
         "moon_alt_waxing_gibbous_6", 0xE3D4, 'n'),
}};

static_assert(kDescriptors.size() == 56, "descriptor table out of sync");

const MoonIconDescriptor& descriptor_or_default(MoonIcon icon) {
    const auto index = static_cast<std::size_t>(icon);
    if (index < kDescriptors.size()) {
        return kDescriptors[index];
    }
    return kDescriptors[0];
}

} // namespace

const MoonIconDescriptor& moon_icon_descriptor(MoonIcon icon) { return descriptor_or_default(icon); }

MoonIconCategory moon_icon_category(MoonIcon icon) { return moon_icon_descriptor(icon).category; }

const char* moon_icon_path(MoonIcon icon) { return moon_icon_descriptor(icon).path; }

const char* moon_icon_nerd_suffix(MoonIcon icon) { return moon_icon_descriptor(icon).nerd_suffix; }

const char* moon_icon_category_path(MoonIconCategory category) {
    switch (category) {
    case MoonIconCategory::Moon: return "moon";
    case MoonIconCategory::MoonAlt: return "moon-alt";
    }
    return "moon";
}

const char* moon_icon_category_label(MoonIconCategory category) {
    switch (category) {
    case MoonIconCategory::Moon: return "Moon";
    case MoonIconCategory::MoonAlt: return "Moon Alt";
    }
    return "Other";
}

std::vector<std::string_view> moon_icon_path_segments(MoonIcon icon) {
    const char* path = moon_icon_path(icon);
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

std::optional<MoonIcon> moon_icon_from_path(std::string_view query) {
    for (const MoonIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.path) {
            return descriptor.kind;
        }
    }
    for (const MoonIconDescriptor& descriptor : kDescriptors) {
        const std::string legacy = std::string("moon-") + descriptor.nerd_suffix;
        if (query == legacy) {
            return descriptor.kind;
        }
    }
    for (const MoonIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.nerd_suffix) {
            return descriptor.kind;
        }
    }
    return std::nullopt;
}

std::vector<MoonIcon> moon_icons_in_category(MoonIconCategory category) {
    std::vector<MoonIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const MoonIconDescriptor& descriptor : kDescriptors) {
        if (descriptor.category == category) {
            icons.push_back(descriptor.kind);
        }
    }
    return icons;
}

std::vector<MoonIconCategory> all_moon_icon_categories() {
    std::vector<MoonIconCategory> categories;
    categories.reserve(2);
    categories.push_back(MoonIconCategory::Moon);
    categories.push_back(MoonIconCategory::MoonAlt);
    return categories;
}

std::vector<MoonIcon> all_moon_icons() {
    std::vector<MoonIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const MoonIconDescriptor& descriptor : kDescriptors) {
        icons.push_back(descriptor.kind);
    }
    return icons;
}

std::string moon_icon_glyph(MoonIcon icon, GlyphSet glyphs) {
    const MoonIconDescriptor& descriptor = moon_icon_descriptor(icon);
    if (glyphs == GlyphSet::Auto) {
        glyphs = detect_file_icon_glyph_set();
    }
    if (glyphs == GlyphSet::Ascii) {
        return std::string(1, descriptor.ascii_fallback);
    }
    return utf8_from(descriptor.codepoint);
}

} // namespace tuinator
