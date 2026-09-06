#include <tuinator/scene/detail/json_node.hpp>
#include <tuinator/scene/detail/style_resolve.hpp>

#include <stdexcept>
#include <unordered_map>

namespace tuinator::scene::style {

namespace {

const std::unordered_map<std::string, Color> k_colors = {
    {"Default", Color::Default}, {"Black", Color::Black},   {"Red", Color::Red},
    {"Green", Color::Green},     {"Yellow", Color::Yellow}, {"Blue", Color::Blue},
    {"Magenta", Color::Magenta}, {"Cyan", Color::Cyan},     {"White", Color::White},
};

Style style_from_color_name(const std::string& name) {
    const auto it = k_colors.find(name);
    if (it == k_colors.end()) {
        throw std::runtime_error("Unknown color name: " + name);
    }
    Style style;
    style.foreground = it->second;
    return style;
}

Style style_from_object(const nlohmann::json& object, const Theme& theme) {
    Style style;
    if (object.contains("foreground")) {
        const auto& fg = object["foreground"];
        if (fg.is_string()) {
            const std::string name = fg.get<std::string>();
            if (k_colors.count(name)) {
                style.foreground = k_colors.at(name);
            }
        } else if (fg.is_object()) {
            style.foreground_rgb = json::as_rgb(fg);
        }
    }
    if (object.contains("background")) {
        const auto& bg = object["background"];
        if (bg.is_string()) {
            const std::string name = bg.get<std::string>();
            if (k_colors.count(name)) {
                style.background = k_colors.at(name);
            }
        } else if (bg.is_object()) {
            style.background_rgb = json::as_rgb(bg);
        }
    }
    if (object.value("bold", false)) {
        style.bold = true;
    }
    if (object.value("dim", false)) {
        style.dim = true;
    }
    if (object.value("reverse", false)) {
        style.reverse = true;
    }
    return style;
}

const Style& theme_style(const Theme& theme, const std::string& name) {
    if (name == "heading")
        return theme.heading;
    if (name == "label")
        return theme.label;
    if (name == "muted")
        return theme.muted;
    if (name == "button")
        return theme.button;
    if (name == "button_focused")
        return theme.button_focused;
    if (name == "text_input")
        return theme.text_input;
    if (name == "text_input_focused")
        return theme.text_input_focused;
    if (name == "border")
        return theme.border;
    if (name == "accent")
        return theme.accent;
    if (name == "success")
        return theme.success;
    if (name == "danger")
        return theme.danger;
    throw std::runtime_error("Unknown style preset: " + name);
}

} // namespace

Style resolve_style(const nlohmann::json& value, const Theme& theme) {
    if (value.is_null() || (value.is_object() && value.empty())) {
        return Style{};
    }
    if (value.is_string()) {
        const std::string name = value.get<std::string>();
        if (k_colors.count(name)) {
            return style_from_color_name(name);
        }
        return theme_style(theme, name);
    }
    if (value.is_object()) {
        return style_from_object(value, theme);
    }
    return Style{};
}

} // namespace tuinator::scene::style
