#pragma once

#include <tuinator/testing/app_driver.hpp>
#include <tuinator/testing/widget_query.hpp>

#include <string>

namespace tuinator::test::expect {

inline bool tree_contains(const std::string& json, std::string_view needle) {
    return json.find(needle) != std::string::npos;
}

inline bool snapshot_contains(const testing::AppDriver& driver, std::string_view needle) {
    return driver.snapshot_text().find(needle) != std::string::npos;
}

inline bool widget_checked(const testing::AppDriver& driver, std::string_view id) {
    const std::string tree = driver.ui_tree_json();
    const std::string marker = std::string("\"id\":\"") + std::string(id) + "\"";
    const std::size_t pos = tree.find(marker);
    if (pos == std::string::npos) {
        return false;
    }
    const std::size_t checked = tree.find("\"checked\":true", pos);
    const std::size_t next_id = tree.find("\"id\":", pos + marker.size());
    if (checked == std::string::npos) {
        return false;
    }
    return next_id == std::string::npos || checked < next_id;
}

inline int widget_bounds_width(const testing::AppDriver& driver, std::string_view id) {
    const Widget* widget = testing::find_by_selector(driver.root(), id);
    if (widget == nullptr) {
        return -1;
    }
    return widget->bounds().width;
}

} // namespace tuinator::test::expect
