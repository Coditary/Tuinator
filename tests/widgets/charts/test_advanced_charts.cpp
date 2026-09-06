#include <tuinator/widgets/charts/candlestick_chart.hpp>
#include <tuinator/widgets/charts/gauge_chart.hpp>
#include <tuinator/widgets/charts/heatmap.hpp>
#include <tuinator/widgets/charts/histogram.hpp>
#include <tuinator/widgets/charts/stacked_area_chart.hpp>
#include <tuinator/widgets/charts/waterfall_chart.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(histogram_renders_bins) {
    tuinator::MemoryTerminalBackend backend({36, 12});
    backend.init();

    tuinator::Histogram chart(
        {
            {"A", 10.0, {}},
            {"B", 25.0, {}},
            {"C", 15.0, {}},
        },
        tuinator::HistogramOptions{.min_width = 24, .min_height = 8});

    tuinator::test::render_root(chart, backend);
    TUINATOR_CHECK(tuinator::test::row_has(backend, "A"));
}

TUINATOR_TEST(heatmap_renders_grid) {
    tuinator::MemoryTerminalBackend backend({30, 10});
    backend.init();

    tuinator::Heatmap chart({{1.0, 2.0}, {3.0, 4.0}}, {"R0", "R1"}, {"C0", "C1"},
                            tuinator::HeatmapOptions{.min_width = 20, .min_height = 6});

    tuinator::test::render_root(chart, backend);
    TUINATOR_CHECK(tuinator::test::row_has(backend, "R0"));
}

TUINATOR_TEST(gauge_chart_renders_value) {
    tuinator::MemoryTerminalBackend backend({24, 10});
    backend.init();

    tuinator::GaugeChart chart(72.0, tuinator::GaugeChartOptions{
                                         .style = tuinator::GaugeStyle::Horizontal,
                                         .unit = "%",
                                         .diameter = 20,
                                     });

    tuinator::test::render_root(chart, backend);

    bool found = false;
    for (int y = 0; y < backend.terminal_size().height; ++y) {
        if (tuinator::test::row_contains(backend, y, "72")) {
            found = true;
        }
    }
    TUINATOR_CHECK(found);
}

TUINATOR_TEST(stacked_area_chart_renders) {
    tuinator::MemoryTerminalBackend backend({40, 10});
    backend.init();

    tuinator::StackedAreaChart chart(
        {
            {"A", {1.0, 2.0, 3.0}, tuinator::Style{.foreground = tuinator::Color::Red}},
            {"B", {2.0, 2.0, 2.0}, tuinator::Style{.foreground = tuinator::Color::Green}},
        },
        tuinator::StackedAreaChartOptions{.min_width = 24, .min_height = 8});

    tuinator::test::render_root(chart, backend);
    TUINATOR_CHECK(chart.series().size() == 2U);
}

TUINATOR_TEST(waterfall_chart_renders_steps) {
    tuinator::MemoryTerminalBackend backend({40, 12});
    backend.init();

    tuinator::WaterfallChart chart(
        {
            {"Start", 100.0, {}, {}},
            {"Delta", -20.0, {}, {}},
        },
        tuinator::WaterfallChartOptions{.bar_width = 6, .min_width = 28, .min_height = 10});

    tuinator::test::render_root(chart, backend);
    TUINATOR_CHECK(tuinator::test::row_has(backend, "Start"));
}

TUINATOR_TEST(candlestick_chart_renders_ohlc) {
    tuinator::MemoryTerminalBackend backend({40, 12});
    backend.init();

    tuinator::CandlestickChart chart(
        {
            {"Mon", 10.0, 12.0, 9.0, 11.0, 100.0, {}, {}},
            {"Tue", 11.0, 13.0, 10.0, 12.0, 120.0, {}, {}},
        },
        tuinator::CandlestickChartOptions{.min_width = 24, .min_height = 10});

    tuinator::test::render_root(chart, backend);
    TUINATOR_CHECK(chart.bars().size() == 2U);
}
