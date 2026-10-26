#include <tuinator/core/screen.hpp>
#include <tuinator/layout/box.hpp>
#include <tuinator/layout/grid.hpp>
#include <tuinator/testing/app_driver.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/containers/scroll_view.hpp>
#include <tuinator/widgets/containers/panel.hpp>
#include <tuinator/widgets/containers/split_pane.hpp>

#include "component_expect.hpp"
#include "pane_test_helpers.hpp"
#include "test_harness.hpp"

#include <memory>

TUINATOR_TEST(panel_title_and_body_visible_in_snapshot) {
    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(tuinator::test::panes::labeled_panel("Files", tuinator::test::panes::make_label("readme.md"),
                                                         "files-panel"));

    tuinator::testing::AppDriver driver({36, 10}, std::move(root));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Files"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "readme.md"));
}

TUINATOR_TEST(panel_content_lives_inside_border_insets) {
    auto panel = std::make_unique<tuinator::Panel>("Box");
    panel->set_test_id("outer");
    panel->set_content(tuinator::test::panes::make_label("X", "inner"));
    panel->layout({0, 0, 20, 8});

    const tuinator::Rect frame = panel->bounds();
    const tuinator::Rect label = panel->content()->bounds();

    TUINATOR_CHECK_EQ(label.x, frame.x + 1);
    TUINATOR_CHECK_EQ(label.y, frame.y + 1);
    TUINATOR_CHECK(label.right() <= frame.right() - 1);
    TUINATOR_CHECK(label.bottom() <= frame.bottom() - 1);
}

TUINATOR_TEST(split_pane_assigns_distinct_regions_to_children) {
    auto split = std::make_unique<tuinator::SplitPane>(
        tuinator::test::panes::make_label("ALPHA", "left"), tuinator::test::panes::make_label("BETA", "right"),
        tuinator::SplitPaneOptions{
            .orientation = tuinator::SplitOrientation::Horizontal,
            .first_size = 16,
        });
    split->set_test_id("split");

    auto root = tuinator::make_screen({.padding = 0});
    root->add_child(std::move(split));

    tuinator::testing::AppDriver driver({48, 8}, std::move(root));
    const tuinator::Rect left = tuinator::test::panes::bounds_of(driver, "left");
    const tuinator::Rect right = tuinator::test::panes::bounds_of(driver, "right");

    TUINATOR_CHECK(left.width > 0);
    TUINATOR_CHECK(right.width > 0);
    TUINATOR_CHECK(left.right() <= right.x);
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "ALPHA"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "BETA"));
}

TUINATOR_TEST(split_pane_vertical_stacks_first_above_second) {
    auto split = std::make_unique<tuinator::SplitPane>(
        tuinator::test::panes::make_label("TOP", "top"), tuinator::test::panes::make_label("BOTTOM", "bottom"),
        tuinator::SplitPaneOptions{
            .orientation = tuinator::SplitOrientation::Vertical,
            .first_size = 4,
        });
    split->set_test_id("vsplit");

    auto root = tuinator::make_screen({.padding = 0});
    root->add_child(std::move(split));

    tuinator::testing::AppDriver driver({40, 14}, std::move(root));
    const tuinator::Rect top = tuinator::test::panes::bounds_of(driver, "top");
    const tuinator::Rect bottom = tuinator::test::panes::bounds_of(driver, "bottom");
    TUINATOR_CHECK(top.bottom() <= bottom.y);
}

TUINATOR_TEST(scroll_view_viewport_hides_scrolled_away_rows) {
    auto scroll = std::make_unique<tuinator::ScrollView>(tuinator::test::panes::tall_list_vbox(20, "line"),
                                                         tuinator::ScrollViewOptions{.width = 24, .height = 6});
    scroll->set_test_id("scroll");

    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(std::move(scroll));

    tuinator::testing::AppDriver driver({30, 10}, std::move(root));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "line-0"));

    tuinator::ScrollView* view = tuinator::test::panes::as_scroll(driver, "scroll");
    TUINATOR_CHECK(view != nullptr);
    view->scroll_to(0, 12);
    driver.present();

    TUINATOR_CHECK(!tuinator::test::expect::snapshot_contains(driver, "line-0"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "line-12"));
}

TUINATOR_TEST(scroll_view_wheel_scrolls_content) {
    auto scroll = std::make_unique<tuinator::ScrollView>(tuinator::test::panes::tall_list_vbox(16, "item"),
                                                         tuinator::ScrollViewOptions{.width = 20, .height = 5});
    scroll->set_test_id("scroll");

    auto root = tuinator::make_screen({.padding = 0});
    root->add_child(std::move(scroll));

    tuinator::testing::AppDriver driver({24, 8}, std::move(root));
    const tuinator::Rect bounds = tuinator::test::panes::bounds_of(driver, "scroll");
    for (int i = 0; i < 6; ++i) {
        driver.wheel_at({bounds.x + 2, bounds.y + 2}, true);
    }
    TUINATOR_CHECK(!tuinator::test::expect::snapshot_contains(driver, "item-0"));
}

TUINATOR_TEST(grid_places_children_in_row_major_order) {
    auto grid = std::make_unique<tuinator::Grid>(tuinator::GridOptions{.columns = 2, .gap = 1, .padding = 0});
    grid->set_test_id("grid");
    grid->add_child(tuinator::test::panes::make_label("A", "a"));
    grid->add_child(tuinator::test::panes::make_label("B", "b"));
    grid->add_child(tuinator::test::panes::make_label("C", "c"));
    grid->add_child(tuinator::test::panes::make_label("D", "d"));

    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(std::move(grid));

    tuinator::testing::AppDriver driver({30, 8}, std::move(root));
    const tuinator::Rect a = tuinator::test::panes::bounds_of(driver, "a");
    const tuinator::Rect b = tuinator::test::panes::bounds_of(driver, "b");
    const tuinator::Rect c = tuinator::test::panes::bounds_of(driver, "c");

    TUINATOR_CHECK(a.y == b.y);
    TUINATOR_CHECK(a.x < b.x);
    TUINATOR_CHECK(c.y > a.y);
}

TUINATOR_TEST(panel_as_pane_exposes_capability) {
    auto root = tuinator::make_screen({.padding = 0});
    root->add_child(tuinator::test::panes::labeled_panel("Pane", tuinator::test::panes::make_label("ok"), "p"));

    tuinator::testing::AppDriver driver({24, 6}, std::move(root));
    TUINATOR_CHECK(tuinator::widget_has_capability(driver.require("p"), tuinator::WidgetCapability::Pane));
}

TUINATOR_TEST(split_pane_edge_clamps_when_first_size_exceeds_width) {
    auto split = std::make_unique<tuinator::SplitPane>(
        tuinator::test::panes::make_label("L", "l"), tuinator::test::panes::make_label("R", "r"),
        tuinator::SplitPaneOptions{
            .orientation = tuinator::SplitOrientation::Horizontal,
            .first_size = 200,
        });
    split->set_test_id("split");

    auto root = tuinator::make_screen({.padding = 0});
    root->add_child(std::move(split));

    tuinator::testing::AppDriver driver({24, 6}, std::move(root));
    const tuinator::Rect left = tuinator::test::panes::bounds_of(driver, "l");
    const tuinator::Rect right = tuinator::test::panes::bounds_of(driver, "r");

    // first_size is clamped to inner width minus divider; the second pane may collapse to zero.
    TUINATOR_CHECK_EQ(left.width, 23);
    TUINATOR_CHECK_EQ(right.width, 0);
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "L"));
}
