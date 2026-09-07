#include <tuinator/widgets/charts/bar_chart.hpp>
#include <tuinator/widgets/charts/line_chart.hpp>

#include <vector>

#include "bench_harness.hpp"
#include "render_helper.hpp"

namespace {

tuinator::BarChart make_bar_chart() {
    std::vector<tuinator::BarChartBar> items;
    for (int i = 0; i < 12; ++i) {
        items.push_back({
            "M" + std::to_string(i + 1),
            static_cast<double>((i * 37) % 100),
            tuinator::Style{.foreground = tuinator::Color::Cyan},
        });
    }

    return tuinator::BarChart(std::move(items), tuinator::BarChartOptions{
                                                    .style = tuinator::ChartGlyphStyle::Braille,
                                                    .min_width = 60,
                                                    .min_height = 18,
                                                });
}

tuinator::LineChart make_line_chart() {
    std::vector<tuinator::LineChartSeries> series;
    for (int s = 0; s < 3; ++s) {
        tuinator::LineChartSeries line;
        line.label = "Series " + std::to_string(s + 1);
        for (int i = 0; i < 48; ++i) {
            line.values.push_back(static_cast<double>((i * (s + 3) * 11) % 100));
        }
        series.push_back(std::move(line));
    }

    return tuinator::LineChart(std::move(series), tuinator::LineChartOptions{
                                                      .style = tuinator::ChartGlyphStyle::Dots,
                                                      .show_grid = true,
                                                      .show_legend = true,
                                                      .min_width = 70,
                                                      .min_height = 20,
                                                  });
}

} // namespace

TUINATOR_PERF_TEST(bar_chart_braille_render) {
    tuinator::MemoryTerminalBackend backend({80, 24});
    backend.init();
    tuinator::BarChart chart = make_bar_chart();

    return tuinator::perf::bench("bar_chart_braille_render", 200.0, 2, 25,
                                 [&]() { tuinator::test::render_root(chart, backend); });
}

TUINATOR_PERF_TEST(line_chart_dots_render) {
    tuinator::MemoryTerminalBackend backend({90, 26});
    backend.init();
    tuinator::LineChart chart = make_line_chart();

    return tuinator::perf::bench("line_chart_dots_render", 250.0, 2, 25,
                                 [&]() { tuinator::test::render_root(chart, backend); });
}

TUINATOR_PERF_TEST(chart_gallery_render) {
    tuinator::MemoryTerminalBackend backend({90, 26});
    backend.init();
    tuinator::BarChart bar = make_bar_chart();
    tuinator::LineChart line = make_line_chart();

    return tuinator::perf::bench("chart_gallery_render", 450.0, 2, 15, [&]() {
        tuinator::test::render_root(bar, backend);
        tuinator::test::render_root(line, backend);
    });
}
