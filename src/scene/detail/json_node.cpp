#include <tuinator/scene/detail/json_node.hpp>

#include <initializer_list>
#include <stdexcept>
#include <string>

namespace tuinator::scene::json {

namespace {

const nlohmann::json* find_key(const nlohmann::json& node, std::string_view key) {
    if (!node.is_object()) {
        return nullptr;
    }
    auto it = node.find(std::string(key));
    if (it == node.end() || it->is_null()) {
        return nullptr;
    }
    return &(*it);
}

template <typename... Keys>
const nlohmann::json* find_keys(const nlohmann::json& node, std::string_view first, Keys... rest) {
    if (const nlohmann::json* value = find_key(node, first)) {
        return value;
    }
    if constexpr (sizeof...(rest) > 0) {
        return find_keys(node, rest...);
    }
    return nullptr;
}

} // namespace

const nlohmann::json* find(const nlohmann::json& node, std::string_view key) { return find_key(node, key); }

const nlohmann::json* find(const nlohmann::json& node, std::string_view key1, std::string_view key2) {
    return find_keys(node, key1, key2);
}

const nlohmann::json* find(const nlohmann::json& node, std::string_view key1, std::string_view key2,
                           std::string_view key3) {
    return find_keys(node, key1, key2, key3);
}

const nlohmann::json* find(const nlohmann::json& node, std::string_view key1, std::string_view key2,
                           std::string_view key3, std::string_view key4) {
    return find_keys(node, key1, key2, key3, key4);
}

const nlohmann::json& sub(const nlohmann::json& node, std::string_view key) {
    if (const nlohmann::json* value = find_key(node, key)) {
        return *value;
    }
    static const nlohmann::json empty = nlohmann::json::object();
    return empty;
}

const nlohmann::json& sub(const nlohmann::json& node, std::string_view key1, std::string_view key2) {
    if (const nlohmann::json* value = find_keys(node, key1, key2)) {
        return *value;
    }
    static const nlohmann::json empty = nlohmann::json::object();
    return empty;
}

std::string as_string(const nlohmann::json& value, std::string_view fallback) {
    if (value.is_string()) {
        return value.get<std::string>();
    }
    if (value.is_number_integer()) {
        return std::to_string(value.get<int>());
    }
    if (value.is_number_float()) {
        return std::to_string(value.get<double>());
    }
    if (value.is_boolean()) {
        return value.get<bool>() ? "true" : "false";
    }
    return std::string(fallback);
}

int as_int(const nlohmann::json& value, int fallback) {
    if (value.is_number_integer()) {
        return value.get<int>();
    }
    if (value.is_number_float()) {
        return static_cast<int>(value.get<double>());
    }
    if (value.is_boolean()) {
        return value.get<bool>() ? 1 : 0;
    }
    return fallback;
}

bool as_bool(const nlohmann::json& value, bool fallback) {
    if (value.is_boolean()) {
        return value.get<bool>();
    }
    if (value.is_number()) {
        return value.get<double>() != 0.0;
    }
    return fallback;
}

double as_double(const nlohmann::json& value, double fallback) {
    if (value.is_number()) {
        return value.get<double>();
    }
    return fallback;
}

Rgb as_rgb(const nlohmann::json& value) {
    if (value.is_string()) {
        std::string hex = value.get<std::string>();
        if (!hex.empty() && hex.front() == '#') {
            hex.erase(hex.begin());
        }
        if (hex.size() == 6) {
            auto byte = [&](std::size_t offset) {
                return static_cast<int>(std::stoul(hex.substr(offset, 2), nullptr, 16));
            };
            return Rgb{static_cast<std::uint8_t>(byte(0)), static_cast<std::uint8_t>(byte(2)),
                       static_cast<std::uint8_t>(byte(4))};
        }
    }
    if (value.is_object()) {
        return Rgb{static_cast<std::uint8_t>(as_int(value.value("r", 0), 0)),
                   static_cast<std::uint8_t>(as_int(value.value("g", 0), 0)),
                   static_cast<std::uint8_t>(as_int(value.value("b", 0), 0))};
    }
    return Rgb{};
}

std::string node_string(const nlohmann::json& node, std::string_view key) {
    if (const nlohmann::json* value = find_key(node, key)) {
        return as_string(*value);
    }
    return {};
}

std::string node_string(const nlohmann::json& node, std::string_view key1, std::string_view key2) {
    if (const nlohmann::json* value = find_keys(node, key1, key2)) {
        return as_string(*value);
    }
    return {};
}

std::string node_string(const nlohmann::json& node, std::string_view key1, std::string_view key2,
                        std::string_view key3) {
    if (const nlohmann::json* value = find_keys(node, key1, key2, key3)) {
        return as_string(*value);
    }
    return {};
}

std::string node_string(const nlohmann::json& node, std::string_view key1, std::string_view key2, std::string_view key3,
                        std::string_view key4) {
    if (const nlohmann::json* value = find_keys(node, key1, key2, key3, key4)) {
        return as_string(*value);
    }
    return {};
}

std::string node_string_default(const nlohmann::json& node, std::string_view fallback, std::string_view key) {
    if (const nlohmann::json* value = find_key(node, key)) {
        return as_string(*value);
    }
    return std::string(fallback);
}

std::string node_string_default(const nlohmann::json& node, std::string_view fallback, std::string_view key1,
                                std::string_view key2) {
    if (const nlohmann::json* value = find_keys(node, key1, key2)) {
        return as_string(*value);
    }
    return std::string(fallback);
}

int node_int(const nlohmann::json& node, int fallback, std::string_view key) {
    if (const nlohmann::json* value = find_key(node, key)) {
        return as_int(*value, fallback);
    }
    return fallback;
}

int node_int(const nlohmann::json& node, int fallback, std::string_view key1, std::string_view key2) {
    if (const nlohmann::json* value = find_keys(node, key1, key2)) {
        return as_int(*value, fallback);
    }
    return fallback;
}

int node_int(const nlohmann::json& node, int fallback, std::string_view key1, std::string_view key2,
             std::string_view key3) {
    if (const nlohmann::json* value = find_keys(node, key1, key2, key3)) {
        return as_int(*value, fallback);
    }
    return fallback;
}

int node_int(const nlohmann::json& node, int fallback, std::string_view key1, std::string_view key2,
             std::string_view key3, std::string_view key4) {
    if (const nlohmann::json* value = find_keys(node, key1, key2, key3, key4)) {
        return as_int(*value, fallback);
    }
    return fallback;
}

bool node_bool(const nlohmann::json& node, bool fallback, std::string_view key) {
    if (const nlohmann::json* value = find_key(node, key)) {
        return as_bool(*value, fallback);
    }
    return fallback;
}

bool node_bool(const nlohmann::json& node, bool fallback, std::string_view key1, std::string_view key2) {
    if (const nlohmann::json* value = find_keys(node, key1, key2)) {
        return as_bool(*value, fallback);
    }
    return fallback;
}

double node_double(const nlohmann::json& node, double fallback, std::string_view key) {
    if (const nlohmann::json* value = find_key(node, key)) {
        return as_double(*value, fallback);
    }
    return fallback;
}

double node_double(const nlohmann::json& node, double fallback, std::string_view key1, std::string_view key2) {
    if (const nlohmann::json* value = find_keys(node, key1, key2)) {
        return as_double(*value, fallback);
    }
    return fallback;
}

std::uint32_t as_hex_color(const nlohmann::json& value) {
    if (value.is_string()) {
        std::string hex = value.get<std::string>();
        if (!hex.empty() && hex.front() == '#') {
            hex.erase(hex.begin());
        }
        if (hex.size() == 6) {
            return static_cast<std::uint32_t>(std::stoul(hex, nullptr, 16));
        }
        throw std::runtime_error("Invalid hex color: " + value.get<std::string>());
    }
    if (value.is_object()) {
        const Rgb rgb = as_rgb(value);
        return (static_cast<std::uint32_t>(rgb.r) << 16) | (static_cast<std::uint32_t>(rgb.g) << 8) |
               static_cast<std::uint32_t>(rgb.b);
    }
    throw std::runtime_error("Invalid gradient color value");
}

std::vector<ProgressBarGradientStop> build_progress_bar_gradient(const nlohmann::json& stops_json, const Theme&) {
    std::vector<ProgressBarGradientStop> stops;
    if (!stops_json.is_array()) {
        return stops;
    }
    for (const auto& stop : stops_json) {
        if (!stop.is_object()) {
            continue;
        }
        const float position = static_cast<float>(as_double(stop.value("position", 0.0), 0.0));
        if (const nlohmann::json* color = find_key(stop, "color")) {
            stops.push_back({position, Rgb::hex(as_hex_color(*color))});
        }
    }
    return stops;
}

std::vector<BigTextGradientStop> build_big_text_gradient(const nlohmann::json& stops_json, const Theme&) {
    std::vector<BigTextGradientStop> stops;
    if (!stops_json.is_array()) {
        return stops;
    }
    for (const auto& stop : stops_json) {
        if (!stop.is_object()) {
            continue;
        }
        const float position = static_cast<float>(as_double(stop.value("position", 0.0), 0.0));
        if (const nlohmann::json* color = find_key(stop, "color")) {
            stops.push_back({position, Rgb::hex(as_hex_color(*color))});
        }
    }
    return stops;
}

nlohmann::json merge_options_source(const nlohmann::json& node) {
    nlohmann::json merged = nlohmann::json::object();
    if (node.contains("options") && node["options"].is_object()) {
        merged.update(node["options"]);
    }
    static const char* skip[] = {"type",        "id",           "children",       "content",        "first",
                                 "second",      "tabs",         "windows",        "properties",     "background",
                                 "contextMenu", "context_menu", "commandPalette", "command_palette"};
    for (auto it = node.begin(); it != node.end(); ++it) {
        const std::string key = it.key();
        bool ignored = false;
        for (const char* name : skip) {
            if (key == name) {
                ignored = true;
                break;
            }
        }
        if (!ignored && !merged.contains(key)) {
            merged[key] = it.value();
        }
    }
    if (node.contains("properties") && node["properties"].is_object()) {
        for (auto it = node["properties"].begin(); it != node["properties"].end(); ++it) {
            if (!merged.contains(it.key())) {
                merged[it.key()] = it.value();
            }
        }
    }
    return merged;
}

bool has_any_option(const nlohmann::json& node, std::initializer_list<const char*> keys) {
    const nlohmann::json merged = merge_options_source(node);
    for (const char* key : keys) {
        if (find_key(merged, key) || find_key(node, key)) {
            return true;
        }
    }
    return false;
}

} // namespace tuinator::scene::json
