#include <tuinator/core/event.hpp>
#include <tuinator/platform/clipboard.hpp>
#include <tuinator/widgets/views/table.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

namespace {

void setup_sample_table(tuinator::Table& table) {
    table.set_columns({{"Name", 8}, {"Value", 6}});
    table.set_rows({{"alpha", "1"}, {"beta", "2"}, {"gamma", "3"}});
}

} // namespace

TUINATOR_TEST(table_renders_column_headers) {
    tuinator::MemoryTerminalBackend backend({24, 6});
    backend.init();

    tuinator::Table table;
    setup_sample_table(table);
    table.layout({0, 0, 24, 6});

    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    table.paint(ctx);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "Name"));
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "Value"));
}

TUINATOR_TEST(table_arrow_keys_move_selection) {
    tuinator::Table table;
    setup_sample_table(table);
    table.layout({0, 0, 24, 6});
    table.set_focused(true);
    TUINATOR_CHECK_EQ(table.selected_row(), 0);

    TUINATOR_CHECK(table.handle_event(tuinator::KeyPress{tuinator::Key::Down}));
    TUINATOR_CHECK_EQ(table.selected_row(), 1);

    TUINATOR_CHECK(table.handle_event(tuinator::KeyPress{tuinator::Key::Up}));
    TUINATOR_CHECK_EQ(table.selected_row(), 0);
}

TUINATOR_TEST(table_home_and_end_jump_selection) {
    tuinator::Table table;
    setup_sample_table(table);
    table.layout({0, 0, 24, 6});
    table.set_focused(true);
    table.set_selected_row(2);

    TUINATOR_CHECK(table.handle_event(tuinator::KeyPress{tuinator::Key::Home}));
    TUINATOR_CHECK_EQ(table.selected_row(), 0);

    TUINATOR_CHECK(table.handle_event(tuinator::KeyPress{tuinator::Key::End}));
    TUINATOR_CHECK_EQ(table.selected_row(), 2);
}

TUINATOR_TEST(table_mouse_click_selects_row) {
    tuinator::Table table;
    setup_sample_table(table);
    table.layout({0, 0, 24, 6});

    tuinator::MouseEvent click{};
    click.position = {2, 3};
    click.action = tuinator::MouseAction::Click;

    TUINATOR_CHECK(table.handle_event(click));
    TUINATOR_CHECK_EQ(table.selected_row(), 2);
}

TUINATOR_TEST(table_enter_invokes_activate_callback) {
    tuinator::Table table;
    setup_sample_table(table);
    table.layout({0, 0, 24, 6});
    table.set_focused(true);
    table.set_selected_row(1);

    int activated_row = -1;
    table.set_on_activate([&](int row, const std::vector<std::string>& cells) {
        activated_row = row;
        TUINATOR_CHECK_EQ(cells.size(), 2u);
        TUINATOR_CHECK_EQ(cells[0], "beta");
    });

    TUINATOR_CHECK(table.handle_event(tuinator::KeyPress{tuinator::Key::Enter}));
    TUINATOR_CHECK_EQ(activated_row, 1);
}

TUINATOR_TEST(table_ctrl_c_copies_selected_row) {
    tuinator::Table table;
    setup_sample_table(table);
    table.layout({0, 0, 24, 6});
    table.set_focused(true);
    table.set_selected_row(1);

    tuinator::clipboard::set("");
    TUINATOR_CHECK(table.handle_event(tuinator::KeyPress{.character = 'c', .ctrl = true}));
    TUINATOR_CHECK_EQ(tuinator::clipboard::get(), "beta\t2");
}

TUINATOR_TEST(table_copy_disabled_ignores_ctrl_c) {
    tuinator::Table table;
    setup_sample_table(table);
    table.set_copyable(false);
    table.layout({0, 0, 24, 6});
    table.set_focused(true);

    tuinator::clipboard::set("unchanged");
    TUINATOR_CHECK(!table.handle_event(tuinator::KeyPress{.character = 'c', .ctrl = true}));
    TUINATOR_CHECK_EQ(tuinator::clipboard::get(), "unchanged");
}
