#include "render_helper.hpp"
#include "test_harness.hpp"

#include <tuinator/core/event.hpp>
#include <tuinator/widgets/text_area.hpp>

#include <string>

namespace {

tuinator::KeyPress key_char(char ch) {
    tuinator::KeyPress press{};
    press.character = ch;
    return press;
}

tuinator::KeyPress key(tuinator::Key code) {
    tuinator::KeyPress press{};
    press.key = code;
    return press;
}

} // namespace

TUINATOR_TEST(text_area_roundtrip_value) {
    tuinator::TextArea area;
    area.set_value("hello\nworld");
    TUINATOR_CHECK_EQ(area.value(), std::string("hello\nworld"));
    TUINATOR_CHECK_EQ(area.line_count(), 2);
}

TUINATOR_TEST(text_area_inserts_and_splits_lines) {
    tuinator::TextArea area;
    area.set_focused(true);
    area.layout({0, 0, 40, 10});

    area.handle_event(key_char('a'));
    area.handle_event(key(tuinator::Key::Enter));
    area.handle_event(key_char('b'));

    TUINATOR_CHECK_EQ(area.value(), std::string("a\nb"));
    TUINATOR_CHECK_EQ(area.cursor_row(), 1);
    TUINATOR_CHECK_EQ(area.cursor_column(), 1);
}

TUINATOR_TEST(text_area_backspace_joins_lines) {
    tuinator::TextArea area;
    area.set_value("ab\ncd");
    area.set_focused(true);
    area.layout({0, 0, 40, 10});

    area.handle_event(key(tuinator::Key::Down));
    area.handle_event(key(tuinator::Key::Home));
    area.handle_event(key(tuinator::Key::Backspace));

    TUINATOR_CHECK_EQ(area.value(), std::string("abcd"));
    TUINATOR_CHECK_EQ(area.cursor_row(), 0);
    TUINATOR_CHECK_EQ(area.cursor_column(), 2);
}

TUINATOR_TEST(text_area_paints_line_numbers_and_status) {
    tuinator::MemoryTerminalBackend backend({40, 8});
    backend.init();

    tuinator::TextAreaOptions options;
    options.line_numbers = true;
    options.status_bar = true;
    options.title = "README.md";

    tuinator::TextArea area(options);
    area.set_value("# title\n\nbody");
    area.set_focused(true);
    tuinator::test::render_root(area, backend);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "# title"));
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 2, "body"));
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 7, "README.md"));
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 7, "(1,1)"));
}

TUINATOR_TEST(text_area_mouse_click_moves_cursor) {
    tuinator::TextAreaOptions options;
    options.line_numbers = true;
    options.status_bar = false;

    tuinator::TextArea area(options);
    area.set_value("hello\nworld");
    area.set_focused(true);
    area.layout({0, 0, 40, 6});

    tuinator::MouseEvent mouse{};
    mouse.action = tuinator::MouseAction::Click;
    mouse.position = {4, 1};
    area.handle_event(mouse);

    TUINATOR_CHECK_EQ(area.cursor_row(), 1);
    TUINATOR_CHECK_EQ(area.cursor_column(), 1);
}

TUINATOR_TEST(text_area_custom_gutter_paints_symbols) {
    tuinator::MemoryTerminalBackend backend({40, 6});
    backend.init();

    tuinator::TextAreaOptions options;
    options.line_numbers = true;
    options.status_bar = false;
    options.gutter_width = 4;
    options.gutter = [](const tuinator::GutterLine& line) {
        tuinator::GutterCell cell;
        cell.text = line.current ? ">1" : "*2";
        return cell;
    };

    tuinator::TextArea area(options);
    area.set_value("hello\nworld");
    area.set_focused(true);
    tuinator::test::render_root(area, backend);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, ">1"));
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 1, "*2"));
}

TUINATOR_TEST(text_area_relative_gutter_uses_cursor_distance) {
    tuinator::MemoryTerminalBackend backend({40, 6});
    backend.init();

    tuinator::TextAreaOptions options;
    options.line_numbers = true;
    options.status_bar = false;
    options.gutter = tuinator::relative_gutter;

    tuinator::TextArea area(options);
    area.set_value("a\nb\nc");
    area.set_focused(true);
    area.layout({0, 0, 40, 6});
    area.handle_event(key(tuinator::Key::Down));
    tuinator::test::render_root(area, backend);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "1"));
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 1, "0"));
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 2, "1"));
}
