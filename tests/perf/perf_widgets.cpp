#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/layout/box.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/containers/scroll_view.hpp>
#include <tuinator/widgets/containers/split_pane.hpp>
#include <tuinator/widgets/containers/tabs.hpp>
#include <tuinator/widgets/display/label.hpp>
#include <tuinator/widgets/views/list_view.hpp>
#include <tuinator/widgets/views/table.hpp>

#include <memory>
#include <string>
#include <vector>

#include "bench_harness.hpp"
#include "param_perf.hpp"
#include "render_helper.hpp"

namespace {

const tuinator::Theme& theme() {
    static const tuinator::Theme k = tuinator::dark_theme();
    return k;
}

struct TerminalCase {
    const char* label;
    int width;
    int height;
    double budget_ms;
};

constexpr TerminalCase kTerminals[] = {
    {"80x24", 80, 24, 35.0},
    {"120x40", 120, 40, 55.0},
    {"200x50", 200, 50, 90.0},
};

void paint_widget(tuinator::Widget& widget, tuinator::MemoryTerminalBackend& backend) {
    const tuinator::Size term = backend.terminal_size();
    widget.layout({0, 0, term.width, term.height});
    tuinator::test::render_root(widget, backend);
}

std::vector<std::string> make_items(int count) {
    std::vector<std::string> items;
    items.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        items.push_back("entry-" + std::to_string(i));
    }
    return items;
}

} // namespace

TUINATOR_PERF_TEST(param_list_view_full_paint_table) {
    bool all_ok = true;
    TUINATOR_PERF_TABLE(list_paint, TerminalCase, kTerminals, 2, 18, {
        tuinator::MemoryTerminalBackend backend({_tuinator_perf_row.width, _tuinator_perf_row.height});
        backend.init();
        tuinator::ListView list(theme().label, theme().button_focused);
        list.set_items(make_items(120));
        paint_widget(list, backend);
    });
    return all_ok;
}

TUINATOR_PERF_TEST(param_list_view_item_count_scaling) {
    bool all_ok = true;
    for (int count : {25, 100, 250, 500}) {
        const std::string label = "items_" + std::to_string(count);
        const double budget = 8.0 + static_cast<double>(count) * 0.12;
        TUINATOR_PERF_PARAM_CASE(list_scaling, label.c_str(), budget, 2, 15, {
            tuinator::MemoryTerminalBackend backend({100, 30});
            backend.init();
            tuinator::ListView list(theme().label, theme().button_focused);
            list.set_items(make_items(count));
            paint_widget(list, backend);
        });
    }
    return all_ok;
}

TUINATOR_PERF_TEST(param_table_many_rows_paint) {
    bool all_ok = true;
    TUINATOR_PERF_TABLE(table_paint, TerminalCase, kTerminals, 2, 16, {
        tuinator::MemoryTerminalBackend backend({_tuinator_perf_row.width, _tuinator_perf_row.height});
        backend.init();
        tuinator::Table table;
        table.set_columns({{"Name", 16}, {"Value", 8}});
        std::vector<std::vector<std::string>> rows;
        for (int i = 0; i < 80; ++i) {
            rows.push_back({"row-" + std::to_string(i), std::to_string(i)});
        }
        table.set_rows(std::move(rows));
        paint_widget(table, backend);
    });
    return all_ok;
}

TUINATOR_PERF_TEST(param_scroll_view_vertical_scroll_paint) {
    bool all_ok = true;
    for (int rows : {40, 120, 240}) {
        const std::string label = "rows_" + std::to_string(rows);
        const double budget = 15.0 + static_cast<double>(rows) * 0.08;
        TUINATOR_PERF_PARAM_CASE(scroll_paint, label.c_str(), budget, 2, 14, {
            tuinator::MemoryTerminalBackend backend({72, 20});
            backend.init();
            auto content = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
            for (int i = 0; i < rows; ++i) {
                content->add_child(std::make_unique<tuinator::Label>("line-" + std::to_string(i)));
            }
            tuinator::ScrollView scroll(std::move(content), tuinator::ScrollViewOptions{.height = 12});
            scroll.scroll_to(0, rows / 3);
            paint_widget(scroll, backend);
        });
    }
    return all_ok;
}

TUINATOR_PERF_TEST(param_split_pane_layout_paint) {
    bool all_ok = true;
    for (int first : {10, 30, 60, 120}) {
        const std::string label = "first_" + std::to_string(first);
        TUINATOR_PERF_PARAM_CASE(split_paint, label.c_str(), 45.0, 2, 20, {
            tuinator::MemoryTerminalBackend backend({140, 36});
            backend.init();
            auto split = std::make_unique<tuinator::SplitPane>(
                std::make_unique<tuinator::Label>("left-pane"),
                std::make_unique<tuinator::Label>("right-pane"),
                tuinator::SplitPaneOptions{
                    .orientation = tuinator::SplitOrientation::Horizontal,
                    .first_size = first,
                });
            paint_widget(*split, backend);
        });
    }
    return all_ok;
}

TUINATOR_PERF_TEST(param_tabs_switch_layout_paint) {
    bool all_ok = true;
    for (int tab_index = 0; tab_index < 4; ++tab_index) {
        const std::string label = "tab_" + std::to_string(tab_index);
        TUINATOR_PERF_PARAM_CASE(tabs_paint, label.c_str(), 40.0, 2, 18, {
            tuinator::MemoryTerminalBackend backend({96, 28});
            backend.init();
            tuinator::Tabs tabs;
            for (int i = 0; i < 4; ++i) {
                auto panel = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
                for (int line = 0; line < 8; ++line) {
                    panel->add_child(std::make_unique<tuinator::Label>("tab" + std::to_string(i) + "-line" +
                                                                        std::to_string(line)));
                }
                tabs.add_tab("T" + std::to_string(i), std::move(panel));
            }
            tabs.set_selected_index(tab_index);
            paint_widget(tabs, backend);
        });
    }
    return all_ok;
}
