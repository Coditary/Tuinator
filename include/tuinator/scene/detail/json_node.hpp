#pragma once

#include <tuinator/render/color.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/display/big_text.hpp>
#include <tuinator/widgets/display/progress_bar.hpp>

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator::scene::json {

const nlohmann::json* find(const nlohmann::json& node, std::string_view key);
const nlohmann::json* find(const nlohmann::json& node, std::string_view key1, std::string_view key2);
const nlohmann::json* find(const nlohmann::json& node, std::string_view key1, std::string_view key2,
                           std::string_view key3);
const nlohmann::json* find(const nlohmann::json& node, std::string_view key1, std::string_view key2,
                           std::string_view key3, std::string_view key4);

const nlohmann::json& sub(const nlohmann::json& node, std::string_view key);
const nlohmann::json& sub(const nlohmann::json& node, std::string_view key1, std::string_view key2);

std::string as_string(const nlohmann::json& value, std::string_view fallback = "");
int as_int(const nlohmann::json& value, int fallback = 0);
bool as_bool(const nlohmann::json& value, bool fallback = false);
double as_double(const nlohmann::json& value, double fallback = 0.0);
Rgb as_rgb(const nlohmann::json& value);

std::string node_string(const nlohmann::json& node, std::string_view key);
std::string node_string(const nlohmann::json& node, std::string_view key1, std::string_view key2);
std::string node_string(const nlohmann::json& node, std::string_view key1, std::string_view key2,
                        std::string_view key3);
std::string node_string(const nlohmann::json& node, std::string_view key1, std::string_view key2, std::string_view key3,
                        std::string_view key4);
std::string node_string_default(const nlohmann::json& node, std::string_view fallback, std::string_view key);
std::string node_string_default(const nlohmann::json& node, std::string_view fallback, std::string_view key1,
                                std::string_view key2);

int node_int(const nlohmann::json& node, int fallback, std::string_view key);
int node_int(const nlohmann::json& node, int fallback, std::string_view key1, std::string_view key2);
int node_int(const nlohmann::json& node, int fallback, std::string_view key1, std::string_view key2,
             std::string_view key3);
int node_int(const nlohmann::json& node, int fallback, std::string_view key1, std::string_view key2,
             std::string_view key3, std::string_view key4);

bool node_bool(const nlohmann::json& node, bool fallback, std::string_view key);
bool node_bool(const nlohmann::json& node, bool fallback, std::string_view key1, std::string_view key2);

double node_double(const nlohmann::json& node, double fallback, std::string_view key);
double node_double(const nlohmann::json& node, double fallback, std::string_view key1, std::string_view key2);

std::uint32_t as_hex_color(const nlohmann::json& value);
std::vector<ProgressBarGradientStop> build_progress_bar_gradient(const nlohmann::json& stops, const Theme& theme);
std::vector<BigTextGradientStop> build_big_text_gradient(const nlohmann::json& stops, const Theme& theme);

nlohmann::json merge_options_source(const nlohmann::json& node);

bool has_any_option(const nlohmann::json& node, std::initializer_list<const char*> keys);

} // namespace tuinator::scene::json
