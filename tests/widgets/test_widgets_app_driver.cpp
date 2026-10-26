#include <tuinator/core/screen.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/testing/app_driver.hpp>
#include <tuinator/widgets/capabilities.hpp>
#include <tuinator/widgets/containers/tabs.hpp>
#include <tuinator/widgets/controls/combo_box.hpp>
#include <tuinator/widgets/controls/slider.hpp>
#include <tuinator/widgets/display/label.hpp>
#include <tuinator/widgets/views/list_view.hpp>
#include <tuinator/widgets/views/table.hpp>

#include "component_expect.hpp"
#include "test_harness.hpp"

#include <memory>

TUINATOR_TEST(app_driver_list_view_click_selects_row) {
    const tuinator::Theme theme = tuinator::dark_theme();
    auto list = std::make_unique<tuinator::ListView>(theme.label, theme.button_focused);
    list->set_items({"Alpha", "Beta", "Gamma"});
    list->set_test_id("list");

    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(std::move(list));

    tuinator::testing::AppDriver driver({28, 8}, std::move(root));
    driver.click("list");
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, ">"));
}

TUINATOR_TEST(app_driver_table_renders_headers) {
    auto table = std::make_unique<tuinator::Table>();
    table->set_columns({{"Name", 10}, {"Qty", 6}});
    table->set_rows({{"apple", "3"}});
    table->set_test_id("tbl");

    auto root = tuinator::make_screen({.padding = 0});
    root->add_child(std::move(table));

    tuinator::testing::AppDriver driver({32, 8}, std::move(root));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Name"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "apple"));
}

TUINATOR_TEST(app_driver_tabs_switch_with_number_key) {
    auto tabs = std::make_unique<tuinator::Tabs>();
    tabs->add_tab("A", std::make_unique<tuinator::Label>("tab-a"));
    tabs->add_tab("B", std::make_unique<tuinator::Label>("tab-b"));
    tabs->set_test_id("tabs");

    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(std::move(tabs));

    tuinator::testing::AppDriver driver({40, 10}, std::move(root));
    driver.focus("tabs");
    driver.press_key(tuinator::Key::Right);
    const tuinator::TabHost* host = tuinator::as_tab_host(&driver.require("tabs"));
    TUINATOR_CHECK(host != nullptr);
    TUINATOR_CHECK_EQ(host->tab_count(), 2);
    TUINATOR_CHECK_EQ(dynamic_cast<const tuinator::Tabs&>(driver.require("tabs")).selected_index(), 1);
}

TUINATOR_TEST(app_driver_combo_and_slider_respond_to_focus_keys) {
    auto combo = std::make_unique<tuinator::ComboBox>(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    combo->set_items({"One", "Two"});
    combo->set_test_id("combo");

    auto slider = std::make_unique<tuinator::Slider>(0, 10, 5, std::function<void(int)>{},
                                                     tuinator::Theme{}.label, 12);
    slider->set_test_id("slider");

    auto root = tuinator::make_screen({.gap = 1, .padding = 1});
    root->add_child(std::move(combo));
    root->add_child(std::move(slider));

    tuinator::testing::AppDriver driver({36, 10}, std::move(root));
    driver.focus("combo");
    driver.press_key(tuinator::Key::Down);
    driver.focus("slider");
    driver.press_key(tuinator::Key::Right);

    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Two"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "O"));
}
