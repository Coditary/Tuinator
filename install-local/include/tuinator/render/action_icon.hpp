#pragma once

#include <tuinator/render/glyphs.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

enum class ActionIconCategory {
    Action,
};

/// 2 icons from Nerd Fonts weather set (Action, v3.4.0).
enum class ActionIcon {
    // Action
    Refresh,
    RefreshAlt,
};

struct ActionIconDescriptor {
    ActionIcon kind = ActionIcon::Refresh;
    ActionIconCategory category = ActionIconCategory::Action;
    const char* path = "";
    const char* nerd_suffix = "";
    char32_t codepoint = 0;
    char ascii_fallback = '?';
};

const ActionIconDescriptor& action_icon_descriptor(ActionIcon icon);
ActionIconCategory action_icon_category(ActionIcon icon);
const char* action_icon_path(ActionIcon icon);
const char* action_icon_nerd_suffix(ActionIcon icon);
const char* action_icon_category_path(ActionIconCategory category);
const char* action_icon_category_label(ActionIconCategory category);
std::vector<std::string_view> action_icon_path_segments(ActionIcon icon);
std::optional<ActionIcon> action_icon_from_path(std::string_view path);
std::vector<ActionIcon> action_icons_in_category(ActionIconCategory category);
std::vector<ActionIconCategory> all_action_icon_categories();
std::vector<ActionIcon> all_action_icons();
std::string action_icon_glyph(ActionIcon icon, GlyphSet glyphs = GlyphSet::Auto);

} // namespace tuinator
