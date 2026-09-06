#include <tuinator/tuinator.hpp>

#include <cmath>
#include <memory>
#include <string>
#include <vector>

namespace {

class ChartsRoot : public tuinator::VBox {
  public:
    explicit ChartsRoot(tuinator::BoxOptions options) : tuinator::VBox(options) {}
    bool wants_full_screen() const override { return true; }
};

struct AnimatedCharts {
    tuinator::LineChart* cpu = nullptr;
    tuinator::LineChart* network = nullptr;
    tuinator::BarChart* spectrum = nullptr;
    tuinator::BarChart* temps = nullptr;
    double tick = 0.0;
};

tuinator::Style color(tuinator::Color c, bool bold = true) { return tuinator::Style{.foreground = c, .bold = bold}; }

std::vector<double> sine_wave(int points, double phase, double amplitude, double offset) {
    std::vector<double> values;
    values.reserve(static_cast<std::size_t>(points));
    for (int i = 0; i < points; ++i) {
        const double t = static_cast<double>(i) / std::max(1, points - 1);
        values.push_back(offset + std::sin(t * 6.283185307179586 * 2.0 + phase) * amplitude);
    }
    return values;
}

std::vector<double> randomish_wave(int points, double seed) {
    std::vector<double> values;
    values.reserve(static_cast<std::size_t>(points));
    for (int i = 0; i < points; ++i) {
        const double t = static_cast<double>(i) / 8.0 + seed;
        values.push_back(20.0 + std::sin(t) * 15.0 + std::sin(t * 2.3) * 8.0 + std::cos(t * 0.7) * 5.0);
    }
    return values;
}

std::unique_ptr<tuinator::Panel> panel(const std::string& title, const tuinator::Theme& theme,
                                       std::unique_ptr<tuinator::Widget> content) {
    auto box = std::make_unique<tuinator::Panel>(title, theme.border, theme.heading);
    box->set_content(std::move(content));
    return box;
}

void add_section_header(tuinator::VBox& parent, const std::string& text, const tuinator::Theme& theme) {
    parent.add_child(std::make_unique<tuinator::Label>(text, theme.heading));
}

} // namespace

int main() {
    tuinator::Application app;
    const tuinator::Theme theme = tuinator::dark_theme();

    AnimatedCharts animated{};

    auto content = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});

    content->add_child(std::make_unique<tuinator::Label>("Tuinator Charts Gallery", theme.heading));
    content->add_child(
        std::make_unique<tuinator::Label>("Scroll through many bar/line graph styles. Animated charts update live. "
                                          "Drag bars in interactive sections. q = quit",
                                          theme.muted));

    // --- Bar charts ---
    add_section_header(*content, "1. Vertical bar charts", theme);

    {
        auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});
        row->add_child(panel("Blocks", theme,
                             std::make_unique<tuinator::BarChart>(
                                 std::vector<tuinator::BarChartBar>{
                                     {"Mon", 42, color(tuinator::Color::Cyan)},
                                     {"Tue", 68, color(tuinator::Color::Green)},
                                     {"Wed", 35, color(tuinator::Color::Yellow)},
                                     {"Thu", 81, color(tuinator::Color::Magenta)},
                                     {"Fri", 57, color(tuinator::Color::Blue)},
                                 },
                                 tuinator::BarChartOptions{
                                     .style = tuinator::ChartGlyphStyle::Blocks,
                                     .min_width = 28,
                                     .min_height = 10,
                                     .title_style = theme.muted,
                                     .axis_style = theme.muted,
                                     .grid_style = theme.muted,
                                 })));
        row->add_child(panel("Braille", theme,
                             std::make_unique<tuinator::BarChart>(
                                 std::vector<tuinator::BarChartBar>{
                                     {"A", 55, color(tuinator::Color::Red)},
                                     {"B", 72, color(tuinator::Color::Green)},
                                     {"C", 38, color(tuinator::Color::Yellow)},
                                     {"D", 91, color(tuinator::Color::Cyan)},
                                 },
                                 tuinator::BarChartOptions{
                                     .style = tuinator::ChartGlyphStyle::Braille,
                                     .min_width = 24,
                                     .min_height = 10,
                                     .axis_style = theme.muted,
                                     .grid_style = theme.muted,
                                 })));
        row->add_child(panel("Stars", theme,
                             std::make_unique<tuinator::BarChart>(
                                 std::vector<tuinator::BarChartBar>{
                                     {"S0", 2, color(tuinator::Color::Green)},
                                     {"S1", 5, color(tuinator::Color::Green)},
                                     {"S2", 3, color(tuinator::Color::Green)},
                                     {"S3", 9, color(tuinator::Color::Green)},
                                     {"S4", 5, color(tuinator::Color::Green)},
                                     {"S5", 3, color(tuinator::Color::Green)},
                                 },
                                 tuinator::BarChartOptions{
                                     .style = tuinator::ChartGlyphStyle::Stars,
                                     .min_width = 28,
                                     .min_height = 10,
                                     .axis_style = theme.muted,
                                     .grid_style = theme.muted,
                                 })));
        content->add_child(std::move(row));
    }

    add_section_header(*content, "2. Horizontal bars + interactive temperature", theme);
    {
        auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});

        row->add_child(panel("Horizontal blocks", theme,
                             std::make_unique<tuinator::BarChart>(
                                 std::vector<tuinator::BarChartBar>{
                                     {"CPU", 72, color(tuinator::Color::Cyan)},
                                     {"GPU", 58, color(tuinator::Color::Magenta)},
                                     {"RAM", 64, color(tuinator::Color::Green)},
                                     {"Disk", 41, color(tuinator::Color::Yellow)},
                                 },
                                 tuinator::BarChartOptions{
                                     .style = tuinator::ChartGlyphStyle::Blocks,
                                     .orientation = tuinator::BarChartOrientation::Horizontal,
                                     .min_width = 34,
                                     .min_height = 10,
                                     .axis_style = theme.muted,
                                     .grid_style = theme.muted,
                                 })));

        auto temps = std::make_unique<tuinator::BarChart>(
            std::vector<tuinator::BarChartBar>{
                {"Core0", 60, color(tuinator::Color::Cyan)},
                {"Core1", 59, color(tuinator::Color::Cyan)},
                {"Core2", 72, color(tuinator::Color::Yellow)},
                {"Core3", 78, color(tuinator::Color::Red)},
                {"Pack", 65, color(tuinator::Color::Magenta)},
            },
            tuinator::BarChartOptions{
                .style = tuinator::ChartGlyphStyle::Dots,
                .orientation = tuinator::BarChartOrientation::Vertical,
                .max_value = 100.0,
                .interactive = true,
                .min_width = 30,
                .min_height = 12,
                .axis_style = theme.muted,
                .grid_style = theme.muted,
            });
        animated.temps = temps.get();
        row->add_child(panel("Interactive temps (drag)", theme, std::move(temps)));
        content->add_child(std::move(row));
    }

    // --- Line charts ---
    add_section_header(*content, "3. Line charts", theme);
    {
        auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});

        row->add_child(panel("Dot line", theme,
                             std::make_unique<tuinator::LineChart>(
                                 std::vector<tuinator::LineChartSeries>{
                                     {"sin", sine_wave(80, 0.0, 0.9, 0.0), color(tuinator::Color::Red)},
                                 },
                                 tuinator::LineChartOptions{
                                     .style = tuinator::ChartGlyphStyle::Dots,
                                     .mode = tuinator::LineChartMode::Line,
                                     .min_width = 36,
                                     .min_height = 10,
                                     .axis_style = theme.muted,
                                     .grid_style = theme.muted,
                                 })));

        row->add_child(panel("Braille line", theme,
                             std::make_unique<tuinator::LineChart>(
                                 std::vector<tuinator::LineChartSeries>{
                                     {"wave", sine_wave(120, 1.2, 0.85, 0.1), color(tuinator::Color::Yellow)},
                                 },
                                 tuinator::LineChartOptions{
                                     .style = tuinator::ChartGlyphStyle::Braille,
                                     .mode = tuinator::LineChartMode::Line,
                                     .min_width = 36,
                                     .min_height = 10,
                                     .axis_style = theme.muted,
                                     .grid_style = theme.muted,
                                 })));

        row->add_child(panel("Multi-series", theme,
                             std::make_unique<tuinator::LineChart>(
                                 std::vector<tuinator::LineChartSeries>{
                                     {"rx", sine_wave(60, 0.0, 0.7, 0.2), color(tuinator::Color::Cyan)},
                                     {"tx", sine_wave(60, 1.5, 0.5, -0.1), color(tuinator::Color::Magenta)},
                                 },
                                 tuinator::LineChartOptions{
                                     .style = tuinator::ChartGlyphStyle::FineDots,
                                     .mode = tuinator::LineChartMode::Line,
                                     .min_width = 36,
                                     .min_height = 10,
                                     .axis_style = theme.muted,
                                     .grid_style = theme.muted,
                                     .legend_style = theme.label,
                                 })));
        content->add_child(std::move(row));
    }

    add_section_header(*content, "4. Area, step, scatter", theme);
    {
        auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});

        row->add_child(panel("Area fill", theme,
                             std::make_unique<tuinator::LineChart>(
                                 std::vector<tuinator::LineChartSeries>{
                                     {"load", randomish_wave(50, 0.0), color(tuinator::Color::Green)},
                                 },
                                 tuinator::LineChartOptions{
                                     .style = tuinator::ChartGlyphStyle::Blocks,
                                     .mode = tuinator::LineChartMode::Area,
                                     .max_value = 60.0,
                                     .min_width = 30,
                                     .min_height = 10,
                                     .axis_style = theme.muted,
                                     .grid_style = theme.muted,
                                 })));

        row->add_child(panel("Step", theme,
                             std::make_unique<tuinator::LineChart>(
                                 std::vector<tuinator::LineChartSeries>{
                                     {"freq", std::vector<double>{1.2, 1.2, 2.4, 2.4, 3.1, 3.1, 2.0, 2.0, 3.6, 3.6},
                                      color(tuinator::Color::Blue)},
                                 },
                                 tuinator::LineChartOptions{
                                     .style = tuinator::ChartGlyphStyle::Plus,
                                     .mode = tuinator::LineChartMode::Step,
                                     .min_width = 30,
                                     .min_height = 10,
                                     .axis_style = theme.muted,
                                     .grid_style = theme.muted,
                                 })));

        row->add_child(panel("Scatter", theme,
                             std::make_unique<tuinator::LineChart>(
                                 std::vector<tuinator::LineChartSeries>{
                                     {"points", randomish_wave(24, 2.0), color(tuinator::Color::Magenta)},
                                 },
                                 tuinator::LineChartOptions{
                                     .style = tuinator::ChartGlyphStyle::Stars,
                                     .mode = tuinator::LineChartMode::Scatter,
                                     .min_width = 30,
                                     .min_height = 10,
                                     .axis_style = theme.muted,
                                     .grid_style = theme.muted,
                                 })));
        content->add_child(std::move(row));
    }

    add_section_header(*content, "5. Sparklines + mirrored network", theme);
    {
        auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});

        row->add_child(panel("Sparkline", theme,
                             std::make_unique<tuinator::LineChart>(
                                 std::vector<tuinator::LineChartSeries>{
                                     {"srv0", sine_wave(80, 0.4, 0.8, 0.0), color(tuinator::Color::Cyan)},
                                 },
                                 tuinator::LineChartOptions{
                                     .style = tuinator::ChartGlyphStyle::Dots,
                                     .mode = tuinator::LineChartMode::Sparkline,
                                     .show_axes = false,
                                     .show_grid = false,
                                     .min_width = 34,
                                     .min_height = 4,
                                 })));

        auto network = std::make_unique<tuinator::LineChart>(
            std::vector<tuinator::LineChartSeries>{
                {"upload", sine_wave(80, 0.0, 0.6, 0.1), color(tuinator::Color::Yellow)},
                {"download", sine_wave(80, 2.1, 0.4, -0.1), color(tuinator::Color::Green)},
            },
            tuinator::LineChartOptions{
                .style = tuinator::ChartGlyphStyle::FineDots,
                .mode = tuinator::LineChartMode::Line,
                .mirror = true,
                .min_width = 34,
                .min_height = 10,
                .axis_style = theme.muted,
                .grid_style = theme.muted,
                .legend_style = theme.label,
            });
        animated.network = network.get();
        row->add_child(panel("Mirrored network", theme, std::move(network)));
        content->add_child(std::move(row));
    }

    add_section_header(*content, "6. Live animated charts", theme);
    {
        auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});

        auto cpu = std::make_unique<tuinator::LineChart>(
            std::vector<tuinator::LineChartSeries>{
                {"cpu", std::vector<double>{}, color(tuinator::Color::Green)},
            },
            tuinator::LineChartOptions{
                .style = tuinator::ChartGlyphStyle::Braille,
                .mode = tuinator::LineChartMode::Line,
                .max_value = 100.0,
                .min_width = 40,
                .min_height = 10,
                .axis_style = theme.muted,
                .grid_style = theme.muted,
            });
        animated.cpu = cpu.get();
        row->add_child(panel("Live CPU history", theme, std::move(cpu)));

        auto spectrum = std::make_unique<tuinator::BarChart>(std::vector<tuinator::BarChartBar>{},
                                                             tuinator::BarChartOptions{
                                                                 .style = tuinator::ChartGlyphStyle::Braille,
                                                                 .max_value = 100.0,
                                                                 .show_labels = false,
                                                                 .min_width = 40,
                                                                 .min_height = 10,
                                                                 .axis_style = theme.muted,
                                                                 .grid_style = theme.muted,
                                                             });
        {
            std::vector<tuinator::BarChartBar> bars;
            bars.reserve(32);
            for (int j = 0; j < 32; ++j) {
                bars.push_back({
                    "",
                    20.0,
                    color(static_cast<tuinator::Color>(static_cast<int>(tuinator::Color::Red) + (j % 6))),
                });
            }
            spectrum->set_bars(std::move(bars));
        }
        animated.spectrum = spectrum.get();
        row->add_child(panel("Braille spectrum", theme, std::move(spectrum)));
        content->add_child(std::move(row));
    }

    add_section_header(*content, "7. Glyph style gallery (hash / plus / small dots)", theme);
    {
        auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});
        const std::vector<std::pair<tuinator::ChartGlyphStyle, tuinator::Color>> styles = {
            {tuinator::ChartGlyphStyle::Hash, tuinator::Color::Red},
            {tuinator::ChartGlyphStyle::Plus, tuinator::Color::Yellow},
            {tuinator::ChartGlyphStyle::SmallDots, tuinator::Color::Cyan},
        };

        for (const auto& [style, col] : styles) {
            std::string title = "Custom";
            for (const tuinator::ChartGlyphStyleInfo& info : tuinator::all_chart_glyph_styles()) {
                if (info.style == style) {
                    title = info.title;
                    break;
                }
            }

            row->add_child(panel(title, theme,
                                 std::make_unique<tuinator::LineChart>(
                                     std::vector<tuinator::LineChartSeries>{
                                         {"data", sine_wave(50, static_cast<double>(col), 0.8, 0.0), color(col)},
                                     },
                                     tuinator::LineChartOptions{
                                         .style = style,
                                         .mode = tuinator::LineChartMode::Line,
                                         .min_width = 26,
                                         .min_height = 8,
                                         .axis_style = theme.muted,
                                         .grid_style = theme.muted,
                                     })));
        }
        content->add_child(std::move(row));
    }

    add_section_header(*content, "8. Histogram", theme);
    {
        auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});
        row->add_child(panel("Latency distribution", theme,
                             std::make_unique<tuinator::Histogram>(
                                 std::vector<tuinator::HistogramBin>{
                                     {"0-10", 12, color(tuinator::Color::Green)},
                                     {"10-20", 28, color(tuinator::Color::Cyan)},
                                     {"20-30", 45, color(tuinator::Color::Blue)},
                                     {"30-40", 31, color(tuinator::Color::Yellow)},
                                     {"40-50", 18, color(tuinator::Color::Magenta)},
                                     {"50+", 7, color(tuinator::Color::Red)},
                                 },
                                 tuinator::HistogramOptions{
                                     .style = tuinator::ChartGlyphStyle::Blocks,
                                     .show_counts = true,
                                     .min_width = 34,
                                     .min_height = 10,
                                     .axis_style = theme.muted,
                                     .grid_style = theme.muted,
                                     .count_style = theme.label,
                                 })));
        row->add_child(panel("Braille histogram", theme,
                             std::make_unique<tuinator::Histogram>(
                                 std::vector<tuinator::HistogramBin>{
                                     {"A", 8, color(tuinator::Color::Cyan)},
                                     {"B", 22, color(tuinator::Color::Green)},
                                     {"C", 35, color(tuinator::Color::Yellow)},
                                     {"D", 19, color(tuinator::Color::Magenta)},
                                     {"E", 11, color(tuinator::Color::Red)},
                                 },
                                 tuinator::HistogramOptions{
                                     .style = tuinator::ChartGlyphStyle::Braille,
                                     .min_width = 28,
                                     .min_height = 10,
                                     .axis_style = theme.muted,
                                     .grid_style = theme.muted,
                                 })));
        content->add_child(std::move(row));
    }

    add_section_header(*content, "9. Heatmap", theme);
    {
        std::vector<std::vector<double>> activity;
        for (int row = 0; row < 7; ++row) {
            std::vector<double> line;
            for (int col = 0; col < 12; ++col) {
                line.push_back(static_cast<double>((row + 1) * (col + 2) % 17));
            }
            activity.push_back(std::move(line));
        }

        content->add_child(panel(
            "Weekly activity", theme,
            std::make_unique<tuinator::Heatmap>(
                activity, std::vector<std::string>{"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"},
                std::vector<std::string>{"W1", "W2", "W3", "W4", "W5", "W6", "W7", "W8", "W9", "W10", "W11", "W12"},
                tuinator::HeatmapOptions{
                    .style = tuinator::ChartGlyphStyle::FineDots,
                    .cell_gap = 1,
                    .min_width = 36,
                    .min_height = 10,
                    .title_style = theme.heading,
                    .label_style = theme.muted,
                    .low_style = style_fg(tuinator::Rgb::hex(0x1D4ED8)),
                    .high_style = style_fg(tuinator::Rgb::hex(0xEF4444)),
                })));
    }

    add_section_header(*content, "10. Gauge / Radial", theme);
    {
        auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});
        row->add_child(
            panel("CPU", theme,
                  std::make_unique<tuinator::GaugeChart>(72.0, tuinator::GaugeChartOptions{
                                                                   .style = tuinator::GaugeStyle::Arc,
                                                                   .glyph = tuinator::ChartGlyphStyle::Blocks,
                                                                   .unit = "%",
                                                                   .diameter = 16,
                                                                   .track_style = theme.muted,
                                                                   .fill_style = color(tuinator::Color::Green),
                                                                   .value_style = theme.heading,
                                                               })));
        row->add_child(
            panel("RAM", theme,
                  std::make_unique<tuinator::GaugeChart>(64.0, tuinator::GaugeChartOptions{
                                                                   .style = tuinator::GaugeStyle::Arc,
                                                                   .glyph = tuinator::ChartGlyphStyle::Dots,
                                                                   .unit = "%",
                                                                   .diameter = 16,
                                                                   .track_style = theme.muted,
                                                                   .fill_style = color(tuinator::Color::Cyan),
                                                                   .value_style = theme.heading,
                                                               })));
        row->add_child(
            panel("Disk", theme,
                  std::make_unique<tuinator::GaugeChart>(81.0, tuinator::GaugeChartOptions{
                                                                   .style = tuinator::GaugeStyle::Horizontal,
                                                                   .glyph = tuinator::ChartGlyphStyle::Blocks,
                                                                   .unit = "%",
                                                                   .diameter = 24,
                                                                   .track_style = theme.muted,
                                                                   .fill_style = color(tuinator::Color::Yellow),
                                                                   .value_style = theme.label,
                                                               })));
        content->add_child(std::move(row));
    }

    add_section_header(*content, "11. Stacked area", theme);
    {
        content->add_child(panel("Memory breakdown", theme,
                                 std::make_unique<tuinator::StackedAreaChart>(
                                     std::vector<tuinator::StackedAreaSeries>{
                                         {"Used", {20, 24, 28, 32, 30, 35, 40, 38}, color(tuinator::Color::Red)},
                                         {"Cached", {10, 12, 11, 14, 15, 13, 16, 18}, color(tuinator::Color::Yellow)},
                                         {"Free", {30, 28, 26, 24, 25, 22, 20, 19}, color(tuinator::Color::Green)},
                                     },
                                     tuinator::StackedAreaChartOptions{
                                         .style = tuinator::ChartGlyphStyle::Blocks,
                                         .min_width = 48,
                                         .min_height = 10,
                                         .axis_style = theme.muted,
                                         .grid_style = theme.muted,
                                         .legend_style = theme.label,
                                     })));
    }

    add_section_header(*content, "12. Waterfall", theme);
    {
        content->add_child(
            panel("Revenue bridge", theme,
                  std::make_unique<tuinator::WaterfallChart>(
                      std::vector<tuinator::WaterfallStep>{
                          {"Start", 100.0, color(tuinator::Color::Cyan), color(tuinator::Color::Red)},
                          {"Product A", 35.0, color(tuinator::Color::Green), color(tuinator::Color::Red)},
                          {"Product B", -12.0, color(tuinator::Color::Green), color(tuinator::Color::Red)},
                          {"Costs", -28.0, color(tuinator::Color::Green), color(tuinator::Color::Red)},
                          {"Tax", -8.0, color(tuinator::Color::Green), color(tuinator::Color::Red)},
                      },
                      tuinator::WaterfallChartOptions{
                          .baseline = 0.0,
                          .bar_width = 2,
                          .bar_gap = 1,
                          .compact_layout = true,
                          .min_width = 28,
                          .min_height = 12,
                          .axis_style = theme.muted,
                          .grid_style = theme.muted,
                          .connector_style = theme.muted,
                          .total_style = color(tuinator::Color::Cyan),
                      })));
    }

    add_section_header(*content, "13. Candlestick / OHLC", theme);
    {
        content->add_child(panel("Price + volume", theme,
                                 std::make_unique<tuinator::CandlestickChart>(
                                     std::vector<tuinator::OhlcBar>{
                                         {"03/01", 271, 296, 268, 288, 120, {}, {}},
                                         {"03/08", 288, 305, 280, 298, 180, {}, {}},
                                         {"03/15", 298, 320, 292, 315, 220, {}, {}},
                                         {"03/22", 315, 330, 308, 322, 160, {}, {}},
                                         {"03/29", 322, 346, 318, 340, 240, {}, {}},
                                         {"04/05", 340, 355, 332, 348, 210, {}, {}},
                                         {"04/12", 348, 371, 342, 365, 190, {}, {}},
                                     },
                                     tuinator::CandlestickChartOptions{
                                         .show_volume = true,
                                         .bar_width = 2,
                                         .bar_gap = 1,
                                         .compact_layout = true,
                                         .min_width = 24,
                                         .min_height = 14,
                                         .axis_style = theme.muted,
                                         .grid_style = theme.muted,
                                         .up_style = color(tuinator::Color::Green),
                                         .down_style = color(tuinator::Color::Red),
                                     })));
    }

    auto scroll = std::make_unique<tuinator::ScrollView>(std::move(content),
                                                         tuinator::ScrollViewOptions{.width = 100, .height = 24});
    scroll->set_flex(1);

    auto status = std::make_unique<tuinator::StatusBar>(
        "Charts gallery • histogram, heatmap, gauge, stacked, waterfall, OHLC • scroll to explore", theme.muted);

    auto root = std::make_unique<ChartsRoot>(tuinator::BoxOptions{.gap = 0, .padding = 0});
    root->add_child(std::move(scroll));
    root->add_child(std::move(status));

    app.set_root(std::move(root));

    app.set_interval(100, [&animated]() {
        animated.tick += 0.15;
        const double t = animated.tick;

        if (animated.cpu != nullptr) {
            const double cpu = 35.0 + std::sin(t * 0.7) * 25.0 + std::sin(t * 2.1) * 10.0;
            animated.cpu->push_value(0, cpu, 80);
        }

        if (animated.network != nullptr) {
            auto series = animated.network->series();
            if (series.size() >= 2) {
                series[0].values = sine_wave(80, t, 0.6, 0.15);
                series[1].values = sine_wave(80, t + 2.0, 0.45, -0.12);
                animated.network->set_series(std::move(series));
            }
        }

        if (animated.spectrum != nullptr) {
            std::vector<tuinator::BarChartBar> bars;
            bars.reserve(32);
            for (int i = 0; i < 32; ++i) {
                const double value =
                    std::max(2.0, 15.0 + std::sin(t * 1.4 + i * 0.35) * 35.0 + std::cos(t * 0.8 + i * 0.2) * 20.0);
                bars.push_back({
                    "",
                    value,
                    color(static_cast<tuinator::Color>(static_cast<int>(tuinator::Color::Red) + (i % 6))),
                });
            }
            animated.spectrum->set_bars(std::move(bars));
        }
    });

    return app.run();
}
