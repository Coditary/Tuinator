#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/render/theme.hpp>

#include <nlohmann/json.hpp>

namespace tuinator::scene::style {

Style resolve_style(const nlohmann::json& value, const Theme& theme);

} // namespace tuinator::scene::style
