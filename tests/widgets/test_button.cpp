#include <tuinator/core/event.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/capabilities.hpp>
#include <tuinator/widgets/controls/button.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(button_renders_bracketed_label) {
    tuinator::MemoryTerminalBackend backend({20, 3});
    backend.init();

    tuinator::Button button("Save");
    button.layout({0, 0, 20, 1});

    tuinator::test::render_root(button, backend);
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "[Save]"));
}

TUINATOR_TEST(button_activate_invokes_callback) {
    bool fired = false;
    tuinator::Button button("Go", [&fired]() { fired = true; });

    tuinator::Activatable* activatable = tuinator::as_activatable(&button);
    TUINATOR_CHECK(activatable != nullptr);
    activatable->activate();
    TUINATOR_CHECK(fired);
}

TUINATOR_TEST(button_enter_when_focused_activates) {
    bool fired = false;
    tuinator::Button button("OK", [&fired]() { fired = true; });
    button.layout({0, 0, 10, 1});
    button.set_focused(true);

    TUINATOR_CHECK(button.handle_event(tuinator::KeyPress{tuinator::Key::Enter}));
    TUINATOR_CHECK(fired);
}

TUINATOR_TEST(button_mouse_click_activates) {
    bool fired = false;
    tuinator::Button button("Click", [&fired]() { fired = true; });
    button.layout({0, 0, 16, 1});

    tuinator::MouseEvent click{};
    click.position = {button.bounds().x + 2, button.bounds().y};
    click.action = tuinator::MouseAction::Click;

    TUINATOR_CHECK(button.handle_event(click));
    TUINATOR_CHECK(fired);
}
