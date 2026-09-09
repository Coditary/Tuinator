#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/render/stylesheet.hpp>
#include <tuinator/widgets/charts/chart_widget.hpp>
#include <tuinator/widgets/charts/line_chart.hpp>

#include <cmath>
#include <vector>

#include "render_helper.hpp"
#include "test_harness.hpp"

namespace {

std::vector<double> sine_points(int count) {
    std::vector<double> values;
    values.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        const double t = static_cast<double>(i) / std::max(1, count - 1);
        values.push_back(std::sin(t * 6.283185307179586 * 2.0));
    }
    return values;
}

} // namespace

TUINATOR_TEST(line_chart_renders_line_mode) {
    tuinator::MemoryTerminalBackend backend({40, 12});
    backend.init();

    tuinator::LineChart chart({{"sin", sine_points(40), tuinator::Style{.foreground = tuinator::Color::Red}}},
                              tuinator::LineChartOptions{
                                  .style = tuinator::ChartGlyphStyle::Dots,
                                  .mode = tuinator::LineChartMode::Line,
                                  .min_width = 30,
                                  .min_height = 8,
                              });

    tuinator::test::render_root(chart, backend);

    bool has_marker = false;
    for (int y = 0; y < backend.terminal_size().height; ++y) {
        for (int x = 0; x < backend.terminal_size().width; ++x) {
            const char ch = tuinator::test::cell_at(backend, x, y);
            if (ch == 'O' || ch == '*' || ch == '.' || ch == ':') {
                has_marker = true;
            }
        }
    }
    TUINATOR_CHECK(has_marker);
}

TUINATOR_TEST(line_chart_push_value_keeps_max_points) {
    tuinator::LineChart chart({{"live", std::vector<double>{}, {}}},
                              tuinator::LineChartOptions{.min_width = 20, .min_height = 6});

    for (int i = 0; i < 20; ++i) {
        chart.push_value(0, static_cast<double>(i), 10);
    }

    TUINATOR_CHECK_EQ(chart.series()[0].values.size(), 10U);
    TUINATOR_CHECK_EQ(chart.series()[0].values.front(), 10.0);
}

TUINATOR_TEST(line_chart_sparkline_mode) {
    tuinator::LineChart chart({{"srv", sine_points(30), {}}}, tuinator::LineChartOptions{
                                                                  .mode = tuinator::LineChartMode::Sparkline,
                                                                  .show_axes = false,
                                                                  .show_grid = false,
                                                                  .min_width = 24,
                                                                  .min_height = 4,
                                                              });

    const tuinator::Size size = chart.preferred_size();
    TUINATOR_CHECK(size.height >= 4);
}

TUINATOR_TEST(line_chart_mirror_mode_renders) {
    tuinator::MemoryTerminalBackend backend({40, 12});
    backend.init();

    tuinator::LineChart chart(
        {
            {"up", std::vector<double>{0.2, 0.5, 0.3}, tuinator::Style{.foreground = tuinator::Color::Yellow}},
            {"down", std::vector<double>{-0.1, -0.4, -0.2}, tuinator::Style{.foreground = tuinator::Color::Green}},
        },
        tuinator::LineChartOptions{
            .mode = tuinator::LineChartMode::Line,
            .mirror = true,
            .min_width = 30,
            .min_height = 8,
        });

    tuinator::test::render_root(chart, backend);
    TUINATOR_CHECK(backend.terminal_size().width > 0);
}

TUINATOR_TEST(line_chart_clears_bounds_when_empty) {
    tuinator::MemoryTerminalBackend backend({20, 8});
    backend.init();

    tuinator::LineChart chart({{"sin", sine_points(20), tuinator::Style{.foreground = tuinator::Color::Red}}},
                              tuinator::LineChartOptions{
                                  .style = tuinator::ChartGlyphStyle::Dots,
                                  .mode = tuinator::LineChartMode::Line,
                                  .min_width = 16,
                                  .min_height = 6,
                              });
    chart.layout({0, 0, 20, 8});

    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    chart.paint(ctx);
    TUINATOR_CHECK(tuinator::test::cell_at(backend, 10, 4) != ' ');

    chart.set_series({});
    chart.paint(ctx);
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 10, 4), ' ');
}

TUINATOR_TEST(line_chart_scatter_mode) {
    tuinator::LineChart chart({{"pts", std::vector<double>{1.0, 4.0, 2.0, 5.0, 3.0}, {}}},
                              tuinator::LineChartOptions{
                                  .style = tuinator::ChartGlyphStyle::Stars,
                                  .mode = tuinator::LineChartMode::Scatter,
                                  .min_width = 20,
                                  .min_height = 8,
                              });

    TUINATOR_CHECK_EQ(chart.series()[0].values.size(), 5U);
}

TUINATOR_TEST(line_chart_stylesheet_resolves_paint_styles) {
    const tuinator::Stylesheet sheet = tuinator::Stylesheet::load_from_string(R"(
LineChart {
  color: cyan;
}
)");

    tuinator::LineChart chart({{"sin", sine_points(10), tuinator::Style{}}},
                              tuinator::LineChartOptions{
                                  .mode = tuinator::LineChartMode::Line,
                                  .title = "Series",
                                  .min_width = 20,
                                  .min_height = 8,
                              });

    tuinator::MemoryTerminalBackend backend({20, 8});
    backend.init();
    const tuinator::Theme theme = tuinator::dark_theme();
    tuinator::Canvas canvas(backend);
    const tuinator::PaintContext ctx{canvas, theme, &sheet};

    const tuinator::ChartStyleBundle styles =
        tuinator::resolve_chart_styles(ctx, chart, {}, {}, {}, tuinator::Style{});

    TUINATOR_CHECK_EQ(styles.title.foreground, tuinator::Color::Cyan);
    TUINATOR_CHECK_EQ(styles.axis.foreground, tuinator::Color::Cyan);
    TUINATOR_CHECK_EQ(styles.value.foreground, tuinator::Color::Cyan);
}
