#include <tuinator/render/graphics_protocol.hpp>

#include <cstdlib>
#include <cstring>

namespace tuinator {

namespace {

GraphicsProtocol protocol_from_name(const char* name) {
    if (name == nullptr || name[0] == '\0') {
        return GraphicsProtocol::None;
    }

    if (std::strcmp(name, "auto") == 0) {
        return GraphicsProtocol::None;
    }
    if (std::strcmp(name, "kitty") == 0) {
        return GraphicsProtocol::Kitty;
    }
    if (std::strcmp(name, "iterm2") == 0 || std::strcmp(name, "iterm") == 0) {
        return GraphicsProtocol::Iterm2;
    }
    if (std::strcmp(name, "sixel") == 0) {
        return GraphicsProtocol::Sixel;
    }
    if (std::strcmp(name, "none") == 0) {
        return GraphicsProtocol::None;
    }

    return GraphicsProtocol::None;
}

bool term_contains(const char* haystack, const char* needle) {
    return haystack != nullptr && needle != nullptr && std::strstr(haystack, needle) != nullptr;
}

} // namespace

GraphicsProtocol detect_graphics_protocol() {
    const char* term_program = std::getenv("TERM_PROGRAM");
    if (term_program != nullptr) {
        if (std::strcmp(term_program, "ghostty") == 0
            || std::strcmp(term_program, "kitty") == 0
            || std::strcmp(term_program, "WezTerm") == 0) {
            return GraphicsProtocol::Kitty;
        }
        if (std::strcmp(term_program, "iTerm.app") == 0) {
            return GraphicsProtocol::Iterm2;
        }
    }

    const char* term = std::getenv("TERM");
    if (term_contains(term, "ghostty") || term_contains(term, "kitty") || term_contains(term, "wezterm")) {
        return GraphicsProtocol::Kitty;
    }

    const char* ghostty_resources = std::getenv("GHOSTTY_RESOURCES_DIR");
    if (ghostty_resources != nullptr && ghostty_resources[0] != '\0') {
        return GraphicsProtocol::Kitty;
    }

    if (term_contains(term, "sixel")) {
        return GraphicsProtocol::Sixel;
    }

    return GraphicsProtocol::None;
}

GraphicsProtocol active_graphics_protocol() {
    const char* override_value = std::getenv("TUINATOR_GRAPHICS");
    if (override_value != nullptr && override_value[0] != '\0' && std::strcmp(override_value, "auto") != 0) {
        const GraphicsProtocol forced = protocol_from_name(override_value);
        if (forced != GraphicsProtocol::None || std::strcmp(override_value, "none") == 0) {
            return forced;
        }
    }

    return detect_graphics_protocol();
}

std::string graphics_protocol_name(GraphicsProtocol protocol) {
    switch (protocol) {
    case GraphicsProtocol::Kitty:
        return "kitty";
    case GraphicsProtocol::Iterm2:
        return "iterm2";
    case GraphicsProtocol::Sixel:
        return "sixel";
    case GraphicsProtocol::None:
        return "none";
    }

    return "none";
}

} // namespace tuinator
