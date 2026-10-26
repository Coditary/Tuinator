#include <tuinator/window/desktop.hpp>
#include <tuinator/widgets/display/label.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

#include <memory>

TUINATOR_TEST(desktop_create_window_sets_active_window) {
    tuinator::Desktop desktop;
    desktop.layout({0, 0, 80, 24});

    tuinator::Window* window =
        desktop.create_window("Editor", {10, 5, 40, 14}, std::make_unique<tuinator::Label>("Hello"));
    TUINATOR_CHECK(window != nullptr);
    TUINATOR_CHECK_EQ(desktop.active_window(), window);
}

TUINATOR_TEST(desktop_modal_window_is_marked_modal) {
    tuinator::Desktop desktop;
    desktop.layout({0, 0, 80, 24});

    tuinator::Window* modal = desktop.show_modal("Confirm", {20, 8, 36, 10}, std::make_unique<tuinator::Label>("Sure?"));
    TUINATOR_CHECK(modal != nullptr);
    TUINATOR_CHECK(modal->is_modal());
    TUINATOR_CHECK(!modal->is_resizable());
}

TUINATOR_TEST(desktop_close_window_clears_active) {
    tuinator::Desktop desktop;
    desktop.layout({0, 0, 60, 20});

    tuinator::Window* window =
        desktop.create_window("Tmp", {4, 3, 24, 10}, std::make_unique<tuinator::Label>("Body"));
    TUINATOR_CHECK_EQ(desktop.active_window(), window);
    desktop.close_window(window);
    TUINATOR_CHECK(desktop.active_window() == nullptr);
}
