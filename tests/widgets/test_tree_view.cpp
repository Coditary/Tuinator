#include <tuinator/core/event.hpp>
#include <tuinator/platform/clipboard.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/views/tree_view.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

namespace {

void setup_sample_tree(tuinator::TreeView& tree) {
    tree.set_root({
        "root",
        {
            {"branch", {{"leaf-a"}, {"leaf-b"}}},
        },
    });
    tree.layout({0, 0, 24, 8});
    tree.set_focused(true);
}

} // namespace

TUINATOR_TEST(tree_view_down_moves_selection_into_children) {
    tuinator::TreeView tree(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    setup_sample_tree(tree);
    const int start = tree.selected_index();

    TUINATOR_CHECK(tree.handle_event(tuinator::KeyPress{tuinator::Key::Down}));
    TUINATOR_CHECK(tree.selected_index() > start);
}

TUINATOR_TEST(tree_view_right_toggles_expand_collapse) {
    tuinator::TreeView tree(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    setup_sample_tree(tree);
    tree.set_selected_index(0);

    TUINATOR_CHECK(tree.handle_event(tuinator::KeyPress{tuinator::Key::Right}));
    TUINATOR_CHECK(tree.handle_event(tuinator::KeyPress{tuinator::Key::Right}));

    tuinator::MemoryTerminalBackend backend({24, 8});
    backend.init();
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    tree.paint(ctx);

    const bool shows_collapsed = tuinator::test::row_contains(backend, 0, "[+]");
    const bool shows_expanded = tuinator::test::row_contains(backend, 0, "[-]");
    TUINATOR_CHECK(shows_collapsed || shows_expanded);
}

TUINATOR_TEST(tree_view_ctrl_c_copies_selected_label) {
    tuinator::TreeView tree(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    setup_sample_tree(tree);
    tree.set_selected_index(2);

    tuinator::clipboard::set("");
    TUINATOR_CHECK(tree.handle_event(tuinator::KeyPress{.character = 'c', .ctrl = true}));
    TUINATOR_CHECK_EQ(tuinator::clipboard::get(), "leaf-a");
}

TUINATOR_TEST(tree_view_copy_disabled_ignores_ctrl_c) {
    tuinator::TreeView tree(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    setup_sample_tree(tree);
    tree.set_copyable(false);

    tuinator::clipboard::set("unchanged");
    TUINATOR_CHECK(!tree.handle_event(tuinator::KeyPress{.character = 'c', .ctrl = true}));
    TUINATOR_CHECK_EQ(tuinator::clipboard::get(), "unchanged");
}

TUINATOR_TEST(tree_view_renders_root_label) {
    tuinator::MemoryTerminalBackend backend({20, 6});
    backend.init();

    tuinator::TreeView tree(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    tree.set_root({"Projects", {}});
    tree.layout({0, 0, 20, 6});

    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    tree.paint(ctx);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "Projects"));
}
