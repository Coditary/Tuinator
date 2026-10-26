#include <tuinator/widgets/charts/bar_chart.hpp>
#include <tuinator/widgets/charts/gauge_chart.hpp>
#include <tuinator/widgets/charts/line_chart.hpp>
#include <tuinator/widgets/charts/pie_chart.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

#include <vector>

TUINATOR_TEST(edge_bar_chart_empty_series_renders) {
    tuinator::MemoryTerminalBackend backend({20, 8});
    backend.init();
    tuinator::BarChart chart({}, tuinator::BarChartOptions{.min_width = 12, .min_height = 6});
    chart.layout({0, 0, 20, 8});
    tuinator::test::render_root(chart, backend);
    TUINATOR_CHECK(true);
}

TUINATOR_TEST(edge_line_chart_single_point) {
    tuinator::LineChart chart({tuinator::LineChartSeries{.label = "s", .values = {1.0}}});
    const tuinator::Size size = chart.preferred_size();
    TUINATOR_CHECK(size.width > 0);
    TUINATOR_CHECK(size.height > 0);
}

TUINATOR_TEST(edge_pie_chart_zero_total_slices_safe) {
    tuinator::MemoryTerminalBackend backend({16, 10});
    backend.init();
    tuinator::PieChart chart(
        {
            {"A", 0.0, {}},
            {"B", 0.0, {}},
        },
        tuinator::PieChartOptions{.diameter = 8});
    chart.layout({0, 0, 16, 10});
    tuinator::test::render_root(chart, backend);
    TUINATOR_CHECK(true);
}

TUINATOR_TEST(edge_gauge_chart_value_clamps_to_options) {
    tuinator::GaugeChart gauge(50.0);
    tuinator::GaugeChartOptions options;
    options.min_value = 0.0;
    options.max_value = 100.0;
    gauge.set_options(options);
    gauge.set_value(150.0);
    TUINATOR_CHECK_EQ(gauge.value(), 100.0);
    gauge.set_value(-5.0);
    TUINATOR_CHECK_EQ(gauge.value(), 0.0);
}
