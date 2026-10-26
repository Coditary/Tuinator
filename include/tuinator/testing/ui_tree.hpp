#pragma once

#include <tuinator/widgets/widget.hpp>

#include <optional>
#include <string>

namespace tuinator {

class MemoryTerminalBackend;

}

namespace tuinator::testing {

struct UiTreeOptions {
    bool include_screen_text = false;
    int max_screen_text_chars = 8000;
    std::optional<std::string> screen_text;
    /// Sample painted terminal colors inside each widget's bounds (needs render source).
    bool include_render_styles = false;
};

/// JSON description of the widget tree for agents and snapshot tests.
std::string build_ui_tree_json(const Widget& root, UiTreeOptions options = {},
                               const MemoryTerminalBackend* render = nullptr);

} // namespace tuinator::testing
