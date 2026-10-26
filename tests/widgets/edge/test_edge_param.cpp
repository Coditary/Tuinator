#include <tuinator/core/event.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/containers/split_pane.hpp>
#include <tuinator/widgets/containers/tabs.hpp>
#include <tuinator/widgets/controls/slider.hpp>
#include <tuinator/widgets/display/spinner.hpp>
#include <tuinator/widgets/display/label.hpp>
#include <tuinator/widgets/display/progress_bar.hpp>
#include <tuinator/widgets/views/list_view.hpp>
#include <tuinator/widgets/views/table.hpp>

#include "param_test.hpp"
#include "test_harness.hpp"

#include <memory>
#include <string>

namespace {

const tuinator::Theme& theme() {
    static const tuinator::Theme k = tuinator::dark_theme();
    return k;
}

struct IndexClampCase {
    const char* label;
    int select;
    int item_count;
    int expected;
};

constexpr IndexClampCase kListClampCases[] = {
    {"negative_index", -12, 3, 0},
    {"past_end", 99, 3, 2},
    {"empty_list", 5, 0, 0},
    {"single_item_high", 8, 1, 0},
};

struct SliderInitCase {
    const char* label;
    int min_v;
    int max_v;
    int initial;
    int expected;
};

constexpr SliderInitCase kSliderInitCases[] = {
    {"above_max", 0, 10, 42, 10},
    {"below_min", 0, 10, -3, 0},
    {"in_range", 2, 8, 5, 5},
    {"min_equals_max", 7, 7, 1, 7},
};

struct TabsIndexCase {
    const char* label;
    int tab_count;
    int select;
    int expected;
};

constexpr TabsIndexCase kTabsIndexCases[] = {
    {"high", 2, 99, 1},
    {"negative", 3, -4, 0},
    {"single_tab", 1, 5, 0},
};

std::unique_ptr<tuinator::Widget> tab_label(std::string text) {
    return std::make_unique<tuinator::Label>(std::move(text));
}

} // namespace

TUINATOR_TEST(param_list_view_selection_clamp_table) {
    TUINATOR_PARAM_TABLE(list_clamp, IndexClampCase, kListClampCases, {
        tuinator::ListView list(theme().label, theme().button_focused);
        if (_tuinator_param_row.item_count > 0) {
            std::vector<std::string> items;
            for (int i = 0; i < _tuinator_param_row.item_count; ++i) {
                items.push_back("i" + std::to_string(i));
            }
            list.set_items(std::move(items));
        }
        list.set_selected_index(_tuinator_param_row.select);
        TUINATOR_CHECK_EQ(list.selected_index(), _tuinator_param_row.expected);
    });
}

TUINATOR_TEST(param_table_selection_clamp_table) {
    TUINATOR_PARAM_TABLE(table_clamp, IndexClampCase, kListClampCases, {
        tuinator::Table table;
        table.set_columns({{"c", 6}});
        if (_tuinator_param_row.item_count > 0) {
            std::vector<std::vector<std::string>> rows;
            for (int i = 0; i < _tuinator_param_row.item_count; ++i) {
                rows.push_back({std::to_string(i)});
            }
            table.set_rows(std::move(rows));
        }
        table.set_selected_row(_tuinator_param_row.select);
        TUINATOR_CHECK_EQ(table.selected_row(), _tuinator_param_row.expected);
    });
}

TUINATOR_TEST(param_slider_initial_value_table) {
    TUINATOR_PARAM_TABLE(slider_init, SliderInitCase, kSliderInitCases, {
        tuinator::Slider slider(_tuinator_param_row.min_v, _tuinator_param_row.max_v, _tuinator_param_row.initial);
        TUINATOR_CHECK_EQ(slider.value(), _tuinator_param_row.expected);
    });
}

TUINATOR_TEST(param_tabs_selected_index_table) {
    TUINATOR_PARAM_TABLE(tabs_index, TabsIndexCase, kTabsIndexCases, {
        tuinator::Tabs tabs;
        for (int i = 0; i < _tuinator_param_row.tab_count; ++i) {
            tabs.add_tab("T" + std::to_string(i), tab_label("x" + std::to_string(i)));
        }
        tabs.set_selected_index(_tuinator_param_row.select);
        TUINATOR_CHECK_EQ(tabs.selected_index(), _tuinator_param_row.expected);
    });
}

TUINATOR_TEST(param_split_horizontal_first_size_matrix) {
    for (int width = 10; width <= 34; width += 8) {
        for (int first : {0, 1, 5, 1000}) {
            const std::string label = "w" + std::to_string(width) + "_f" + std::to_string(first);
            TUINATOR_PARAM_CASE(label, {
                auto split = std::make_unique<tuinator::SplitPane>(
                    tab_label("L"), tab_label("R"),
                    tuinator::SplitPaneOptions{
                        .orientation = tuinator::SplitOrientation::Horizontal,
                        .first_size = first,
                    });
                split->layout({0, 0, width, 5});
                tuinator::Widget* left = split->first_child();
                TUINATOR_CHECK(left != nullptr);
                TUINATOR_CHECK(left->bounds().width >= 0);
                TUINATOR_CHECK(left->bounds().width < width);
            });
        }
    }
}

TUINATOR_TEST(param_spinner_step_clamp_matrix) {
    for (int start = 0; start <= 4; ++start) {
        for (int steps : {0, 1, 3, 20}) {
            const std::string label = "start" + std::to_string(start) + "_steps" + std::to_string(steps);
            TUINATOR_PARAM_CASE(label, {
                tuinator::Spinner spinner(0, 4, start);
                spinner.layout({0, 0, 14, 1});
                spinner.set_focused(true);
                for (int i = 0; i < steps; ++i) {
                    spinner.handle_event(tuinator::KeyPress{tuinator::Key::Right});
                }
                TUINATOR_CHECK(spinner.value() >= 0);
                TUINATOR_CHECK(spinner.value() <= 4);
            });
        }
    }
}

TUINATOR_PARAM_TEST(list_clamp, shrink_items_resets_index, {
    tuinator::ListView list(theme().label, theme().button_focused);
    list.set_items({"a", "b", "c"});
    list.set_selected_index(2);
    list.set_items({"solo"});
    TUINATOR_CHECK_EQ(list.selected_index(), 0);
});

TUINATOR_PARAM_TEST(progress_clamp, unit_interval, {
    tuinator::ProgressBar bar;
    bar.set_value(2.5);
    TUINATOR_CHECK_EQ(bar.value(), 1.0);
    bar.set_value(-1.0);
    TUINATOR_CHECK_EQ(bar.value(), 0.0);
});
