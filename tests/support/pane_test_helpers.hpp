#pragma once

#include <tuinator/core/screen.hpp>
#include <tuinator/layout/box.hpp>
#include <tuinator/layout/grid.hpp>
#include <tuinator/testing/app_driver.hpp>
#include <tuinator/testing/widget_query.hpp>
#include <tuinator/widgets/containers/panel.hpp>
#include <tuinator/widgets/containers/scroll_view.hpp>
#include <tuinator/widgets/containers/split_pane.hpp>
#include <tuinator/widgets/display/label.hpp>

#include <memory>
#include <string>
#include <vector>

namespace tuinator::test::panes {

inline std::unique_ptr<Widget> labeled_panel(std::string title, std::unique_ptr<Widget> content,
                                             std::string test_id = {}) {
    auto panel = std::make_unique<Panel>(std::move(title));
    if (!test_id.empty()) {
        panel->set_test_id(std::move(test_id));
    }
    panel->set_content(std::move(content));
    return panel;
}

inline std::unique_ptr<Widget> make_label(std::string text, std::string test_id = {}) {
    auto label = std::make_unique<Label>(std::move(text));
    if (!test_id.empty()) {
        label->set_test_id(std::move(test_id));
    }
    return label;
}

inline std::unique_ptr<Widget> tall_list_vbox(int rows, std::string prefix = "row") {
    auto list = std::make_unique<VBox>(BoxOptions{.gap = 0});
    for (int i = 0; i < rows; ++i) {
        list->add_child(make_label(prefix + "-" + std::to_string(i)));
    }
    return list;
}

inline Rect bounds_of(const testing::AppDriver& driver, std::string_view id) {
    const Widget* widget = testing::find_by_selector(driver.root(), id);
    if (widget == nullptr) {
        return {};
    }
    return widget->bounds();
}

inline ScrollView* as_scroll(testing::AppDriver& driver, std::string_view id) {
    Widget& widget = driver.require(id);
    return dynamic_cast<ScrollView*>(&widget);
}

inline SplitPane* as_split(testing::AppDriver& driver, std::string_view id) {
    Widget& widget = driver.require(id);
    return dynamic_cast<SplitPane*>(&widget);
}

inline Panel* as_panel(testing::AppDriver& driver, std::string_view id) {
    Widget& widget = driver.require(id);
    return dynamic_cast<Panel*>(&widget);
}

} // namespace tuinator::test::panes
