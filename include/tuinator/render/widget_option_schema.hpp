#pragma once

#include <tuinator/render/widget_options.hpp>

#include <string>
#include <vector>

namespace tuinator {

class Widget;

/// Returns warnings for unknown stylesheet options on a widget type.
std::vector<std::string> validate_widget_options(const Widget& widget, const WidgetOptions& options);

} // namespace tuinator
