#include "render_helper.hpp"
#include "test_harness.hpp"

#include <tuinator/widgets/charts/bar_chart.hpp>

TUINATOR_TEST(bar_chart_renders_vertical_blocks) {
    tuinator::MemoryTerminalBackend backend({40, 14});
    backend.init();

    tuinator::BarChart chart(
        {
            {"Mon", 42.0, tuinator::Style{.foreground = tuinator::Color::Cyan}},
            {"Tue", 68.0, tuinator::Style{.foreground = tuinator::Color::Green}},
            {"Wed", 35.0, tuinator::Style{.foreground = tuinator::Color::Yellow}},
        },
        tuinator::BarChartOptions{
            .style = tuinator::ChartGlyphStyle::Blocks,
            .min_width = 24,
            .min_height = 10,
        });

    tuinator::test::render_root(chart, backend);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 11, "Mon"));
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 11, "Tue"));
}

TUINATOR_TEST(bar_chart_horizontal_orientation) {
    tuinator::BarChart chart(
        {
            {"CPU", 72.0, {}},
            {"GPU", 58.0, {}},
        },
        tuinator::BarChartOptions{
            .orientation = tuinator::BarChartOrientation::Horizontal,
            .min_width = 30,
            .min_height = 6,
        });

    const tuinator::Size size = chart.preferred_size();
    TUINATOR_CHECK(size.height >= 6);
}

TUINATOR_TEST(bar_chart_interactive_wheel) {
    tuinator::BarChart chart(
        {
            {"A", 50.0, {}},
            {"B", 30.0, {}},
        },
        tuinator::BarChartOptions{.interactive = true, .min_width = 20, .min_height = 8});

    const double before = chart.bars()[0].value;
    chart.handle_event(tuinator::MouseEvent{
        .position = {10, 4},
        .action = tuinator::MouseAction::WheelUp,
    });
    TUINATOR_CHECK(chart.bars()[0].value > before);
}

TUINATOR_TEST(bar_chart_braille_style_catalog) {
    bool found = false;
    for (const tuinator::ChartGlyphStyleInfo& info : tuinator::all_chart_glyph_styles()) {
        if (info.style == tuinator::ChartGlyphStyle::Braille) {
            found = true;
        }
    }
    TUINATOR_CHECK(found);
}
