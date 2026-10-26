#include <tuinator/core/geometry.hpp>
#include <tuinator/core/screen.hpp>
#include <tuinator/layout/box.hpp>
#include <tuinator/layout/grid.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/testing/app_driver.hpp>
#include <tuinator/testing/widget_query.hpp>
#include <tuinator/widgets/containers/scroll_view.hpp>
#include <tuinator/widgets/containers/split_pane.hpp>
#include <tuinator/widgets/containers/tabs.hpp>
#include <tuinator/widgets/controls/combo_box.hpp>
#include <tuinator/widgets/controls/text_input.hpp>
#include <tuinator/widgets/views/list_view.hpp>
#include <tuinator/widgets/views/table.hpp>

#include "component_expect.hpp"
#include "pane_test_helpers.hpp"
#include "test_harness.hpp"

#include <memory>

namespace {

const tuinator::Theme& theme() {
    static const tuinator::Theme k_theme = tuinator::dark_theme();
    return k_theme;
}

std::unique_ptr<tuinator::Widget> make_list(std::vector<std::string> items, std::string test_id) {
    auto list = std::make_unique<tuinator::ListView>(theme().label, theme().button_focused);
    list->set_items(std::move(items));
    list->set_test_id(std::move(test_id));
    return list;
}

bool bounds_overlap(tuinator::Rect a, tuinator::Rect b) {
    const tuinator::Rect overlap = tuinator::intersect(a, b);
    return overlap.width > 0 && overlap.height > 0;
}

std::unique_ptr<tuinator::Widget> make_table(std::string test_id) {
    auto table = std::make_unique<tuinator::Table>();
    table->set_columns({{"Item", 12}, {"Count", 6}});
    table->set_rows({{"widgets", "42"}, {"tests", "99"}});
    table->set_test_id(std::move(test_id));
    return table;
}

} // namespace

TUINATOR_TEST(integration_tabs_panel_list_only_active_tab_is_painted) {
    auto tabs = std::make_unique<tuinator::Tabs>();
    tabs->add_tab("Files",
                  tuinator::test::panes::labeled_panel("F", make_list({"alpha.cpp"}, "list-a"), "panel-a"));
    tabs->add_tab("Search",
                  tuinator::test::panes::labeled_panel("S", tuinator::test::panes::make_label("query-results", "lbl-b"),
                                                       "panel-b"));
    tabs->set_test_id("tabs");

    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(std::move(tabs));

    tuinator::testing::AppDriver driver({56, 14}, std::move(root));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "alpha.cpp"));
    TUINATOR_CHECK(!tuinator::test::expect::snapshot_contains(driver, "query-results"));

    driver.focus("tabs");
    driver.press_key(tuinator::Key::Right);
    driver.present();
    TUINATOR_CHECK(!tuinator::test::expect::snapshot_contains(driver, "alpha.cpp"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "query-results"));

    TUINATOR_CHECK(tuinator::testing::find_by_selector(driver.root(), "lbl-b") != nullptr);
    driver.press_key(tuinator::Key::Left);
    driver.present();
    TUINATOR_CHECK(tuinator::testing::find_by_selector(driver.root(), "list-a") != nullptr);
}

TUINATOR_TEST(integration_split_list_and_text_input_do_not_clobber_each_other) {
    auto left = tuinator::test::panes::labeled_panel("Nav", make_list({"nav-one", "nav-two"}, "nav-list"), "left");

    auto field = std::make_unique<tuinator::TextInput>(tuinator::TextInputOptions{.min_width = 12});
    field->set_test_id("editor");
    auto right = tuinator::test::panes::labeled_panel("Edit", std::move(field), "right");

    auto split = std::make_unique<tuinator::SplitPane>(
        std::move(left), std::move(right),
        tuinator::SplitPaneOptions{.orientation = tuinator::SplitOrientation::Horizontal, .first_size = 22});
    split->set_flex(1);

    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(std::move(split));

    tuinator::testing::AppDriver driver({72, 12}, std::move(root));
    driver.click("nav-list");
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "nav-two"));

    driver.type_into("editor", "typed");
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "typed"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "nav-two"));
}

TUINATOR_TEST(integration_grid_panel_cells_have_disjoint_bounds) {
    auto grid = std::make_unique<tuinator::Grid>(tuinator::GridOptions{.columns = 2, .gap = 1, .padding = 0});
    grid->set_test_id("grid");
    grid->add_child(tuinator::test::panes::labeled_panel("A", tuinator::test::panes::make_label("a", "cell-a")));
    grid->add_child(tuinator::test::panes::labeled_panel("B", tuinator::test::panes::make_label("b", "cell-b")));
    grid->add_child(tuinator::test::panes::labeled_panel("C", tuinator::test::panes::make_label("c", "cell-c")));
    grid->add_child(tuinator::test::panes::labeled_panel("D", tuinator::test::panes::make_label("d", "cell-d")));

    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(std::move(grid));

    tuinator::testing::AppDriver driver({60, 16}, std::move(root));
    const tuinator::Rect a = tuinator::test::panes::bounds_of(driver, "cell-a");
    const tuinator::Rect b = tuinator::test::panes::bounds_of(driver, "cell-b");
    const tuinator::Rect c = tuinator::test::panes::bounds_of(driver, "cell-c");
    const tuinator::Rect d = tuinator::test::panes::bounds_of(driver, "cell-d");
    TUINATOR_CHECK(a.width > 0 && b.width > 0);

    TUINATOR_CHECK(!bounds_overlap(a, b));
    TUINATOR_CHECK(!bounds_overlap(a, c));
    TUINATOR_CHECK(!bounds_overlap(b, d));
    TUINATOR_CHECK(a.x < b.x);
    TUINATOR_CHECK(a.y < c.y);
}

TUINATOR_TEST(integration_nested_scroll_wheel_does_not_hide_split_sibling) {
    auto column = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    column->add_child(tuinator::test::panes::make_label("sticky-header", "hdr"));
    auto inner_scroll = std::make_unique<tuinator::ScrollView>(
        tuinator::test::panes::tall_list_vbox(30, "row"),
        tuinator::ScrollViewOptions{.width = 18, .height = 6});
    inner_scroll->set_test_id("inner-scroll");
    column->add_child(std::move(inner_scroll));
    auto main = tuinator::test::panes::labeled_panel("Main", std::move(column), "main");
    auto side = tuinator::test::panes::labeled_panel("Side", tuinator::test::panes::make_label("side-ok", "side"), "side");

    auto split = std::make_unique<tuinator::SplitPane>(
        std::move(main), std::move(side),
        tuinator::SplitPaneOptions{.orientation = tuinator::SplitOrientation::Horizontal, .first_size = 26});
    split->set_test_id("split");
    split->set_flex(1);

    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(std::move(split));

    tuinator::testing::AppDriver driver({64, 14}, std::move(root));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "row-0"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "sticky-header"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "side-ok"));

    tuinator::ScrollView* scroll = tuinator::test::panes::as_scroll(driver, "inner-scroll");
    TUINATOR_CHECK(scroll != nullptr);
    const tuinator::Rect scroll_bounds = scroll->bounds();
    for (int i = 0; i < 8; ++i) {
        driver.wheel_at({scroll_bounds.x + 2, scroll_bounds.y + 2}, true);
    }

    TUINATOR_CHECK(!tuinator::test::expect::snapshot_contains(driver, "row-0"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "sticky-header"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "side-ok"));
}

TUINATOR_TEST(integration_tabs_with_split_list_and_table_survives_resize) {
    auto list_tab = std::make_unique<tuinator::SplitPane>(
        tuinator::test::panes::labeled_panel("List", make_list({"entry-a", "entry-b"}, "tab-list"), "list-pane"),
        tuinator::test::panes::labeled_panel("Hint", tuinator::test::panes::make_label("pick-one", "hint"), "hint-pane"),
        tuinator::SplitPaneOptions{.orientation = tuinator::SplitOrientation::Horizontal, .first_size = 20});
    list_tab->set_flex(1);

    auto tabs = std::make_unique<tuinator::Tabs>();
    tabs->add_tab("Browse", std::move(list_tab));
    tabs->add_tab("Data", tuinator::test::panes::labeled_panel("Table", make_table("data-table"), "table-pane"));
    tabs->set_test_id("work-tabs");

    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(std::move(tabs));

    tuinator::testing::AppDriver driver({70, 16}, std::move(root));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "entry-a"));
    TUINATOR_CHECK(!tuinator::test::expect::snapshot_contains(driver, "widgets"));

    driver.focus("work-tabs");
    driver.press_key(tuinator::Key::Right);
    driver.present();
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "widgets"));
    TUINATOR_CHECK(!tuinator::test::expect::snapshot_contains(driver, "entry-a"));

    driver.resize_terminal({48, 12});
    TUINATOR_CHECK(tuinator::testing::find_by_selector(driver.root(), "data-table") != nullptr);
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "widgets"));
}

TUINATOR_TEST(integration_vbox_combo_and_text_input_keep_independent_state) {
    auto combo = std::make_unique<tuinator::ComboBox>(theme().label, theme().button_focused);
    combo->set_items({"Draft", "Published"});
    combo->set_test_id("status-combo");

    auto field = std::make_unique<tuinator::TextInput>(tuinator::TextInputOptions{.min_width = 14});
    field->set_test_id("title-field");

    auto column = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 0});
    column->add_child(std::move(combo));
    column->add_child(std::move(field));

    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(std::move(column));

    tuinator::testing::AppDriver driver({40, 10}, std::move(root));
    driver.focus("status-combo");
    driver.press_key(tuinator::Key::Down);
    driver.type_into("title-field", "My Post");
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Published"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "My Post"));
}

TUINATOR_TEST(integration_split_table_click_leaves_sibling_label_unchanged) {
    auto table = make_table("rows");
    auto left = tuinator::test::panes::labeled_panel("Rows", std::move(table), "left");
    auto right = tuinator::test::panes::labeled_panel("Meta", tuinator::test::panes::make_label("unchanged-meta", "meta"), "right");

    auto split = std::make_unique<tuinator::SplitPane>(
        std::move(left), std::move(right),
        tuinator::SplitPaneOptions{.orientation = tuinator::SplitOrientation::Horizontal, .first_size = 28});
    split->set_flex(1);

    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(std::move(split));

    tuinator::testing::AppDriver driver({60, 12}, std::move(root));
    driver.click("rows");
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "unchanged-meta"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "widgets"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "tests"));
}
