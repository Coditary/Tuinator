#include "backend/terminal_input.hpp"
#include "test_harness.hpp"

#include <tuinator/core/event.hpp>

#include <optional>
#include <string>
#include <variant>

namespace {

std::optional<tuinator::Event> feed(tuinator::detail::TerminalInput& input, const char* bytes) {
    for (const char* p = bytes; *p != '\0'; ++p) {
        if (const std::optional<tuinator::Event> event = input.feed(static_cast<unsigned char>(*p))) {
            return event;
        }
    }
    return std::nullopt;
}

} // namespace

TUINATOR_TEST(terminal_input_decodes_arrow_keys) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> up = feed(input, "\033[A");
    TUINATOR_CHECK(up.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*up);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->key == tuinator::Key::Up);
}

TUINATOR_TEST(terminal_input_decodes_delete_key) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> del = feed(input, "\033[3~");
    TUINATOR_CHECK(del.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*del);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->key == tuinator::Key::Delete);
}

TUINATOR_TEST(terminal_input_decodes_sgr_mouse_press) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[<0;12;5M");
    TUINATOR_CHECK(event.has_value());
    const auto* mouse = std::get_if<tuinator::MouseEvent>(&*event);
    TUINATOR_CHECK(mouse != nullptr);
    TUINATOR_CHECK(mouse->position.x == 11);
    TUINATOR_CHECK(mouse->position.y == 4);
    TUINATOR_CHECK(mouse->action == tuinator::MouseAction::Press);
    TUINATOR_CHECK(mouse->left_pressed);
}

TUINATOR_TEST(terminal_input_decodes_sgr_mouse_drag_motion) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[<32;20;5M");
    TUINATOR_CHECK(event.has_value());
    const auto* mouse = std::get_if<tuinator::MouseEvent>(&*event);
    TUINATOR_CHECK(mouse != nullptr);
    TUINATOR_CHECK(mouse->position.x == 19);
    TUINATOR_CHECK(mouse->position.y == 4);
    TUINATOR_CHECK(mouse->action == tuinator::MouseAction::Move);
    TUINATOR_CHECK(mouse->left_pressed);
}

TUINATOR_TEST(terminal_input_decodes_sgr_mouse_hover_without_button) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[<35;20;5M");
    TUINATOR_CHECK(event.has_value());
    const auto* mouse = std::get_if<tuinator::MouseEvent>(&*event);
    TUINATOR_CHECK(mouse != nullptr);
    TUINATOR_CHECK(mouse->action == tuinator::MouseAction::Move);
    TUINATOR_CHECK(!mouse->left_pressed);
}

TUINATOR_TEST(terminal_input_decodes_ctrl_a) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\001");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->ctrl);
    TUINATOR_CHECK_EQ(key->character, 'a');
}

TUINATOR_TEST(terminal_input_decodes_alt_delete) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[3;3~");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->key == tuinator::Key::Delete);
    TUINATOR_CHECK(key->alt);
}

TUINATOR_TEST(terminal_input_decodes_kitty_ctrl_v) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[118;5u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->ctrl);
    TUINATOR_CHECK_EQ(key->character, 'v');
}

TUINATOR_TEST(terminal_input_decodes_kitty_ctrl_v_control_code) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[22;5u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->ctrl);
    TUINATOR_CHECK_EQ(key->character, 'v');
}

TUINATOR_TEST(terminal_input_decodes_kitty_ctrl_delete) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[57349;5u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->key == tuinator::Key::Delete);
    TUINATOR_CHECK(key->ctrl);
}

TUINATOR_TEST(terminal_input_decodes_ctrl_delete) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[3;5~");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->key == tuinator::Key::Delete);
    TUINATOR_CHECK(key->ctrl);
}

TUINATOR_TEST(terminal_input_decodes_shift_right) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[1;2C");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->key == tuinator::Key::Right);
    TUINATOR_CHECK(key->shift);
}

TUINATOR_TEST(terminal_input_decodes_bracketed_paste) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[200~between fields\033[201~");
    TUINATOR_CHECK(event.has_value());
    const auto* paste = std::get_if<tuinator::ClipboardPaste>(&*event);
    TUINATOR_CHECK(paste != nullptr);
    TUINATOR_CHECK_EQ(paste->text, "between fields");
}

TUINATOR_TEST(terminal_input_decodes_kitty_u_with_alternate_ctrl_code) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[117:21;1:1u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(!key->ctrl);
    TUINATOR_CHECK_EQ(key->character, 'u');
}

TUINATOR_TEST(terminal_input_decodes_kitty_w_with_alternate_ctrl_code) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[119:23;1:1u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(!key->ctrl);
    TUINATOR_CHECK_EQ(key->character, 'w');
}

TUINATOR_TEST(terminal_input_decodes_kitty_shift_slash_alternate) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[55:47;2:1u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->shift);
    TUINATOR_CHECK_EQ(key->character, '/');
}

TUINATOR_TEST(terminal_input_decodes_kitty_shift_colon_alternate) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[46:58;2:1u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->shift);
    TUINATOR_CHECK_EQ(key->character, ':');
}

TUINATOR_TEST(terminal_input_decodes_kitty_euro_codepoint) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[8364;1:1u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK_EQ(key->text, "\xE2\x82\xAC");
}

TUINATOR_TEST(terminal_input_decodes_kitty_euro_with_altgr) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[8364;7:1u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->alt);
    TUINATOR_CHECK(key->ctrl);
    TUINATOR_CHECK_EQ(key->text, "\xE2\x82\xAC");
}

TUINATOR_TEST(terminal_input_decodes_kitty_associated_text_field) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[8364;1:1;8364u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK_EQ(key->text, "\xE2\x82\xAC");
}

TUINATOR_TEST(terminal_input_decodes_kitty_associated_text_plain_letter) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[97;1;97u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK_EQ(key->text, "a");
}

TUINATOR_TEST(terminal_input_ignores_associated_text_control_chars) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[97;1:1;1:1u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->text.empty());
    TUINATOR_CHECK_EQ(key->character, 'a');
}

TUINATOR_TEST(terminal_input_decodes_kitty_unshifted_slash) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[47;1:1u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(!key->shift);
    TUINATOR_CHECK_EQ(key->character, '/');
}

TUINATOR_TEST(terminal_input_decodes_kitty_shift_uppercase_letter) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[97;2:1u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->shift);
    TUINATOR_CHECK_EQ(key->character, 'A');
}

TUINATOR_TEST(terminal_input_decodes_kitty_shift_uppercase_codepoint) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[65;2:1u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->shift);
    TUINATOR_CHECK_EQ(key->character, 'A');
}

TUINATOR_TEST(terminal_input_decodes_kitty_ctrl_u_from_modifier) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[117:21;5:1u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->ctrl);
    TUINATOR_CHECK_EQ(key->character, 'u');
}

TUINATOR_TEST(terminal_input_ignores_kitty_key_release) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> press = feed(input, "h");
    TUINATOR_CHECK(press.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*press);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK_EQ(key->character, 'h');

    const std::optional<tuinator::Event> release = feed(input, "\033[104;1:3u");
    TUINATOR_CHECK(!release.has_value());
}

TUINATOR_TEST(terminal_input_decodes_kitty_arrow_press_not_release) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> press = feed(input, "\033[57352;1:1u");
    TUINATOR_CHECK(press.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*press);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->key == tuinator::Key::Up);

    const std::optional<tuinator::Event> release = feed(input, "\033[57352;1:3u");
    TUINATOR_CHECK(!release.has_value());
}

TUINATOR_TEST(terminal_input_ignores_kitty_arrow_release_with_semicolon_event_type) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> release = feed(input, "\033[57352;1;3u");
    TUINATOR_CHECK(!release.has_value());
}

TUINATOR_TEST(terminal_input_ignores_kitty_legacy_trailer_arrow_release) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> release = feed(input, "\033[1;1:3A");
    TUINATOR_CHECK(!release.has_value());
}

TUINATOR_TEST(terminal_input_decodes_printable_character) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "x");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->character == 'x');
}

TUINATOR_TEST(terminal_input_swallows_kitty_shift_press) {
    tuinator::detail::TerminalInput input;

    // Left shift press/release must not leak private-use glyphs into text fields.
    TUINATOR_CHECK(!feed(input, "\033[57441;2:1u").has_value());
    TUINATOR_CHECK(!feed(input, "\033[57441;1:3u").has_value());
}

TUINATOR_TEST(terminal_input_swallows_kitty_modifier_and_lock_keys) {
    tuinator::detail::TerminalInput input;

    TUINATOR_CHECK(!feed(input, "\033[57442;5:1u").has_value()); // left ctrl
    TUINATOR_CHECK(!feed(input, "\033[57443;3:1u").has_value()); // left alt
    TUINATOR_CHECK(!feed(input, "\033[57447;2:1u").has_value()); // right shift
    TUINATOR_CHECK(!feed(input, "\033[57358;1:1u").has_value()); // caps lock
    TUINATOR_CHECK(!feed(input, "\033[57360;1:1u").has_value()); // num lock
    TUINATOR_CHECK(!feed(input, "\033[57428;1:1u").has_value()); // media play
}

TUINATOR_TEST(terminal_input_decodes_kitty_functional_keys) {
    tuinator::detail::TerminalInput input;

    const struct {
        const char* sequence;
        tuinator::Key expected;
    } cases[] = {
        {"\033[57345;1:1u", tuinator::Key::Enter},  {"\033[57346;1:1u", tuinator::Key::Tab},
        {"\033[57347;1:1u", tuinator::Key::Backspace}, {"\033[57348;1:1u", tuinator::Key::Insert},
        {"\033[57349;1:1u", tuinator::Key::Delete}, {"\033[57350;1:1u", tuinator::Key::Left},
        {"\033[57351;1:1u", tuinator::Key::Right},  {"\033[57353;1:1u", tuinator::Key::Down},
        {"\033[57354;1:1u", tuinator::Key::PageUp}, {"\033[57355;1:1u", tuinator::Key::PageDown},
        {"\033[57356;1:1u", tuinator::Key::Home},   {"\033[57357;1:1u", tuinator::Key::End},
    };

    for (const auto& test_case : cases) {
        const std::optional<tuinator::Event> event = feed(input, test_case.sequence);
        TUINATOR_CHECK(event.has_value());
        const auto* key = std::get_if<tuinator::KeyPress>(&*event);
        TUINATOR_CHECK(key != nullptr);
        TUINATOR_CHECK(key->key == test_case.expected);
    }
}

TUINATOR_TEST(terminal_input_decodes_kitty_f_keys) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> f1 = feed(input, "\033[57364;1:1u");
    TUINATOR_CHECK(f1.has_value());
    TUINATOR_CHECK(std::get_if<tuinator::KeyPress>(&*f1)->key == tuinator::Key::F1);

    const std::optional<tuinator::Event> f12 = feed(input, "\033[57375;1:1u");
    TUINATOR_CHECK(f12.has_value());
    TUINATOR_CHECK(std::get_if<tuinator::KeyPress>(&*f12)->key == tuinator::Key::F12);
}

TUINATOR_TEST(terminal_input_decodes_kitty_keypad_digits_as_text) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[57399;1:1u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->key == tuinator::Key::Unknown);
    TUINATOR_CHECK_EQ(key->character, '0');
}

TUINATOR_TEST(terminal_input_decodes_digit_three_as_text_not_delete) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[51;1:1u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->key == tuinator::Key::Unknown);
    TUINATOR_CHECK_EQ(key->character, '3');
}

TUINATOR_TEST(terminal_input_decodes_kitty_shift_tab_as_backtab) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[9;2u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->key == tuinator::Key::BackTab);
}

TUINATOR_TEST(terminal_input_decodes_csi_z_as_backtab) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[Z");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->key == tuinator::Key::BackTab);
}

TUINATOR_TEST(terminal_input_decodes_modify_other_keys_shift_a) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[27;2;65~");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->shift);
    TUINATOR_CHECK_EQ(key->character, 'A');
}

TUINATOR_TEST(terminal_input_decodes_modify_other_keys_alt_e) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[27;3;101~");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->alt);
    TUINATOR_CHECK_EQ(key->character, 'e');
}

TUINATOR_TEST(terminal_input_decodes_utf8_multibyte_characters) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> umlaut = feed(input, "\xC3\xA4");
    TUINATOR_CHECK(umlaut.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*umlaut);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK_EQ(key->text, "\xC3\xA4");

    const std::optional<tuinator::Event> emoji = feed(input, "\xF0\x9F\x98\x80");
    TUINATOR_CHECK(emoji.has_value());
    TUINATOR_CHECK_EQ(std::get_if<tuinator::KeyPress>(&*emoji)->text, "\xF0\x9F\x98\x80");
}

TUINATOR_TEST(terminal_input_decodes_alt_utf8_character) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033\xC3\xA4");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->alt);
    TUINATOR_CHECK_EQ(key->text, "\xC3\xA4");
}

TUINATOR_TEST(terminal_input_recovers_from_truncated_utf8) {
    tuinator::detail::TerminalInput input;

    // Lead byte without continuation, then a plain ASCII key.
    const std::optional<tuinator::Event> event = feed(input, "\xC3""x");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK_EQ(key->character, 'x');
}

TUINATOR_TEST(terminal_input_decodes_wheel_left_right) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> left = feed(input, "\033[<66;10;5M");
    TUINATOR_CHECK(left.has_value());
    TUINATOR_CHECK(std::get_if<tuinator::MouseEvent>(&*left)->action == tuinator::MouseAction::WheelLeft);

    const std::optional<tuinator::Event> right = feed(input, "\033[<67;10;5M");
    TUINATOR_CHECK(right.has_value());
    TUINATOR_CHECK(std::get_if<tuinator::MouseEvent>(&*right)->action == tuinator::MouseAction::WheelRight);
}

TUINATOR_TEST(terminal_input_decodes_wheel_with_modifiers) {
    tuinator::detail::TerminalInput input;

    const std::optional<tuinator::Event> event = feed(input, "\033[<68;10;5M");
    TUINATOR_CHECK(event.has_value());
    const auto* mouse = std::get_if<tuinator::MouseEvent>(&*event);
    TUINATOR_CHECK(mouse != nullptr);
    TUINATOR_CHECK(mouse->action == tuinator::MouseAction::WheelUp);
}

TUINATOR_TEST(terminal_input_flushes_pending_escape) {
    tuinator::detail::TerminalInput input;

    TUINATOR_CHECK(!feed(input, "\033").has_value());
    TUINATOR_CHECK(input.has_pending_escape());

    const std::optional<tuinator::Event> escape = input.flush_escape();
    TUINATOR_CHECK(escape.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*escape);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->key == tuinator::Key::Escape);
    TUINATOR_CHECK(!input.has_pending_escape());
}

TUINATOR_TEST(terminal_input_feed_bytes_returns_all_events) {
    tuinator::detail::TerminalInput input;

    const char* text = "abc";
    const std::vector<tuinator::Event> events =
        input.feed_bytes(reinterpret_cast<const unsigned char*>(text), 3);
    TUINATOR_CHECK_EQ(events.size(), 3);
    for (std::size_t i = 0; i < events.size(); ++i) {
        const auto* key = std::get_if<tuinator::KeyPress>(&events[i]);
        TUINATOR_CHECK(key != nullptr);
        TUINATOR_CHECK_EQ(key->character, static_cast<char>('a' + i));
    }
}

TUINATOR_TEST(terminal_input_decodes_kitty_altgr_text_field) {
    tuinator::detail::TerminalInput input;

    // AltGr+e on a German layout: base key 'e', produced text €.
    const std::optional<tuinator::Event> event = feed(input, "\033[101;3:1;8364u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK_EQ(key->text, "\xE2\x82\xAC");
    TUINATOR_CHECK(tuinator::allows_text_insert_modifiers(*key));
}

TUINATOR_TEST(terminal_input_tolerates_empty_alternate_key_subfield) {
    tuinator::detail::TerminalInput input;

    // Base layout key without shifted key: CSI 97::99 ... u
    const std::optional<tuinator::Event> event = feed(input, "\033[97::99;1:1u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK_EQ(key->character, 'a');
}

TUINATOR_TEST(terminal_input_decodes_kitty_escape_key) {
    tuinator::detail::TerminalInput input;

    // With the disambiguate flag, the Esc key arrives as CSI 27u instead of a lone ESC byte.
    const std::optional<tuinator::Event> event = feed(input, "\033[27u");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK(key->key == tuinator::Key::Escape);
    TUINATOR_CHECK(!input.has_pending_escape());
}

TUINATOR_TEST(terminal_input_decodes_dead_key_composition_as_utf8) {
    tuinator::detail::TerminalInput input;

    // Without kitty flag 8 the terminal composes dead keys itself: the dead key press sends
    // nothing, the composed character arrives as plain UTF-8 (dead_grave + a -> à).
    const std::optional<tuinator::Event> composed = feed(input, "\xC3\xA0");
    TUINATOR_CHECK(composed.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*composed);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK_EQ(key->text, "\xC3\xA0");
    TUINATOR_CHECK(tuinator::allows_text_insert_modifiers(*key));
}

TUINATOR_TEST(terminal_input_decodes_standalone_backtick) {
    tuinator::detail::TerminalInput input;

    // dead_grave + space composes to a plain backtick byte on German layouts.
    const std::optional<tuinator::Event> event = feed(input, "`");
    TUINATOR_CHECK(event.has_value());
    const auto* key = std::get_if<tuinator::KeyPress>(&*event);
    TUINATOR_CHECK(key != nullptr);
    TUINATOR_CHECK_EQ(key->character, '`');
    TUINATOR_CHECK(tuinator::allows_text_insert_modifiers(*key));
}
