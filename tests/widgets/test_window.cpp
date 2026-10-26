#include <tuinator/window/window.hpp>
#include <tuinator/widgets/display/label.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

#include <memory>

TUINATOR_TEST(window_title_bar_and_content_area) {
    auto content = std::make_unique<tuinator::Label>("Inside");
    tuinator::Window window("My App", {0, 0, 28, 10}, std::move(content));

    TUINATOR_CHECK(window.is_title_bar({0, 0}));
    TUINATOR_CHECK(!window.is_title_bar({0, 1}));

    const tuinator::Rect area = window.content_area_in_window();
    TUINATOR_CHECK_EQ(area.x, 1);
    TUINATOR_CHECK_EQ(area.y, 1);
    TUINATOR_CHECK(area.width > 0);
    TUINATOR_CHECK(area.height > 0);
}

TUINATOR_TEST(window_paints_title_and_content) {
    tuinator::MemoryTerminalBackend backend({32, 12});
    backend.init();

    auto content = std::make_unique<tuinator::Label>("Payload");
    tuinator::Window window("Title", {0, 0, 30, 10}, std::move(content));
    window.layout({0, 0, 30, 10});

    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    window.paint(ctx);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "Title"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "Payload"));
}

TUINATOR_TEST(window_hit_test_focusable_targets_content) {
    auto content = std::make_unique<tuinator::Label>("X");
    tuinator::Window window("W", {5, 3, 20, 8}, std::move(content));
    window.layout({5, 3, 20, 8});

    TUINATOR_CHECK(window.hit_test_focusable({6, 4}) != nullptr);
    TUINATOR_CHECK(window.hit_test_focusable({5, 3}) == nullptr);
}
