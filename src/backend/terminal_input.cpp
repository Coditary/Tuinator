#include "backend/terminal_input.hpp"

#include <tuinator/render/text.hpp>

#include <cctype>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace tuinator::detail {

namespace {

std::atomic<bool> g_resize_pending{false};

void on_sigwinch(int) { g_resize_pending = true; }

bool parse_int(const char* begin, const char* end, int& value) {
    if (begin >= end) {
        return false;
    }

    int parsed = 0;
    for (const char* p = begin; p < end; ++p) {
        if (!std::isdigit(static_cast<unsigned char>(*p))) {
            return false;
        }
        parsed = parsed * 10 + (*p - '0');
    }
    value = parsed;
    return true;
}

MouseButton mouse_button_from_code(int code) {
    const int base = code & 3;
    switch (base) {
    case 0: return MouseButton::Left;
    case 1: return MouseButton::Middle;
    case 2: return MouseButton::Right;
    default: return MouseButton::None;
    }
}

MouseAction mouse_action_from_code(int code, char final_byte) {
    // Bit 6 (64) marks wheel events; the low two bits select the direction.
    // Modifier bits (4/8/16) may be OR-ed in, so mask instead of comparing.
    if ((code & 64) != 0) {
        switch (code & 3) {
        case 0: return MouseAction::WheelUp;
        case 1: return MouseAction::WheelDown;
        case 2: return MouseAction::WheelLeft;
        default: return MouseAction::WheelRight;
        }
    }

    const int motion = code & 32;
    if (motion != 0) {
        return MouseAction::Move;
    }

    if (final_byte == 'M') {
        return MouseAction::Press;
    }
    if (final_byte == 'm') {
        return MouseAction::Release;
    }

    return MouseAction::Move;
}

} // namespace

void install_terminal_signal_handlers() {
    static bool installed = false;
    if (installed) {
        return;
    }

    std::signal(SIGWINCH, on_sigwinch);
    installed = true;
}

void TerminalInput::notify_resize() { g_resize_pending = true; }

bool TerminalInput::take_resize_pending() {
    return g_resize_pending.exchange(false);
}

void TerminalInput::reset() {
    state_ = State::Normal;
    buffer_.clear();
    paste_buffer_.clear();
    paste_end_match_ = 0;
    alt_pending_ = false;
    utf8_buffer_.clear();
    utf8_remaining_ = 0;
}

namespace {

constexpr char kBracketedPasteEnd[] = "\033[201~";

} // namespace

std::optional<Event> TerminalInput::feed_paste_byte(unsigned char byte) {
    if (paste_end_match_ > 0) {
        if (byte == static_cast<unsigned char>(kBracketedPasteEnd[paste_end_match_])) {
            ++paste_end_match_;
            if (paste_end_match_ == static_cast<int>(sizeof(kBracketedPasteEnd)) - 1) {
                ClipboardPaste paste{};
                paste.text = std::move(paste_buffer_);
                paste_buffer_.clear();
                paste_end_match_ = 0;
                state_ = State::Normal;
                return paste;
            }
            return std::nullopt;
        }

        paste_buffer_.append(kBracketedPasteEnd, static_cast<std::size_t>(paste_end_match_));
        paste_end_match_ = 0;
    }

    if (byte == 27) {
        paste_end_match_ = 1;
        return std::nullopt;
    }

    paste_buffer_.push_back(static_cast<char>(byte));
    return std::nullopt;
}

std::vector<Event> TerminalInput::feed_bytes(const unsigned char* data, std::size_t length) {
    std::vector<Event> events;
    for (std::size_t i = 0; i < length; ++i) {
        if (const std::optional<Event> event = feed(data[i])) {
            events.push_back(*event);
        }
    }
    return events;
}

bool TerminalInput::has_pending_escape() const { return state_ == State::Escape && utf8_remaining_ == 0; }

std::optional<Event> TerminalInput::flush_escape() {
    if (!has_pending_escape()) {
        return std::nullopt;
    }

    state_ = State::Normal;
    return KeyPress{Key::Escape, '\0'};
}

namespace {

void apply_xterm_modifier(KeyPress& press, int modifier) {
    if (modifier <= 1) {
        return;
    }

    press.shift = modifier == 2 || modifier == 4 || modifier == 6 || modifier == 8;
    press.alt = modifier == 3 || modifier == 4 || modifier == 7 || modifier == 8;
    press.ctrl = modifier == 5 || modifier == 6 || modifier == 7 || modifier == 8;
}

void apply_kitty_modifier(KeyPress& press, int modifier) {
    // Kitty encodes modifiers as 1 + bitmask (shift=1, alt=2, ctrl=4, ...).
    const int mask = modifier > 0 ? modifier - 1 : 0;
    press.shift = (mask & 1) != 0;
    press.alt = (mask & 2) != 0;
    press.ctrl = (mask & 4) != 0;
}

struct KittyKeyFields {
    int primary = 0;
    int modifier = 1;
    int event_type = 1;
    std::vector<int> key_codes;
    std::string text_utf8;
};

bool parse_first_int(std::string_view text, int& value) {
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    return parse_int(begin, end, value);
}

// Parses colon-separated sub-fields; empty sub-fields (e.g. "97::99" for a base
// layout key without shifted key) are kept as -1 instead of failing the parse.
bool parse_int_list(std::string_view text, std::vector<int>& values) {
    values.clear();
    if (text.empty()) {
        return false;
    }

    const char* begin = text.data();
    const char* end = text.data() + text.size();
    const char* segment = begin;
    for (const char* cursor = begin; cursor <= end; ++cursor) {
        if (cursor < end && *cursor != ':') {
            continue;
        }

        if (segment == cursor) {
            values.push_back(-1);
        } else {
            int value = 0;
            if (!parse_int(segment, cursor, value)) {
                return false;
            }
            values.push_back(value);
        }
        segment = cursor + 1;
    }

    return !values.empty();
}

int kitty_effective_codepoint(const KittyKeyFields& fields, bool shift) {
    if (fields.key_codes.empty()) {
        return fields.primary;
    }
    if (shift && fields.key_codes.size() >= 2 && fields.key_codes[1] > 0) {
        return fields.key_codes[1];
    }
    return fields.key_codes[0];
}

bool parse_modifier_event_field(std::string_view field, int& modifier, int& event_type) {
    if (field.empty()) {
        return true;
    }

    const std::size_t event_sep = field.find(':');
    if (event_sep == std::string_view::npos) {
        return parse_first_int(field, modifier);
    }

    const std::string_view mod_value = field.substr(0, event_sep);
    const std::string_view event_value = field.substr(event_sep + 1);
    if (!mod_value.empty() && !parse_first_int(mod_value, modifier)) {
        return false;
    }
    if (!event_value.empty() && !parse_first_int(event_value, event_type)) {
        return false;
    }
    return true;
}

bool split_semicolon_fields(std::string_view text, std::vector<std::string_view>& parts) {
    parts.clear();
    if (text.empty()) {
        return false;
    }

    const char* begin = text.data();
    const char* end = text.data() + text.size();
    const char* segment = begin;
    for (const char* cursor = begin; cursor <= end; ++cursor) {
        if (cursor < end && *cursor != ';') {
            continue;
        }
        parts.emplace_back(segment, static_cast<std::size_t>(cursor - segment));
        segment = cursor + 1;
    }
    return !parts.empty();
}

bool parse_kitty_key_fields(std::string_view text, KittyKeyFields& fields) {
    std::vector<std::string_view> parts;
    if (!split_semicolon_fields(text, parts)) {
        return false;
    }

    if (!parse_int_list(parts[0], fields.key_codes) || fields.key_codes[0] <= 0) {
        return false;
    }
    fields.primary = fields.key_codes[0];

    if (parts.size() >= 2) {
        if (!parse_modifier_event_field(parts[1], fields.modifier, fields.event_type)) {
            return false;
        }
    }

    if (parts.size() >= 3) {
        const std::string_view third = parts[2];
        if (fields.event_type == 1 && third.find(':') == std::string_view::npos) {
            int maybe_event = 0;
            if (parse_first_int(third, maybe_event) && (maybe_event == 2 || maybe_event == 3)) {
                fields.event_type = maybe_event;
                return true;
            }
        }

        // A malformed text field must not swallow the key; some terminals mis-encode it.
        std::vector<int> text_codes;
        if (parse_int_list(third, text_codes)) {
            std::string utf8;
            for (const int codepoint : text_codes) {
                if (codepoint > 0) {
                    utf8 += utf8_from_codepoint(static_cast<char32_t>(codepoint));
                }
            }
            fields.text_utf8 = std::move(utf8);
        }
    }

    return true;
}

// Kitty keyboard protocol functional keys live in the Unicode private use area.
constexpr int kKittyFunctionalBegin = 57344; // 0xE000
constexpr int kKittyFunctionalEnd = 63743;   // 0xF8FF

int utf8_sequence_length(unsigned char lead) {
    if (lead >= 0xF0) {
        return 4;
    }
    if (lead >= 0xE0) {
        return 3;
    }
    return 2;
}

// xterm modifyOtherKeys format: CSI 27 ; modifiers ; key ~
KeyPress modify_other_keys_press(int modifier, int keycode) {
    KeyPress press{};
    apply_xterm_modifier(press, modifier);

    switch (keycode) {
    case 8:
    case 127: press.key = Key::Backspace; return press;
    case 9:
        press.key = press.shift ? Key::BackTab : Key::Tab;
        return press;
    case 13: press.key = Key::Enter; return press;
    case 27: press.key = Key::Escape; return press;
    default: break;
    }

    if (keycode >= 32 && keycode <= 126) {
        char ch = static_cast<char>(keycode);
        if (press.ctrl && ch >= 'A' && ch <= 'Z') {
            ch = static_cast<char>(ch - 'A' + 'a');
        } else if (press.shift && ch >= 'a' && ch <= 'z') {
            ch = static_cast<char>(ch - 'a' + 'A');
        }
        press.character = ch;
        press.key = Key::Unknown;
        return press;
    }

    if (keycode > 126 && keycode <= 0x10FFFF && !(keycode >= kKittyFunctionalBegin && keycode <= kKittyFunctionalEnd)) {
        press.text = utf8_from_codepoint(static_cast<char32_t>(keycode));
        press.key = Key::Unknown;
        return press;
    }

    press.key = Key::Unknown;
    return press;
}

} // namespace

std::optional<KeyPress> TerminalInput::key_from_csi_params(char final_byte) const {
    if (!buffer_.empty()) {
        KittyKeyFields parsed{};
        if (parse_kitty_key_fields({buffer_.data(), buffer_.size()}, parsed) && parsed.event_type == 3) {
            // Key release events carry no information the widgets need.
            return std::nullopt;
        }
    }

    KeyPress press{};
    std::vector<int> params;

    const char* begin = buffer_.data();
    const char* end = buffer_.data() + buffer_.size();
    const char* segment = begin;

    for (const char* p = begin; p <= end; ++p) {
        if (p < end && *p != ';' && *p != ':') {
            continue;
        }

        int value = 0;
        if (parse_int(segment, p, value)) {
            params.push_back(value);
        }
        segment = p + 1;
    }

    const int primary = params.empty() ? 0 : params[0];
    const int modifier = params.size() >= 2 ? params[1] : 1;

    if (final_byte == 'u') {
        KittyKeyFields kitty{};
        if (!parse_kitty_key_fields({buffer_.data(), buffer_.size()}, kitty)) {
            return std::nullopt;
        }

        apply_kitty_modifier(press, kitty.modifier);

        if (!kitty.text_utf8.empty() && kitty.primary < kKittyFunctionalBegin &&
            keypress_text_is_printable(kitty.text_utf8)) {
            press.text = kitty.text_utf8;
            press.key = Key::Unknown;
            return press;
        }

        const int codepoint = kitty_effective_codepoint(kitty, press.shift);

        // Functional codepoints follow the kitty keyboard protocol table.
        switch (codepoint) {
        case 8:
        case 127:
        case 57347: press.key = Key::Backspace; return press;
        case 9:
        case 57346:
            press.key = press.shift ? Key::BackTab : Key::Tab;
            return press;
        case 13:
        case 57345:
        case 57414: press.key = Key::Enter; return press;
        case 27:
        case 57344: press.key = Key::Escape; return press;
        case 57348:
        case 57425: press.key = Key::Insert; return press;
        case 57349:
        case 57426: press.key = Key::Delete; return press;
        case 57350:
        case 57417: press.key = Key::Left; return press;
        case 57351:
        case 57418: press.key = Key::Right; return press;
        case 57352:
        case 57419: press.key = Key::Up; return press;
        case 57353:
        case 57420: press.key = Key::Down; return press;
        case 57354:
        case 57421: press.key = Key::PageUp; return press;
        case 57355:
        case 57422: press.key = Key::PageDown; return press;
        case 57356:
        case 57423: press.key = Key::Home; return press;
        case 57357:
        case 57424: press.key = Key::End; return press;
        default: break;
        }

        if (codepoint >= 57364 && codepoint <= 57375) {
            press.key = static_cast<Key>(static_cast<int>(Key::F1) + (codepoint - 57364));
            return press;
        }

        // Keypad digits/operators arrive with their own codes; users expect the characters.
        if (codepoint >= 57399 && codepoint <= 57408) {
            press.character = static_cast<char>('0' + (codepoint - 57399));
            press.key = Key::Unknown;
            return press;
        }
        switch (codepoint) {
        case 57409: press.character = '.'; return press;
        case 57410: press.character = '/'; return press;
        case 57411: press.character = '*'; return press;
        case 57412: press.character = '-'; return press;
        case 57413: press.character = '+'; return press;
        case 57415: press.character = '='; return press;
        default: break;
        }

        if (press.ctrl && codepoint >= 1 && codepoint <= 26) {
            press.character = static_cast<char>('a' + codepoint - 1);
            press.key = Key::Unknown;
            return press;
        }

        if (codepoint >= 32 && codepoint <= 126) {
            char ch = static_cast<char>(codepoint);
            if (press.ctrl && ch >= 'A' && ch <= 'Z') {
                ch = static_cast<char>(ch - 'A' + 'a');
            } else if (press.shift && ch >= 'a' && ch <= 'z' && kitty.key_codes.size() < 2) {
                ch = static_cast<char>(ch - 'a' + 'A');
            }
            press.character = ch;
            press.key = Key::Unknown;
            return press;
        }

        // Unmapped private-use codepoints are kitty functional keys (modifiers, lock keys,
        // media keys, F13+): swallow them instead of leaking glyphs into text fields.
        if (codepoint >= kKittyFunctionalBegin && codepoint <= kKittyFunctionalEnd) {
            return std::nullopt;
        }
        if (codepoint >= 0xD800 && codepoint <= 0xDFFF) {
            return std::nullopt;
        }

        if (codepoint > 126 && codepoint <= 0x10FFFF) {
            press.text = utf8_from_codepoint(static_cast<char32_t>(codepoint));
            press.key = Key::Unknown;
            return press;
        }

        return std::nullopt;
    }

    if (params.size() >= 2) {
        apply_xterm_modifier(press, modifier);
    }

    switch (final_byte) {
    case 'A': press.key = Key::Up; return press;
    case 'B': press.key = Key::Down; return press;
    case 'C': press.key = Key::Right; return press;
    case 'D': press.key = Key::Left; return press;
    case 'H': press.key = Key::Home; return press;
    case 'F': press.key = Key::End; return press;
    case 'Z':
        press.key = Key::BackTab;
        press.shift = true;
        return press;
    case 'P': press.key = Key::F1; return press;
    case 'Q': press.key = Key::F2; return press;
    case 'R': press.key = Key::F3; return press;
    case 'S': press.key = Key::F4; return press;
    case '~':
        switch (primary) {
        case 1:
        case 7: press.key = Key::Home; return press;
        case 2: press.key = Key::Insert; return press;
        case 3: press.key = Key::Delete; return press;
        case 4:
        case 8: press.key = Key::End; return press;
        case 5: press.key = Key::PageUp; return press;
        case 6: press.key = Key::PageDown; return press;
        case 11: press.key = Key::F1; return press;
        case 12: press.key = Key::F2; return press;
        case 13: press.key = Key::F3; return press;
        case 14: press.key = Key::F4; return press;
        case 15: press.key = Key::F5; return press;
        case 17: press.key = Key::F6; return press;
        case 18: press.key = Key::F7; return press;
        case 19: press.key = Key::F8; return press;
        case 20: press.key = Key::F9; return press;
        case 21: press.key = Key::F10; return press;
        case 23: press.key = Key::F11; return press;
        case 24: press.key = Key::F12; return press;
        case 27:
            if (params.size() >= 3) {
                return modify_other_keys_press(modifier, params[2]);
            }
            return std::nullopt;
        default: break;
        }
        break;
    default: break;
    }

    if (press.key != Key::Unknown) {
        return press;
    }
    return std::nullopt;
}

std::optional<Event> TerminalInput::finish_mouse_sgr(char final_byte) {
    if (!buffer_.empty() && (buffer_.back() == 'M' || buffer_.back() == 'm')) {
        buffer_.pop_back();
    }

    // Format: Cb;Cx;Cy with optional further fields.
    int button = 0;
    int x = 0;
    int y = 0;

    const char* begin = buffer_.data();
    const char* end = buffer_.data() + buffer_.size();
    const char* segment = begin;
    int field = 0;

    for (const char* p = begin; p <= end; ++p) {
        if (p < end && *p != ';') {
            continue;
        }

        int value = 0;
        if (!parse_int(segment, p, value)) {
            state_ = State::Normal;
            buffer_.clear();
            return std::nullopt;
        }

        switch (field) {
        case 0: button = value; break;
        case 1: x = value; break;
        case 2: y = value; break;
        default: break;
        }
        ++field;
        segment = p + 1;
    }

    state_ = State::Normal;
    buffer_.clear();

    MouseEvent event{};
    event.position = {x - 1, y - 1};
    event.button = mouse_button_from_code(button);
    event.action = mouse_action_from_code(button, final_byte);
    // Bit 5 marks motion events. Low bits are the active button; 3 means no button (hover in mode 1003).
    const bool motion = (button & 32) != 0;
    const int button_id = button & 3;
    if (motion) {
        event.left_pressed = button_id == 0;
    } else {
        event.left_pressed = button_id == 0 && event.action == MouseAction::Press;
    }
    return event;
}

std::optional<Event> TerminalInput::finish_csi() {
    if (buffer_.empty()) {
        state_ = State::Normal;
        return std::nullopt;
    }

    const char final_byte = buffer_.back();
    if (final_byte == 'M' || final_byte == 'm') {
        buffer_.pop_back();
        return finish_mouse_sgr(final_byte);
    }

    int primary = 0;
    {
        const char* begin = buffer_.data();
        const char* end = buffer_.data() + buffer_.size() - 1;
        const char* segment = begin;
        for (const char* p = begin; p <= end; ++p) {
            if (p < end && *p != ';' && *p != ':') {
                continue;
            }
            int value = 0;
            if (parse_int(segment, p, value)) {
                primary = value;
                break;
            }
            segment = p + 1;
        }
    }

    if (final_byte == '~' && primary == 200) {
        state_ = State::Paste;
        buffer_.clear();
        paste_buffer_.clear();
        paste_end_match_ = 0;
        return std::nullopt;
    }

    buffer_.pop_back();
    if (const std::optional<KeyPress> key = key_from_csi_params(final_byte)) {
        state_ = State::Normal;
        buffer_.clear();
        return *key;
    }

    state_ = State::Normal;
    buffer_.clear();
    return std::nullopt;
}

std::optional<Event> TerminalInput::feed(unsigned char byte) {
    if (state_ == State::Paste) {
        return feed_paste_byte(byte);
    }

    if (utf8_remaining_ > 0) {
        if ((byte & 0xC0) == 0x80) {
            utf8_buffer_.push_back(static_cast<char>(byte));
            --utf8_remaining_;
            if (utf8_remaining_ == 0) {
                KeyPress press{};
                press.key = Key::Unknown;
                press.text = utf8_buffer_;
                press.alt = alt_pending_;
                alt_pending_ = false;
                utf8_buffer_.clear();
                return press;
            }
            return std::nullopt;
        }

        // Truncated or invalid sequence: drop it and reprocess this byte normally.
        utf8_remaining_ = 0;
        utf8_buffer_.clear();
        alt_pending_ = false;
    }

    if (state_ == State::Escape) {
        if (byte == '[') {
            state_ = State::Csi;
            buffer_.clear();
            return std::nullopt;
        }

        if (byte == 'O') {
            state_ = State::Csi;
            buffer_.clear();
            return std::nullopt;
        }

        state_ = State::Normal;
        if (byte == 27) {
            return KeyPress{Key::Escape, '\0'};
        }

        if (byte >= 0xC2 && byte <= 0xF4) {
            // Alt + non-ASCII key: accumulate the UTF-8 sequence.
            alt_pending_ = true;
            utf8_buffer_.assign(1, static_cast<char>(byte));
            utf8_remaining_ = utf8_sequence_length(byte) - 1;
            return std::nullopt;
        }

        KeyPress alt_press{};
        alt_press.key = Key::Unknown;
        alt_press.alt = true;
        if (byte >= 32 && byte <= 126) {
            alt_press.character = static_cast<char>(byte);
        }
        return alt_press;
    }

    if (state_ == State::Csi) {
        if (byte == 27) {
            // A fresh escape aborts a partial sequence.
            state_ = State::Escape;
            buffer_.clear();
            return std::nullopt;
        }

        if (byte == '<') {
            state_ = State::MouseSgr;
            buffer_.clear();
            return std::nullopt;
        }

        buffer_.push_back(static_cast<char>(byte));
        if (byte >= 0x40 && byte <= 0x7E) {
            return finish_csi();
        }
        if (buffer_.size() > 64) {
            // Garbage without a final byte; do not grow unbounded.
            state_ = State::Normal;
            buffer_.clear();
        }
        return std::nullopt;
    }

    if (state_ == State::MouseSgr) {
        buffer_.push_back(static_cast<char>(byte));
        if (byte == 'M' || byte == 'm') {
            return finish_mouse_sgr(static_cast<char>(byte));
        }
        if (buffer_.size() > 32) {
            state_ = State::Normal;
            buffer_.clear();
        }
        return std::nullopt;
    }

    if (byte == 27) {
        state_ = State::Escape;
        return std::nullopt;
    }

    if (byte == '\n') {
        KeyPress press{};
        press.key = Key::Enter;
        press.shift = true;
        return press;
    }
    if (byte == '\r') {
        return KeyPress{Key::Enter, '\0'};
    }
    if (byte == 127 || byte == 8) {
        return KeyPress{Key::Backspace, '\0'};
    }
    if (byte == '\t') {
        return KeyPress{Key::Tab, '\0'};
    }
    if (byte >= 1 && byte <= 26) {
        KeyPress ctrl_press{};
        ctrl_press.ctrl = true;
        ctrl_press.character = static_cast<char>('a' + byte - 1);
        ctrl_press.key = Key::Unknown;
        return ctrl_press;
    }
    if (byte >= 32 && byte <= 126) {
        return KeyPress{Key::Unknown, static_cast<char>(byte)};
    }

    if (byte >= 0xC2 && byte <= 0xF4) {
        // Lead byte of a multi-byte UTF-8 scalar (umlauts, emoji, ...) on terminals
        // without the kitty keyboard protocol.
        utf8_buffer_.assign(1, static_cast<char>(byte));
        utf8_remaining_ = utf8_sequence_length(byte) - 1;
        return std::nullopt;
    }

    // Stray continuation bytes and C1 controls carry no meaning here.
    return std::nullopt;
}

} // namespace tuinator::detail
