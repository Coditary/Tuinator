#include <tuinator/core/screen.hpp>
#include <tuinator/layout/box.hpp>
#include <tuinator/testing/app_driver.hpp>
#include <tuinator/widgets/containers/scroll_view.hpp>
#include <tuinator/widgets/containers/split_pane.hpp>

#include "component_expect.hpp"
#include "pane_test_helpers.hpp"
#include "test_harness.hpp"

#include <memory>

namespace {

std::unique_ptr<tuinator::Widget> ide_layout() {
    auto sidebar_list = tuinator::test::panes::tall_list_vbox(30, "file");
    auto sidebar_scroll = std::make_unique<tuinator::ScrollView>(
        std::move(sidebar_list), tuinator::ScrollViewOptions{.width = 18, .height = 10});
    sidebar_scroll->set_test_id("sidebar-scroll");
    auto sidebar = tuinator::test::panes::labeled_panel("Explorer", std::move(sidebar_scroll), "sidebar");

    auto editor = tuinator::test::panes::labeled_panel("Editor", tuinator::test::panes::make_label("fn main() {}", "code"),
                                                       "editor");

    auto columns = std::make_unique<tuinator::SplitPane>(
        std::move(sidebar), std::move(editor),
        tuinator::SplitPaneOptions{
            .orientation = tuinator::SplitOrientation::Horizontal,
            .first_size = 22,
        });
    columns->set_test_id("columns");

    auto status = tuinator::test::panes::make_label("Ready", "status");
    auto rows = std::make_unique<tuinator::SplitPane>(
        std::move(columns), std::move(status),
        tuinator::SplitPaneOptions{
            .orientation = tuinator::SplitOrientation::Vertical,
            .first_size = 12,
        });
    rows->set_test_id("rows");

    return tuinator::test::panes::labeled_panel("Workspace", std::move(rows), "workspace");
}

} // namespace

TUINATOR_TEST(integration_ide_shell_renders_all_regions) {
    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(ide_layout());

    tuinator::testing::AppDriver driver({72, 20}, std::move(root));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Workspace"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Explorer"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Editor"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "file-0"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Ready"));
}

TUINATOR_TEST(integration_sidebar_scroll_inside_nested_split) {
    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(ide_layout());

    tuinator::testing::AppDriver driver({72, 20}, std::move(root));
    tuinator::ScrollView* scroll = tuinator::test::panes::as_scroll(driver, "sidebar-scroll");
    TUINATOR_CHECK(scroll != nullptr);
    scroll->scroll_to(0, 15);
    driver.present();

    TUINATOR_CHECK(!tuinator::test::expect::snapshot_contains(driver, "file-0"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "file-15"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "fn main()"));
}

TUINATOR_TEST(integration_resize_terminal_reflows_split_children) {
    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(ide_layout());

    tuinator::testing::AppDriver driver({80, 22}, std::move(root));
    const int wide_editor = tuinator::test::panes::bounds_of(driver, "editor").width;

    driver.resize_terminal({44, 14});
    const int narrow_editor = tuinator::test::panes::bounds_of(driver, "editor").width;

    TUINATOR_CHECK(wide_editor > narrow_editor);
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Editor"));
}

TUINATOR_TEST(integration_triple_nested_panel_titles) {
    auto inner = tuinator::test::panes::labeled_panel("L3", tuinator::test::panes::make_label("core", "core"), "l3");
    auto mid = tuinator::test::panes::labeled_panel("L2", std::move(inner), "l2");
    auto outer = tuinator::test::panes::labeled_panel("L1", std::move(mid), "l1");

    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(std::move(outer));

    tuinator::testing::AppDriver driver({50, 18}, std::move(root));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "L1"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "L2"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "L3"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "core"));
}

TUINATOR_TEST(integration_panel_split_panel_chain) {
    auto right = tuinator::test::panes::labeled_panel("Right", tuinator::test::panes::make_label("R"), "right");
    auto left = tuinator::test::panes::labeled_panel("Left", tuinator::test::panes::make_label("L"), "left");
    auto split = std::make_unique<tuinator::SplitPane>(std::move(left), std::move(right),
                                                       tuinator::SplitPaneOptions{
                                                           .orientation = tuinator::SplitOrientation::Horizontal,
                                                           .first_size = 18,
                                                       });
    auto shell = tuinator::test::panes::labeled_panel("Shell", std::move(split), "shell");

    auto root = tuinator::make_screen({.padding = 0});
    root->add_child(std::move(shell));

    tuinator::testing::AppDriver driver({56, 12}, std::move(root));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Shell"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Left"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Right"));

    tuinator::Panel* left_panel = tuinator::test::panes::as_panel(driver, "left");
    tuinator::Panel* right_panel = tuinator::test::panes::as_panel(driver, "right");
    TUINATOR_CHECK(left_panel != nullptr);
    TUINATOR_CHECK(right_panel != nullptr);
    TUINATOR_CHECK(!left_panel->border_edges().right);
    TUINATOR_CHECK(!right_panel->border_edges().left);
}

TUINATOR_TEST(integration_hbox_with_two_panels_and_split) {
    auto a = tuinator::test::panes::labeled_panel("A", tuinator::test::panes::make_label("aaa"), "pa");
    auto b = tuinator::test::panes::labeled_panel("B", tuinator::test::panes::make_label("bbb"), "pb");
    auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 1});
    row->add_child(std::move(a));
    row->add_child(std::move(b));

    auto bottom = tuinator::test::panes::make_label("footer-line", "footer");
    auto split = std::make_unique<tuinator::SplitPane>(
        std::move(row), std::move(bottom),
        tuinator::SplitPaneOptions{
            .orientation = tuinator::SplitOrientation::Vertical,
            .first_size = 6,
        });
    split->set_flex(1);

    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(std::move(split));

    tuinator::testing::AppDriver driver({60, 18}, std::move(root));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "aaa"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "bbb"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "footer-line"));

    const tuinator::Rect pa = tuinator::test::panes::bounds_of(driver, "pa");
    const tuinator::Rect pb = tuinator::test::panes::bounds_of(driver, "pb");
    TUINATOR_CHECK(pa.x < pb.x);
}

TUINATOR_TEST(integration_tiny_terminal_keeps_markers) {
    auto root = tuinator::make_screen({.padding = 0});
    root->add_child(ide_layout());

    tuinator::testing::AppDriver driver({28, 8}, std::move(root));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Workspace"));
}

TUINATOR_TEST(integration_deep_five_panel_nesting_layout) {
    std::unique_ptr<tuinator::Widget> current =
        tuinator::test::panes::make_label("deep", "leaf");
    for (int level = 5; level >= 1; --level) {
        current = tuinator::test::panes::labeled_panel("P" + std::to_string(level), std::move(current),
                                                       "p" + std::to_string(level));
    }

    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(std::move(current));

    tuinator::testing::AppDriver driver({40, 16}, std::move(root));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "P1"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "deep"));

    const tuinator::Rect leaf = tuinator::test::panes::bounds_of(driver, "leaf");
    const tuinator::Rect outer = tuinator::test::panes::bounds_of(driver, "p1");
    TUINATOR_CHECK(leaf.x >= outer.x);
    TUINATOR_CHECK(leaf.y >= outer.y);
    TUINATOR_CHECK(leaf.right() <= outer.right());
    TUINATOR_CHECK(leaf.bottom() <= outer.bottom());
}

TUINATOR_TEST(integration_nested_split_resize_preserves_visibility) {
    auto inner = std::make_unique<tuinator::SplitPane>(
        tuinator::test::panes::make_label("IN-A", "ina"), tuinator::test::panes::make_label("IN-B", "inb"),
        tuinator::SplitPaneOptions{.orientation = tuinator::SplitOrientation::Horizontal, .first_size = 10});
    inner->set_test_id("inner-split");

    auto outer = std::make_unique<tuinator::SplitPane>(
        std::move(inner), tuinator::test::panes::make_label("OUTER", "out"),
        tuinator::SplitPaneOptions{.orientation = tuinator::SplitOrientation::Vertical, .first_size = 6});
    outer->set_test_id("outer-split");
    outer->set_flex(1);

    auto root = tuinator::make_screen({.padding = 0});
    root->add_child(std::move(outer));

    tuinator::testing::AppDriver driver({50, 16}, std::move(root));
    driver.resize_terminal({50, 8});
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "IN-A"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "OUTER"));
}
