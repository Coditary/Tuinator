#pragma once

#include <tuinator/widgets/widget.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace tuinator::testing {

/// Effective test selector: `test_id()` if set, otherwise `widget_id()`.
std::string_view selector_for(const Widget& widget);

Widget* find_by_selector(Widget& root, std::string_view selector);
const Widget* find_by_selector(const Widget& root, std::string_view selector);

/// All widgets whose selector matches (normally zero or one).
std::vector<Widget*> find_all_by_selector(Widget& root, std::string_view selector);

Widget* require_by_selector(Widget& root, std::string_view selector);

} // namespace tuinator::testing
