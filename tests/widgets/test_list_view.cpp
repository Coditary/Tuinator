#include <tuinator/core/event.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/views/list_view.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(list_view_select_with_keys) {
    const tuinator::Theme theme = tuinator::dark_theme();
    tuinator::ListView list(theme.label, theme.button_focused);
    list.set_items({"Alpha", "Beta", "Gamma"});

    list.layout({0, 0, 20, 5});
    list.set_focused(true);

    TUINATOR_CHECK(list.handle_event(tuinator::KeyPress{tuinator::Key::Down}));
    TUINATOR_CHECK_EQ(list.selected_index(), 1);

    TUINATOR_CHECK(list.handle_event(tuinator::KeyPress{tuinator::Key::Up}));
    TUINATOR_CHECK_EQ(list.selected_index(), 0);
}

TUINATOR_TEST(list_view_mouse_click_selects_row) {
    const tuinator::Theme theme = tuinator::dark_theme();
    tuinator::ListView list(theme.label, theme.button_focused);
    list.set_items({"One", "Two", "Three"});
    list.layout({0, 0, 20, 5});

    tuinator::MouseEvent click{};
    click.position = {2, 2};
    click.action = tuinator::MouseAction::Click;

    TUINATOR_CHECK(list.handle_event(click));
    TUINATOR_CHECK_EQ(list.selected_index(), 2);
}

TUINATOR_TEST(list_view_renders_selection_marker) {
    tuinator::MemoryTerminalBackend backend({24, 6});
    backend.init();

    const tuinator::Theme theme = tuinator::dark_theme();
    tuinator::ListView list(theme.label, theme.button_focused);
    list.set_items({"Alpha", "Beta"});
    list.layout({0, 0, 24, 6});

    backend.begin_frame();
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    ctx.with_clip({0, 0, 24, 6}, [&](tuinator::PaintContext& clipped) { list.paint(clipped); });
    backend.end_frame();

    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 0, 0), '>');
}

TUINATOR_TEST(list_view_clears_empty_rows) {
    tuinator::MemoryTerminalBackend backend({12, 4});
    backend.init();
    backend.draw_text(0, 1, "GHOST", tuinator::Style{});
    backend.draw_text(0, 2, "GHOST", tuinator::Style{});
    backend.draw_text(0, 3, "GHOST", tuinator::Style{});

    const tuinator::Theme theme = tuinator::dark_theme();
    tuinator::ListView list(theme.label, theme.button_focused);
    list.set_items({"Only"});
    list.layout({0, 0, 12, 4});

    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    list.paint(ctx);

    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 2, 0), 'O');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 3, 0), 'n');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 0, 1), ' ');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 0, 2), ' ');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 0, 3), ' ');
}
