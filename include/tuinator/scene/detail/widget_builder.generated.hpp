#pragma once

#include <memory>

#include <nlohmann/json_fwd.hpp>

#include <tuinator/scene/scene_context.hpp>

namespace tuinator {
class Widget;
}

namespace tuinator::scene::detail {

std::unique_ptr<Widget> build_widget(const nlohmann::json& node, SceneContext& ctx);

} // namespace tuinator::scene::detail
