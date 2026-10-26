#include <tuinator/layout/box.hpp>
#include <tuinator/layout/grid.hpp>
#include <tuinator/render/terminal_image.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/chrome/dialog.hpp>
#include <tuinator/widgets/display/custom_paint.hpp>
#include <tuinator/widgets/display/image_view.hpp>
#include <tuinator/widgets/display/label.hpp>
#include <tuinator/widgets/menu/menu_bar.hpp>
#include <tuinator/window/window.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

#include <memory>

TUINATOR_TEST(edge_hbox_single_child_lays_out) {
    auto box = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 0});
    box->add_child(std::make_unique<tuinator::Label>("solo"));
    box->layout({0, 0, 30, 1});
    TUINATOR_CHECK(box->bounds().width >= 30);
}

TUINATOR_TEST(edge_grid_one_column_stacks_vertically) {
    tuinator::Grid grid(tuinator::GridOptions{.columns = 1, .gap = 0});
    grid.add_child(std::make_unique<tuinator::Label>("top"));
    grid.add_child(std::make_unique<tuinator::Label>("bottom"));
    grid.layout({0, 0, 12, 4});
    TUINATOR_CHECK(grid.bounds().height >= 2);
}

TUINATOR_TEST(edge_menu_bar_empty_menus_safe) {
    tuinator::MenuBar bar;
    bar.layout({0, 0, 40, 1});
    tuinator::MemoryTerminalBackend backend({40, 2});
    backend.init();
    tuinator::test::render_root(bar, backend);
    TUINATOR_CHECK(true);
}

TUINATOR_TEST(edge_image_view_tiny_cell_layout) {
    tuinator::MemoryTerminalBackend backend({8, 4});
    backend.init();
    tuinator::ImageView view(tuinator::TerminalImage::gradient(4, 4), {4, 2});
    view.layout({0, 0, 4, 2});
    tuinator::test::render_root(view, backend);
    TUINATOR_CHECK(true);
}

TUINATOR_TEST(edge_custom_paint_zero_bounds) {
    tuinator::MemoryTerminalBackend backend({4, 2});
    backend.init();
    tuinator::CustomPaint paint(
        [](tuinator::PaintContext&, tuinator::Rect) {}, tuinator::Size{0, 0});
    paint.layout({0, 0, 0, 0});
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    paint.paint(ctx);
    TUINATOR_CHECK(true);
}

TUINATOR_TEST(edge_dialog_confirm_without_callback_renders) {
    const tuinator::Theme theme = tuinator::dark_theme();
    auto dialog = tuinator::dialog::make_confirm(theme, "Sure?", nullptr);
    dialog->layout({0, 0, 30, 8});
    tuinator::MemoryTerminalBackend backend({30, 8});
    backend.init();
    tuinator::test::render_root(*dialog, backend);
    TUINATOR_CHECK(tuinator::test::row_has(backend, "Sure?"));
}

TUINATOR_TEST(edge_window_empty_title_still_shows_content) {
    tuinator::MemoryTerminalBackend backend({28, 10});
    backend.init();
    tuinator::Window window("", {0, 0, 26, 8}, std::make_unique<tuinator::Label>("body"));
    window.layout({0, 0, 26, 8});
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    window.paint(ctx);
    TUINATOR_CHECK(tuinator::test::row_has(backend, "body"));
}
