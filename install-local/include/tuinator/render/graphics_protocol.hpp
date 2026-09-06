#pragma once

#include <string>

namespace tuinator {

enum class GraphicsProtocol {
    None,
    Kitty,
    Iterm2,
    Sixel,
};

/// Detect the best graphics protocol for the current terminal.
GraphicsProtocol detect_graphics_protocol();

/// Like detect_graphics_protocol(), but honors `TUINATOR_GRAPHICS` (kitty|iterm2|sixel|none|auto).
GraphicsProtocol active_graphics_protocol();

std::string graphics_protocol_name(GraphicsProtocol protocol);

} // namespace tuinator
