#pragma once

#include <tuinator/core/geometry.hpp>

#include <cstdint>
#include <string>
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
    Insert,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
};

struct KeyPress {
    Key key = Key::Unknown;
    char character = '\0';
    std::string text;
    bool alt = false;
    bool ctrl = false;
    bool shift = false;
};

inline bool is_altgr(const KeyPress& key) { return key.alt && key.ctrl && !key.shift; }

inline bool keypress_text_is_printable(std::string_view text) {
    for (std::size_t index = 0; index < text.size(); ++index) {
        if (static_cast<unsigned char>(text[index]) >= 32) {
            return true;
        }
    }
    return false;
}

inline bool allows_text_insert_modifiers(const KeyPress& key) {
    // A terminal that reports the produced text explicitly (kitty keyboard text field)
    // has already resolved AltGr/IME composition; trust it regardless of modifiers.
    if (!key.text.empty() && keypress_text_is_printable(key.text)) {
        return true;
    }
    if (!key.alt && !key.ctrl) {
        return true;
    }
    return is_altgr(key);
}

inline std::string keypress_insert_text(const KeyPress& key) {
    if (!key.text.empty() && keypress_text_is_printable(key.text)) {
        return key.text;
    }
    if (key.character >= 32 && key.character != '\0') {
        return std::string(1, key.character);
    }
    return {};
}

/// Returns `'a'`..`'z'` for Ctrl+letter shortcuts, or `'\0'`.
inline char ctrl_letter(const KeyPress& key) {
    if (key.ctrl && key.character >= 'a' && key.character <= 'z') {
        return key.character;
    }
    if (key.character >= 1 && key.character <= 26) {
        return static_cast<char>('a' + key.character - 1);
    }
    return '\0';
}

inline bool is_ctrl_copy(const KeyPress& key) { return ctrl_letter(key) == 'c'; }

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

struct ClipboardPaste {
    std::string text;
};

using Event = std::variant<KeyPress, Resize, MouseEvent, ClipboardPaste>;

} // namespace tuinator
