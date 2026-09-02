#include "render_helper.hpp"
#include "test_harness.hpp"

#include <tuinator/widgets/charts/pie_chart.hpp>

#include <cmath>

TUINATOR_TEST(pie_chart_renders_legend) {
    tuinator::MemoryTerminalBackend backend({40, 16});
    backend.init();

    tuinator::PieChart chart(
        {
            {"Rust", 44.0, tuinator::Style{.foreground = tuinator::Color::Red}},
            {"Go", 34.0, tuinator::Style{.foreground = tuinator::Color::Blue}},
            {"Python", 22.0, tuinator::Style{.foreground = tuinator::Color::Green}},
        },
        tuinator::PieChartOptions{.style = tuinator::PieChartStyle::Dots, .diameter = 10});

    tuinator::test::render_root(chart, backend);

    TUINATOR_CHECK(tuinator::test::row_has(backend, "Rust"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "44.0%"));
}

TUINATOR_TEST(pie_chart_style_catalog_contains_braille) {
    bool found = false;
    for (const tuinator::PieChartStyleInfo& info : tuinator::all_pie_chart_styles()) {
        if (info.style == tuinator::PieChartStyle::Braille) {
            found = true;
            TUINATOR_CHECK(std::string(info.id) == "braille");
        }
    }
    TUINATOR_CHECK(found);
}

TUINATOR_TEST(pie_chart_adjust_slice_boundary) {
    tuinator::PieChart chart(
        {
            {"A", 50.0, {}},
            {"B", 50.0, {}},
        },
        tuinator::PieChartOptions{.interactive = true});

    const auto before = chart.slices();
    TUINATOR_CHECK_EQ(before.size(), 2U);

    chart.set_focused(true);
    chart.handle_event(tuinator::KeyPress{tuinator::Key::Right});
    const auto after = chart.slices();

    TUINATOR_CHECK(after[0].value > before[0].value);
    TUINATOR_CHECK(after[1].value < before[1].value);
    TUINATOR_CHECK(std::abs((after[0].value + after[1].value) - 100.0) < 0.001);
}

TUINATOR_TEST(pie_chart_braille_glyph_helper) {
    const std::string glyph = tuinator::pie_chart_glyph_for(tuinator::PieChartStyle::Dots);
    TUINATOR_CHECK(!glyph.empty());
}
