#pragma once

#include <tuinator/core/geometry.hpp>

#include <cstdint>
#include <variant>

namespace tuinator {

enum class Key : std::uint16_t {
    None = 0,
    Escape,
    Enter,
    Tab,
    BackTab,
    Backspace,
    Delete,
    Up,
    Down,
    Left,
    Right,
    Home,
    End,
    PageUp,
    PageDown,
    Resize,
    Unknown,
};

struct KeyPress {
    Key key = Key::Unknown;
    char character = '\0';
    bool alt = false;
    bool ctrl = false;
};

struct Resize {
    int width = 0;
    int height = 0;
};

enum class MouseButton : std::uint8_t {
    None = 0,
    Left,
    Middle,
    Right,
};

enum class MouseAction : std::uint8_t {
    Press,
    Release,
    Click,
    Move,
    WheelUp,
    WheelDown,
    WheelLeft,
    WheelRight,
};

struct MouseEvent {
    Point position;
    MouseButton button = MouseButton::Left;
    MouseAction action = MouseAction::Press;
    bool left_pressed = false;
};

using Event = std::variant<KeyPress, Resize, MouseEvent>;

} // namespace tuinator
