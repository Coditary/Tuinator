#include <tuinator/core/event.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/views/list_view.hpp>
#include <tuinator/widgets/views/table.hpp>
#include <tuinator/widgets/views/tree_view.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

namespace {

const tuinator::Theme& theme() {
    static const tuinator::Theme k = tuinator::dark_theme();
    return k;
}

} // namespace

TUINATOR_TEST(edge_list_view_empty_items_ignore_keys) {
    tuinator::ListView list(theme().label, theme().button_focused);
    list.layout({0, 0, 20, 4});
    list.set_focused(true);
    TUINATOR_CHECK_EQ(list.selected_index(), 0);
    TUINATOR_CHECK(!list.handle_event(tuinator::KeyPress{tuinator::Key::Down}));
}

TUINATOR_TEST(edge_list_view_selection_clamps_after_shrink) {
    tuinator::ListView list(theme().label, theme().button_focused);
    list.set_items({"a", "b", "c"});
    list.set_selected_index(2);
    list.set_items({"only"});
    TUINATOR_CHECK_EQ(list.selected_index(), 0);
}

TUINATOR_TEST(edge_list_view_out_of_range_index_clamps) {
    tuinator::ListView list(theme().label, theme().button_focused);
    list.set_items({"x", "y"});
    list.set_selected_index(99);
    TUINATOR_CHECK_EQ(list.selected_index(), 1);
    list.set_selected_index(-5);
    TUINATOR_CHECK_EQ(list.selected_index(), 0);
}

TUINATOR_TEST(edge_list_view_up_at_top_stays) {
    tuinator::ListView list(theme().label, theme().button_focused);
    list.set_items({"first", "second"});
    list.layout({0, 0, 20, 4});
    list.set_focused(true);
    TUINATOR_CHECK(list.handle_event(tuinator::KeyPress{tuinator::Key::Up}));
    TUINATOR_CHECK_EQ(list.selected_index(), 0);
}

TUINATOR_TEST(edge_list_view_click_outside_bounds_ignored) {
    tuinator::ListView list(theme().label, theme().button_focused);
    list.set_items({"a", "b"});
    list.set_selected_index(0);
    list.layout({5, 2, 10, 3});

    tuinator::MouseEvent outside{};
    outside.position = {0, 0};
    outside.action = tuinator::MouseAction::Click;
    TUINATOR_CHECK(!list.handle_event(outside));
    TUINATOR_CHECK_EQ(list.selected_index(), 0);
}

TUINATOR_TEST(edge_list_view_zero_height_paint_is_noop) {
    tuinator::MemoryTerminalBackend backend({10, 4});
    backend.init();
    tuinator::ListView list(theme().label, theme().button_focused);
    list.set_items({"visible"});
    list.layout({0, 0, 0, 0});
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    list.paint(ctx);
    TUINATOR_CHECK(true);
}

TUINATOR_TEST(edge_list_view_scrolls_when_selection_below_viewport) {
    tuinator::ListView list(theme().label, theme().button_focused);
    std::vector<std::string> items;
    for (int i = 0; i < 20; ++i) {
        items.push_back("row-" + std::to_string(i));
    }
    list.set_items(std::move(items));
    list.layout({0, 0, 16, 3});
    list.set_selected_index(18);
    list.set_focused(true);
    TUINATOR_CHECK(list.handle_event(tuinator::KeyPress{tuinator::Key::Home}));
    TUINATOR_CHECK_EQ(list.selected_index(), 0);
}

TUINATOR_TEST(edge_table_empty_rows_ignore_navigation) {
    tuinator::Table table;
    table.set_columns({{"Name", 8}});
    table.layout({0, 0, 12, 6});
    table.set_focused(true);
    TUINATOR_CHECK(!table.handle_event(tuinator::KeyPress{tuinator::Key::Down}));
    TUINATOR_CHECK_EQ(table.selected_row(), 0);
}

TUINATOR_TEST(edge_table_selection_clamps_when_rows_removed) {
    tuinator::Table table;
    table.set_columns({{"A", 4}, {"B", 4}});
    table.set_rows({{"1", "2"}, {"3", "4"}});
    table.set_selected_row(1);
    table.set_rows({{"solo", "x"}});
    TUINATOR_CHECK_EQ(table.selected_row(), 0);
}

TUINATOR_TEST(edge_table_clips_wide_cell_to_column) {
    tuinator::MemoryTerminalBackend backend({14, 4});
    backend.init();
    tuinator::Table table;
    table.set_columns({{"W", 4}});
    table.set_rows({{"abcdefghij"}});
    table.layout({0, 0, 14, 4});
    tuinator::test::render_root(table, backend);
    TUINATOR_CHECK(tuinator::test::row_has(backend, "abcd"));
    TUINATOR_CHECK(!tuinator::test::row_has(backend, "ghij"));
}

TUINATOR_TEST(edge_table_mouse_click_on_header_row_ignored) {
    tuinator::Table table;
    table.set_columns({{"N", 6}});
    table.set_rows({{"one"}});
    table.layout({0, 0, 10, 8});
    table.set_selected_row(0);

    tuinator::MouseEvent click{};
    click.position = {1, 0};
    click.action = tuinator::MouseAction::Click;
    TUINATOR_CHECK(!table.handle_event(click));
    TUINATOR_CHECK_EQ(table.selected_row(), 0);
}

TUINATOR_TEST(edge_tree_view_empty_root_ignores_keys) {
    tuinator::TreeView tree(theme().label, theme().button_focused);
    tree.layout({0, 0, 20, 6});
    tree.set_focused(true);
    TUINATOR_CHECK(!tree.handle_event(tuinator::KeyPress{tuinator::Key::Down}));
}

TUINATOR_TEST(edge_tree_view_collapsed_child_hidden_until_expanded) {
    tuinator::TreeNode root;
    root.label = "root";
    root.expanded = true;
    root.children.push_back(tuinator::TreeNode{.label = "child", .expanded = false});
    root.children.back().children.push_back(tuinator::TreeNode{.label = "grand"});

    tuinator::TreeView tree(theme().label, theme().button_focused);
    tree.set_root(std::move(root));
    tree.layout({0, 0, 24, 8});

    tuinator::MemoryTerminalBackend backend({24, 8});
    backend.init();
    tuinator::test::render_root(tree, backend);
    TUINATOR_CHECK(tuinator::test::row_has(backend, "root"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "child"));
    TUINATOR_CHECK(!tuinator::test::row_has(backend, "grand"));
}

TUINATOR_TEST(edge_tree_view_right_toggles_expand) {
    tuinator::TreeNode root;
    root.label = "r";
    root.expanded = true;
    root.children.push_back(tuinator::TreeNode{.label = "c", .expanded = false});
    root.children.back().children.push_back(tuinator::TreeNode{.label = "leaf"});

    tuinator::TreeView tree(theme().label, theme().button_focused);
    tree.set_root(std::move(root));
    tree.layout({0, 0, 20, 6});
    tree.set_focused(true);
    tree.set_selected_index(1);
    TUINATOR_CHECK(tree.handle_event(tuinator::KeyPress{tuinator::Key::Right}));

    tuinator::MemoryTerminalBackend backend({20, 6});
    backend.init();
    tuinator::test::render_root(tree, backend);
    TUINATOR_CHECK(tuinator::test::row_has(backend, "leaf"));
}
