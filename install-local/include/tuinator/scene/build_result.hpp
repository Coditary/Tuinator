#pragma once

#include <tuinator/core/action_registry.hpp>
#include <tuinator/widgets/widget.hpp>
#include <tuinator/window/desktop.hpp>
#include <tuinator/window/window_host.hpp>

#include <memory>
#include <string>
#include <unordered_map>

namespace tuinator::scene {

struct WidgetRef {
    std::string type;
    Widget* ptr = nullptr;
};

struct BuildResult {
    std::unique_ptr<Widget> root;
    std::shared_ptr<ActionRegistry> actions;
    Desktop* desktop = nullptr;
    WindowHost* window_host = nullptr;
    std::unordered_map<std::string, WidgetRef> widgets;
};

} // namespace tuinator::scene
