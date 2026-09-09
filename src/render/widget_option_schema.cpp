#include <tuinator/render/widget_option_schema.hpp>
#include <tuinator/widgets/widget.hpp>

#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace tuinator {

namespace {

const std::unordered_set<std::string>& common_widget_options() {
    static const std::unordered_set<std::string> options = {"flex"};
    return options;
}

const std::unordered_map<std::string, std::unordered_set<std::string>>& option_schemas() {
    static const std::unordered_map<std::string, std::unordered_set<std::string>> schemas = {
        {"Panel", {"border-top", "border-right", "border-bottom", "border-left"}},
        {"SplitPane", {"outer-border", "first-size", "min-pane-size", "divider-hit-slop", "orientation"}},
        {"VBox", {"gap", "padding"}},
        {"HBox", {"gap", "padding"}},
        {"Grid", {"gap", "padding", "columns", "column-count"}},
        {"TextInput", {"min-width", "placeholder"}},
        {"TextArea", {"min-width", "min-height", "line-numbers", "status-bar", "placeholder", "gutter-width"}},
        {"ScrollView", {"width", "height", "scrollbar-vertical", "scrollbar-horizontal"}},
        {"Slider", {"min-width"}},
        {"ComboBox", {"min-width"}},
        {"Toggle", {}},
        {"TreeView", {}},
        {"ProgressBar", {"min-width", "show-percent"}},
        {"CustomPaint", {"width", "height"}},
        {"BarChart", {"min-width", "min-height", "show-axis", "show-grid", "glyph-style", "custom-glyph"}},
        {"LineChart", {"min-width", "min-height", "show-axis", "show-grid", "glyph-style", "custom-glyph"}},
        {"PieChart", {"glyph-style", "custom-glyph"}},
        {"Histogram", {"min-width", "min-height", "show-axis", "show-grid", "glyph-style", "custom-glyph"}},
        {"Heatmap", {"min-width", "min-height", "glyph-style"}},
        {"GaugeChart", {"glyph-style"}},
        {"CandlestickChart", {"min-width", "min-height", "show-axis", "show-grid"}},
        {"StackedAreaChart", {"min-width", "min-height", "show-axis", "show-grid", "glyph-style"}},
        {"WaterfallChart", {"min-width", "min-height", "show-axis", "show-grid", "glyph-style"}},
    };
    return schemas;
}

bool option_allowed(std::string_view widget_type, std::string_view key) {
    if (common_widget_options().count(std::string(key)) > 0) {
        return true;
    }

    const auto schema = option_schemas().find(std::string(widget_type));
    if (schema == option_schemas().end()) {
        return true;
    }

    return schema->second.count(std::string(key)) > 0;
}

} // namespace

std::vector<std::string> validate_widget_options(const Widget& widget, const WidgetOptions& options) {
    std::vector<std::string> warnings;
    options.for_each([&](std::string_view key, std::string_view /*value*/) {
        if (!option_allowed(widget.widget_type_name(), key)) {
            warnings.push_back(std::string(widget.widget_type_name()) + " does not support option '" +
                               std::string(key) + "'");
        }
    });
    return warnings;
}

} // namespace tuinator
