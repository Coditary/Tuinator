#include <tuinator/core/event.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/display/spinner.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(spinner_up_and_down_change_value) {
    tuinator::Spinner spinner(0, 10, 5, 1, {}, tuinator::Theme{}.label);
    spinner.layout({0, 0, 12, 1});
    spinner.set_focused(true);

    TUINATOR_CHECK(spinner.handle_event(tuinator::KeyPress{tuinator::Key::Up}));
    TUINATOR_CHECK_EQ(spinner.value(), 6);

    TUINATOR_CHECK(spinner.handle_event(tuinator::KeyPress{tuinator::Key::Down}));
    TUINATOR_CHECK_EQ(spinner.value(), 5);
}

TUINATOR_TEST(spinner_left_right_adjust_like_vertical_keys) {
    tuinator::Spinner spinner(0, 10, 4, 1, {}, tuinator::Theme{}.label);
    spinner.layout({0, 0, 12, 1});
    spinner.set_focused(true);

    TUINATOR_CHECK(spinner.handle_event(tuinator::KeyPress{tuinator::Key::Right}));
    TUINATOR_CHECK_EQ(spinner.value(), 5);

    TUINATOR_CHECK(spinner.handle_event(tuinator::KeyPress{tuinator::Key::Left}));
    TUINATOR_CHECK_EQ(spinner.value(), 4);
}

TUINATOR_TEST(spinner_page_keys_step_by_five) {
    tuinator::Spinner spinner(0, 20, 10, 1, {}, tuinator::Theme{}.label);
    spinner.layout({0, 0, 12, 1});
    spinner.set_focused(true);

    TUINATOR_CHECK(spinner.handle_event(tuinator::KeyPress{tuinator::Key::PageUp}));
    TUINATOR_CHECK_EQ(spinner.value(), 15);

    TUINATOR_CHECK(spinner.handle_event(tuinator::KeyPress{tuinator::Key::PageDown}));
    TUINATOR_CHECK_EQ(spinner.value(), 10);
}

TUINATOR_TEST(spinner_clamps_at_min_and_max) {
    tuinator::Spinner spinner(2, 4, 4, 2, {}, tuinator::Theme{}.label);
    spinner.layout({0, 0, 12, 1});
    spinner.set_focused(true);

    TUINATOR_CHECK(spinner.handle_event(tuinator::KeyPress{tuinator::Key::Up}));
    TUINATOR_CHECK_EQ(spinner.value(), 4);

    TUINATOR_CHECK(spinner.handle_event(tuinator::KeyPress{tuinator::Key::Down}));
    TUINATOR_CHECK_EQ(spinner.value(), 2);
}

TUINATOR_TEST(spinner_custom_step_size) {
    tuinator::Spinner spinner(0, 20, 0, 3, {}, tuinator::Theme{}.label);
    spinner.layout({0, 0, 12, 1});
    spinner.set_focused(true);

    TUINATOR_CHECK(spinner.handle_event(tuinator::KeyPress{tuinator::Key::Up}));
    TUINATOR_CHECK_EQ(spinner.value(), 3);
}

TUINATOR_TEST(spinner_on_change_fires) {
    int last = -1;
    tuinator::Spinner spinner(0, 5, 2, 1, [&](int value) { last = value; }, tuinator::Theme{}.label);
    spinner.layout({0, 0, 12, 1});
    spinner.set_focused(true);

    TUINATOR_CHECK(spinner.handle_event(tuinator::KeyPress{tuinator::Key::Up}));
    TUINATOR_CHECK_EQ(last, 3);
}

TUINATOR_TEST(spinner_mouse_click_left_decrements_right_increments) {
    tuinator::Spinner spinner(0, 10, 5, 1, {}, tuinator::Theme{}.label);
    spinner.layout({0, 0, 12, 1});

    tuinator::MouseEvent inc{};
    inc.position = {spinner.bounds().x + 8, spinner.bounds().y};
    inc.action = tuinator::MouseAction::Click;
    TUINATOR_CHECK(spinner.handle_event(inc));
    TUINATOR_CHECK_EQ(spinner.value(), 6);

    tuinator::MouseEvent dec{};
    dec.position = {spinner.bounds().x + 1, spinner.bounds().y};
    dec.action = tuinator::MouseAction::Click;
    TUINATOR_CHECK(spinner.handle_event(dec));
    TUINATOR_CHECK_EQ(spinner.value(), 5);
}

TUINATOR_TEST(spinner_renders_value_between_chevrons) {
    tuinator::MemoryTerminalBackend backend({14, 2});
    backend.init();

    tuinator::Spinner spinner(0, 9, 7, 1, {}, tuinator::Theme{}.label);
    spinner.layout({0, 0, 12, 1});

    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    spinner.paint(ctx);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "< 7 >"));
}
