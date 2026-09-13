#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/backend/terminal_backend.hpp>
#include <tuinator/render/canvas.hpp>
#include <tuinator/render/dirty_region.hpp>
#include <tuinator/render/paint_context.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/widgets/controls/button.hpp>
#include <tuinator/widgets/display/label.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(dirty_region_starts_full) {
    tuinator::DirtyRegion region;
    TUINATOR_CHECK(region.needs_render());
    TUINATOR_CHECK(region.is_full());
}

TUINATOR_TEST(dirty_region_partial_union) {
    tuinator::DirtyRegion region;
    region.clear();

    region.mark_rect({1, 1, 4, 2});
    region.mark_rect({3, 2, 4, 3});

    TUINATOR_CHECK(region.needs_render());
    TUINATOR_CHECK(!region.is_full());
    TUINATOR_CHECK_EQ(region.bounds().x, 1);
    TUINATOR_CHECK_EQ(region.bounds().y, 1);
    TUINATOR_CHECK_EQ(region.bounds().width, 6);
    TUINATOR_CHECK_EQ(region.bounds().height, 4);
}

TUINATOR_TEST(dirty_region_mark_full_clears_partial) {
    tuinator::DirtyRegion region;
    region.clear();
    region.mark_rect({0, 0, 2, 2});
    region.mark_full();

    TUINATOR_CHECK(region.is_full());
    TUINATOR_CHECK(region.needs_render());
}

TUINATOR_TEST(memory_backend_partial_begin_frame) {
    tuinator::MemoryTerminalBackend backend({8, 4});
    backend.init();

    backend.draw_text(0, 0, "AAAAAAAA", tuinator::Style{});
    backend.draw_text(0, 3, "BBBBBBBB", tuinator::Style{});

    tuinator::BeginFrameOptions partial_frame;
    partial_frame.full_redraw = false;
    partial_frame.dirty_region = {0, 1, 8, 2};
    backend.begin_frame(partial_frame);
    backend.draw_text(0, 1, "CCCCCCCC", tuinator::Style{});
    backend.end_frame();

    const std::string snapshot = backend.snapshot();
    TUINATOR_CHECK(snapshot.find("AAAAAAAA") != std::string::npos);
    TUINATOR_CHECK(snapshot.find("BBBBBBBB") != std::string::npos);
    TUINATOR_CHECK(snapshot.find("CCCCCCCC") != std::string::npos);
}

TUINATOR_TEST(full_redraw_clears_stale_cells) {
    tuinator::MemoryTerminalBackend backend({4, 1});
    backend.init();

    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    tuinator::Label label("xxxx");
    label.layout({0, 0, 4, 1});

    backend.begin_frame({.full_redraw = true, .clear_buffer = true});
    label.paint(ctx);
    backend.end_frame();
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 0, 0), 'x');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 3, 0), 'x');

    label.set_text("y");
    backend.begin_frame({.full_redraw = true, .clear_buffer = true});
    label.paint(ctx);
    backend.end_frame();
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 0, 0), ' ');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 1, 0), 'y');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 2, 0), ' ');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 3, 0), ' ');
}

TUINATOR_TEST(partial_redraw_clears_stale_button_label) {
    tuinator::MemoryTerminalBackend backend({10, 1});
    backend.init();

    tuinator::Button button("Save");
    button.layout({0, 0, 10, 1});

    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);

    backend.begin_frame({.full_redraw = false, .dirty_region = {{0, 0}, {10, 1}}});
    button.paint(ctx);
    backend.end_frame();
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 1, 0), 'S');

    button.set_label("OK");
    backend.begin_frame({.full_redraw = false, .dirty_region = {{0, 0}, {10, 1}}});
    button.paint(ctx);
    backend.end_frame();
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 1, 0), 'O');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 2, 0), 'K');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 4, 0), ' ');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 5, 0), ' ');
}
