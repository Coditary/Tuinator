#pragma once

#include <tuinator/tuinator.hpp>
#include <memory>
#include <functional>
#include "../scene_handlers.hpp"

namespace tuinator {
namespace scene {
namespace form {

struct BuildResult {
    std::unique_ptr<tuinator::Widget> root;
    std::shared_ptr<tuinator::ActionRegistry> actions;
    tuinator::Desktop* desktop = nullptr;
    tuinator::WindowHost* window_host = nullptr;
};

BuildResult build(tuinator::Application& app, const tuinator::Theme& theme);

} // namespace
} // namespace
} // namespace
