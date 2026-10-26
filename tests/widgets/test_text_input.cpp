#include <tuinator/platform/clipboard.hpp>
#include <tuinator/widgets/controls/text_input.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(text_input_clears_inner_area) {
    tuinator::MemoryTerminalBackend backend({8, 1});
    backend.init();
    backend.draw_text(1, 0, "STALE", tuinator::Style{});

    tuinator::TextInput input({.min_width = 4});
    input.set_value("a");
    input.layout({0, 0, 8, 1});

    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    input.paint(ctx);

    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 1, 0), 'a');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 2, 0), ' ');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 4, 0), ' ');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 7, 0), ']');
}

TUINATOR_TEST(text_input_scrolls_to_show_cursor) {
    tuinator::MemoryTerminalBackend backend({10, 1});
    backend.init();

    tuinator::TextInput input({.min_width = 4});
    input.set_value("abcdefghij");
    input.set_focused(true);
    input.layout({0, 0, 10, 1});

    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    input.paint(ctx);

    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 1, 0), 'd');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 7, 0), 'j');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 8, 0), ' ');
    TUINATOR_CHECK(!tuinator::test::row_contains(backend, 0, "_"));
}

TUINATOR_TEST(text_input_accepts_text_beyond_min_width) {
    tuinator::TextInput input({.min_width = 4});
    input.layout({0, 0, 8, 1});
    input.set_focused(true);

    for (char ch = 'a'; ch <= 'h'; ++ch) {
        TUINATOR_CHECK(input.handle_event(tuinator::KeyPress{.character = ch}));
    }

    TUINATOR_CHECK_EQ(input.value(), "abcdefgh");
}

TUINATOR_TEST(text_input_password_masks_value) {
    tuinator::MemoryTerminalBackend backend({12, 1});
    backend.init();

    tuinator::TextInput input({.min_width = 8, .password = true});
    input.set_value("secret");
    input.layout({0, 0, 12, 1});

    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    input.paint(ctx);

    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 1, 0), '*');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 6, 0), '*');
    TUINATOR_CHECK(!tuinator::test::row_contains(backend, 0, "secret"));
    TUINATOR_CHECK_EQ(input.value(), "secret");
}

TUINATOR_TEST(text_input_ctrl_a_selects_all) {
    tuinator::TextInput input;
    input.set_value("hello");
    input.set_focused(true);

    TUINATOR_CHECK(input.handle_event(tuinator::KeyPress{.character = 'a', .ctrl = true}));
    TUINATOR_CHECK(input.handle_event(tuinator::KeyPress{.character = 'c', .ctrl = true}));
    TUINATOR_CHECK_EQ(tuinator::clipboard::get(), "hello");
}

TUINATOR_TEST(text_input_bracketed_paste_inserts_text) {
    tuinator::TextInput input;
    input.set_focused(true);

    TUINATOR_CHECK(input.handle_event(tuinator::ClipboardPaste{"world"}));
    TUINATOR_CHECK_EQ(input.value(), "world");
}

TUINATOR_TEST(text_input_ctrl_v_does_not_double_paste_with_bracketed_paste) {
    tuinator::TextInput input;
    input.set_focused(true);

    TUINATOR_CHECK(input.handle_event(tuinator::ClipboardPaste{"type to edit"}));
    TUINATOR_CHECK(input.handle_event(tuinator::KeyPress{.character = 'v', .ctrl = true}));
    TUINATOR_CHECK_EQ(input.value(), "type to edit");
}

TUINATOR_TEST(text_input_mouse_drag_selects_text) {
    tuinator::TextInput input;
    input.set_value("hello world");
    input.layout({2, 1, 15, 1});
    input.set_focused(true);

    const auto at = [](int x, int y) { return tuinator::Point{x, y}; };
    TUINATOR_CHECK(input.handle_event(
        tuinator::MouseEvent{at(3, 1), tuinator::MouseButton::Left, tuinator::MouseAction::Press, true}));
    TUINATOR_CHECK(input.handle_event(
        tuinator::MouseEvent{at(7, 1), tuinator::MouseButton::Left, tuinator::MouseAction::Move, true}));
    TUINATOR_CHECK(input.handle_event(
        tuinator::MouseEvent{at(7, 1), tuinator::MouseButton::Left, tuinator::MouseAction::Release, false}));

    tuinator::clipboard::set("");
    TUINATOR_CHECK(input.handle_event(tuinator::KeyPress{.character = 'c', .ctrl = true}));
    TUINATOR_CHECK_EQ(tuinator::clipboard::get(), "hello");
}

TUINATOR_TEST(text_input_inserts_unicode_text) {
    tuinator::TextInput input;
    input.set_focused(true);

    tuinator::KeyPress key{};
    key.text = "\xE2\x82\xAC";
    TUINATOR_CHECK(input.handle_event(key));
    TUINATOR_CHECK_EQ(input.value(), "\xE2\x82\xAC");
}

TUINATOR_TEST(text_input_inserts_unicode_with_altgr_modifiers) {
    tuinator::TextInput input;
    input.set_focused(true);

    tuinator::KeyPress key{};
    key.text = "\xE2\x82\xAC";
    key.alt = true;
    key.ctrl = true;
    TUINATOR_CHECK(input.handle_event(key));
    TUINATOR_CHECK_EQ(input.value(), "\xE2\x82\xAC");
}

TUINATOR_TEST(text_input_falls_back_to_character_when_text_is_control_chars) {
    tuinator::TextInput input;
    input.set_focused(true);

    tuinator::KeyPress key{};
    key.text = "\x01\x01";
    key.character = 'a';
    TUINATOR_CHECK(input.handle_event(key));
    TUINATOR_CHECK_EQ(input.value(), "a");
}

TUINATOR_TEST(text_input_mouse_click_without_drag_does_not_select) {
    tuinator::TextInput input;
    input.set_value("hello world");
    input.layout({2, 1, 15, 1});
    input.set_focused(true);

    const auto at = [](int x, int y) { return tuinator::Point{x, y}; };
    TUINATOR_CHECK(input.handle_event(
        tuinator::MouseEvent{at(3, 1), tuinator::MouseButton::Left, tuinator::MouseAction::Press, true}));
    TUINATOR_CHECK(input.handle_event(
        tuinator::MouseEvent{at(3, 1), tuinator::MouseButton::Left, tuinator::MouseAction::Release, false}));

    tuinator::clipboard::set("unchanged");
    TUINATOR_CHECK(!input.handle_event(tuinator::KeyPress{.character = 'c', .ctrl = true}));
    TUINATOR_CHECK_EQ(tuinator::clipboard::get(), "unchanged");
}

TUINATOR_TEST(text_input_mouse_press_release_selects_text_without_motion_events) {
    tuinator::TextInput input;
    input.set_value("hello world");
    input.layout({2, 1, 15, 1});
    input.set_focused(true);

    const auto at = [](int x, int y) { return tuinator::Point{x, y}; };
    TUINATOR_CHECK(input.handle_event(
        tuinator::MouseEvent{at(3, 1), tuinator::MouseButton::Left, tuinator::MouseAction::Press, true}));
    TUINATOR_CHECK(input.handle_event(
        tuinator::MouseEvent{at(9, 1), tuinator::MouseButton::Left, tuinator::MouseAction::Release, false}));

    tuinator::clipboard::set("");
    TUINATOR_CHECK(input.handle_event(tuinator::KeyPress{.character = 'c', .ctrl = true}));
    TUINATOR_CHECK_EQ(tuinator::clipboard::get(), "hello w");
}

TUINATOR_TEST(text_input_ctrl_delete_removes_next_word) {
    tuinator::TextInput input;
    input.set_value("one two three");
    input.set_focused(true);
    input.handle_event(tuinator::KeyPress{.key = tuinator::Key::Home});

    TUINATOR_CHECK(input.handle_event(tuinator::KeyPress{.key = tuinator::Key::Delete, .ctrl = true}));
    TUINATOR_CHECK_EQ(input.value(), " two three");
}

TUINATOR_TEST(text_input_ctrl_backspace_removes_previous_word) {
    tuinator::TextInput input;
    input.set_value("one two");
    input.set_focused(true);

    TUINATOR_CHECK(input.handle_event(tuinator::KeyPress{.key = tuinator::Key::Backspace, .ctrl = true}));
    TUINATOR_CHECK_EQ(input.value(), "one ");
}

TUINATOR_TEST(text_input_password_uses_block_cursor_at_end) {
    tuinator::MemoryTerminalBackend backend({10, 1});
    backend.init();

    tuinator::TextInput input({.min_width = 6, .password = true});
    input.set_value("ab");
    input.set_focused(true);
    input.layout({0, 0, 10, 1});

    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    input.paint(ctx);

    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 1, 0), '*');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 2, 0), '*');
    // End-of-field block cursor in password mode (docs: masked glyphs, no underline caret).
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 3, 0), '*');
    TUINATOR_CHECK(!tuinator::test::row_contains(backend, 0, "_"));
}

TUINATOR_TEST(text_input_escape_clears_selection) {
    tuinator::TextInput input;
    input.set_value("hello");
    input.set_focused(true);
    input.layout({0, 0, 10, 1});

    TUINATOR_CHECK(input.handle_event(tuinator::KeyPress{.character = 1}));
    TUINATOR_CHECK(input.handle_event(tuinator::KeyPress{.key = tuinator::Key::Escape}));
    TUINATOR_CHECK(!input.handle_event(tuinator::KeyPress{.key = tuinator::Key::Escape}));
}
