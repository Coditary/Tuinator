#include <tuinator/core/event.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/capabilities.hpp>
#include <tuinator/widgets/controls/toggle.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(toggle_space_flips_state) {
    tuinator::Toggle toggle("Dark mode", false);
    toggle.layout({0, 0, 24, 1});
    toggle.set_focused(true);

    TUINATOR_CHECK(!toggle.is_checked());
    TUINATOR_CHECK(toggle.handle_event(tuinator::KeyPress{.character = ' '}));
    TUINATOR_CHECK(toggle.is_checked());
}

TUINATOR_TEST(toggle_mouse_click_flips_state) {
    tuinator::Toggle toggle("WiFi", true);
    toggle.layout({0, 0, 20, 1});

    tuinator::MouseEvent click{};
    click.position = {toggle.bounds().x + 2, toggle.bounds().y};
    click.action = tuinator::MouseAction::Click;

    TUINATOR_CHECK(toggle.handle_event(click));
    TUINATOR_CHECK(!toggle.is_checked());
}

TUINATOR_TEST(toggle_on_change_callback) {
    bool last = false;
    tuinator::Toggle toggle("Flag", false, [&](bool on) { last = on; });
    toggle.layout({0, 0, 16, 1});
    toggle.set_focused(true);

    TUINATOR_CHECK(toggle.handle_event(tuinator::KeyPress{.character = ' '}));
    TUINATOR_CHECK(last);
}

TUINATOR_TEST(toggle_exposes_toggle_control_capability) {
    tuinator::Toggle toggle("X", false);
    tuinator::ToggleControl* control = tuinator::as_toggle_control(&toggle);
    TUINATOR_CHECK(control != nullptr);
    TUINATOR_CHECK(!control->is_checked());
    control->set_checked(true);
    TUINATOR_CHECK(toggle.is_checked());
}

TUINATOR_TEST(toggle_renders_on_off_labels) {
    tuinator::MemoryTerminalBackend backend({28, 3});
    backend.init();

    tuinator::Toggle toggle("Sound", true);
    toggle.layout({0, 0, 28, 1});
    tuinator::test::render_root(toggle, backend);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "Sound"));
}
