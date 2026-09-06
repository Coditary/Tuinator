#pragma once

#include <tuinator/scene/scene_context.hpp>

#include <nlohmann/json.hpp>

namespace tuinator::scene::detail {

void apply_bindings(const nlohmann::json& bindings, SceneContext& ctx);

} // namespace tuinator::scene::detail
