#include <tuinator/core/screen.hpp>
#include <tuinator/layout/box.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/testing/app_driver.hpp>
#include <tuinator/widgets/containers/split_pane.hpp>
#include <tuinator/widgets/display/label.hpp>
#include <tuinator/widgets/views/list_view.hpp>

#include <memory>
#include <string>
#include <vector>

#include "bench_harness.hpp"
#include "param_perf.hpp"

namespace {

const tuinator::Theme& theme() {
    static const tuinator::Theme k = tuinator::dark_theme();
    return k;
}

std::unique_ptr<tuinator::Screen> nested_shell() {
    auto list = std::make_unique<tuinator::ListView>(theme().label, theme().button_focused);
    std::vector<std::string> items;
    for (int i = 0; i < 60; ++i) {
        items.push_back("file-" + std::to_string(i));
    }
    list->set_items(std::move(items));
    list->set_test_id("files");

    auto left = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    left->add_child(std::make_unique<tuinator::Label>("Explorer"));
    left->add_child(std::move(list));

    auto editor = std::make_unique<tuinator::Label>("editor buffer");

    auto columns = std::make_unique<tuinator::SplitPane>(
        std::move(left), std::move(editor),
        tuinator::SplitPaneOptions{.orientation = tuinator::SplitOrientation::Horizontal, .first_size = 28});

    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(std::move(columns));
    return root;
}

} // namespace

TUINATOR_PERF_TEST(param_app_driver_present_and_snapshot) {
    bool all_ok = true;
    for (int size : {72, 100, 140}) {
        const std::string label = "w" + std::to_string(size);
        const double budget = 25.0 + static_cast<double>(size) * 0.35;
        TUINATOR_PERF_PARAM_CASE(app_present, label.c_str(), budget, 1, 12, {
            std::unique_ptr<tuinator::Widget> root(nested_shell().release());
            tuinator::testing::AppDriver driver({size, 32}, std::move(root));
            driver.present();
            driver.snapshot_text();
            driver.ui_tree_json();
        });
    }
    return all_ok;
}

TUINATOR_PERF_TEST(param_app_driver_click_list_resize) {
    bool all_ok = true;
    TUINATOR_PERF_PARAM_CASE(app_interact, "click_resize_cycle", 120.0, 1, 10, {
        std::unique_ptr<tuinator::Widget> root(nested_shell().release());
        tuinator::testing::AppDriver driver({100, 28}, std::move(root));
        driver.click("files");
        driver.resize_terminal({80, 24});
        driver.present();
        driver.click("files");
        driver.snapshot_text();
    });
    return all_ok;
}
