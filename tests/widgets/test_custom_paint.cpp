#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/display/custom_paint.hpp>
#include <tuinator/widgets/display/label.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(custom_paint_draws_callback) {
    tuinator::MemoryTerminalBackend backend({12, 4});
    backend.init();

    auto canvas = std::make_unique<tuinator::CustomPaint>(
        [](tuinator::PaintContext& ctx, tuinator::Rect area) {
            ctx.canvas.fill_rect(area, '#', {});
            ctx.canvas.draw_text({1, 0}, "Hi", {});
        },
        tuinator::Size{10, 2});

    tuinator::test::render_root(*canvas, backend);

    TUINATOR_CHECK(tuinator::test::row_has(backend, "Hi"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "#"));
}

TUINATOR_TEST(custom_paint_composes_children) {
    tuinator::MemoryTerminalBackend backend({16, 3});
    backend.init();

    auto host = std::make_unique<tuinator::CustomPaint>();
    host->set_preferred_size({12, 2});
    host->set_paint_children_first(false);
    host->set_on_paint([](tuinator::PaintContext& ctx, tuinator::Rect area) {
        ctx.canvas.fill_rect(area, '.', {});
    });
    host->add_child(std::make_unique<tuinator::Label>("Child", tuinator::dark_theme().label));

    tuinator::test::render_root(*host, backend);

    TUINATOR_CHECK(tuinator::test::row_has(backend, "Child"));
}

TUINATOR_TEST(custom_paint_widget_type_name) {
    tuinator::CustomPaint paint;
    TUINATOR_CHECK_EQ(paint.widget_type_name(), "CustomPaint");
}
