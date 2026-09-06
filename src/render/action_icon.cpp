#include <tuinator/render/action_icon.hpp>
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

constexpr ActionIconDescriptor make(ActionIcon kind, ActionIconCategory category, const char* path,
                                    const char* nerd_suffix, char32_t codepoint, char ascii) {
    return ActionIconDescriptor{kind, category, path, nerd_suffix, codepoint, ascii};
}

constexpr std::array<ActionIconDescriptor, 2> kDescriptors{{
    make(ActionIcon::Refresh, ActionIconCategory::Action, "action-refresh", "refresh", 0xE348, '*'),
    make(ActionIcon::RefreshAlt, ActionIconCategory::Action, "action-refresh-alt", "refresh_alt", 0xE347, '*'),
}};

static_assert(kDescriptors.size() == 2, "descriptor table out of sync");

const ActionIconDescriptor& descriptor_or_default(ActionIcon icon) {
    const auto index = static_cast<std::size_t>(icon);
    if (index < kDescriptors.size()) {
        return kDescriptors[index];
    }
    return kDescriptors[0];
}

} // namespace

const ActionIconDescriptor& action_icon_descriptor(ActionIcon icon) { return descriptor_or_default(icon); }

ActionIconCategory action_icon_category(ActionIcon icon) { return action_icon_descriptor(icon).category; }

const char* action_icon_path(ActionIcon icon) { return action_icon_descriptor(icon).path; }

const char* action_icon_nerd_suffix(ActionIcon icon) { return action_icon_descriptor(icon).nerd_suffix; }

const char* action_icon_category_path(ActionIconCategory category) {
    switch (category) {
    case ActionIconCategory::Action: return "action";
    }
    return "action";
}

const char* action_icon_category_label(ActionIconCategory category) {
    switch (category) {
    case ActionIconCategory::Action: return "Action";
    }
    return "Other";
}

std::vector<std::string_view> action_icon_path_segments(ActionIcon icon) {
    const char* path = action_icon_path(icon);
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

std::optional<ActionIcon> action_icon_from_path(std::string_view query) {
    for (const ActionIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.path) {
            return descriptor.kind;
        }
    }
    for (const ActionIconDescriptor& descriptor : kDescriptors) {
        const std::string legacy = std::string("action-") + descriptor.nerd_suffix;
        if (query == legacy) {
            return descriptor.kind;
        }
    }
    for (const ActionIconDescriptor& descriptor : kDescriptors) {
        if (query == descriptor.nerd_suffix) {
            return descriptor.kind;
        }
    }
    return std::nullopt;
}

std::vector<ActionIcon> action_icons_in_category(ActionIconCategory category) {
    std::vector<ActionIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const ActionIconDescriptor& descriptor : kDescriptors) {
        if (descriptor.category == category) {
            icons.push_back(descriptor.kind);
        }
    }
    return icons;
}

std::vector<ActionIconCategory> all_action_icon_categories() {
    std::vector<ActionIconCategory> categories;
    categories.reserve(1);
    categories.push_back(ActionIconCategory::Action);
    return categories;
}

std::vector<ActionIcon> all_action_icons() {
    std::vector<ActionIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const ActionIconDescriptor& descriptor : kDescriptors) {
        icons.push_back(descriptor.kind);
    }
    return icons;
}

std::string action_icon_glyph(ActionIcon icon, GlyphSet glyphs) {
    const ActionIconDescriptor& descriptor = action_icon_descriptor(icon);
    if (glyphs == GlyphSet::Auto) {
        glyphs = detect_file_icon_glyph_set();
    }
    if (glyphs == GlyphSet::Ascii) {
        return std::string(1, descriptor.ascii_fallback);
    }
    return utf8_from(descriptor.codepoint);
}

} // namespace tuinator
