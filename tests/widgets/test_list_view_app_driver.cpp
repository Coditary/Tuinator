#include <tuinator/core/screen.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/testing/app_driver.hpp>
#include <tuinator/testing/widget_query.hpp>
#include <tuinator/widgets/views/list_view.hpp>

#include "component_expect.hpp"
#include "test_harness.hpp"

#include <memory>

TUINATOR_TEST(list_view_selected_row_has_uniform_background) {
    const tuinator::Theme theme = tuinator::dark_theme();
    auto list = std::make_unique<tuinator::ListView>(theme.label, theme.button_focused);
    list->set_items({"identitaet          27d"});
    list->set_test_id("sidebar");
    list->set_selected_index(0);

    tuinator::testing::AppDriver driver({48, 1}, std::move(list));
    const tuinator::testing::RegionStyleAnalysis analysis = driver.analyze_selector("sidebar");
    TUINATOR_CHECK(analysis.uniform_background);
    TUINATOR_CHECK(analysis.left_background.has_value());
    TUINATOR_CHECK(analysis.right_background.has_value());
    TUINATOR_CHECK_EQ(*analysis.left_background, *analysis.right_background);
}

TUINATOR_TEST(list_view_hover_marks_widget_and_repaints_row) {
    const tuinator::Theme theme = tuinator::dark_theme();
    auto list = std::make_unique<tuinator::ListView>(theme.label, theme.button_focused);
    list->set_items({"hover-row"});
    list->set_test_id("list");

    tuinator::testing::AppDriver driver({24, 1}, std::move(list));
    driver.hover("list");

    const tuinator::Widget* widget = tuinator::testing::find_by_selector(driver.root(), "list");
    TUINATOR_CHECK(widget != nullptr);
    TUINATOR_CHECK(widget->is_hovered());
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "hover-row"));
}
