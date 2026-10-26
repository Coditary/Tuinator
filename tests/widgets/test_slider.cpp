#include <tuinator/core/event.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/controls/slider.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(slider_arrow_keys_change_value) {
    tuinator::Slider slider(0, 10, 0, {}, tuinator::Theme{}.label, 12);
    slider.layout({0, 0, 12, 1});
    slider.set_focused(true);

    TUINATOR_CHECK(slider.handle_event(tuinator::KeyPress{tuinator::Key::Right}));
    TUINATOR_CHECK_EQ(slider.value(), 1);

    TUINATOR_CHECK(slider.handle_event(tuinator::KeyPress{tuinator::Key::Left}));
    TUINATOR_CHECK_EQ(slider.value(), 0);
}

TUINATOR_TEST(slider_home_and_end_jump_to_limits) {
    tuinator::Slider slider(2, 20, 10, {}, tuinator::Theme{}.label, 16);
    slider.layout({0, 0, 16, 1});
    slider.set_focused(true);

    TUINATOR_CHECK(slider.handle_event(tuinator::KeyPress{tuinator::Key::Home}));
    TUINATOR_CHECK_EQ(slider.value(), 2);

    TUINATOR_CHECK(slider.handle_event(tuinator::KeyPress{tuinator::Key::End}));
    TUINATOR_CHECK_EQ(slider.value(), 20);
}

TUINATOR_TEST(slider_clamps_value_at_bounds) {
    tuinator::Slider slider(0, 3, 3, {}, tuinator::Theme{}.label, 8);
    slider.layout({0, 0, 8, 1});
    slider.set_focused(true);

    TUINATOR_CHECK(slider.handle_event(tuinator::KeyPress{tuinator::Key::Right}));
    TUINATOR_CHECK_EQ(slider.value(), 3);
}

TUINATOR_TEST(slider_on_change_fires_for_keyboard) {
    int last = -1;
    tuinator::Slider slider(0, 5, 2, [&](int value) { last = value; }, tuinator::Theme{}.label, 10);
    slider.layout({0, 0, 10, 1});
    slider.set_focused(true);

    TUINATOR_CHECK(slider.handle_event(tuinator::KeyPress{tuinator::Key::Right}));
    TUINATOR_CHECK_EQ(last, 3);
}

TUINATOR_TEST(slider_mouse_click_sets_value_from_position) {
    tuinator::Slider slider(0, 10, 0, {}, tuinator::Theme{}.label, 11);
    slider.layout({0, 0, 11, 1});

    tuinator::MouseEvent click{};
    click.position = {slider.bounds().x + 10, slider.bounds().y};
    click.action = tuinator::MouseAction::Click;

    TUINATOR_CHECK(slider.handle_event(click));
    TUINATOR_CHECK(slider.value() > 0);
}

TUINATOR_TEST(slider_renders_track_with_thumb_marker) {
    tuinator::MemoryTerminalBackend backend({14, 2});
    backend.init();

    tuinator::Slider slider(0, 4, 2, {}, tuinator::Theme{}.label, 12);
    slider.layout({0, 0, 12, 1});

    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    slider.paint(ctx);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "O"));
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "-"));
}
