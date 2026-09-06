#pragma once

#include <tuinator/render/theme.hpp>
#include <tuinator/scene/build_result.hpp>
#include <tuinator/scene/handler_registry.hpp>

#include <filesystem>
#include <string>

namespace tuinator {

class Application;

namespace scene {

struct LoadOptions {
    HandlerRegistry* handlers = nullptr;
};

/// Load a .scene.json file and build the widget tree at runtime.
BuildResult load_and_build(const std::filesystem::path& path, Application& app, const Theme& theme,
                           const LoadOptions& options = {});

/// Load scene JSON from a string.
BuildResult load_and_build_from_string(const std::string& json_text, Application& app, const Theme& theme,
                                       const LoadOptions& options = {});

} // namespace scene
} // namespace tuinator
