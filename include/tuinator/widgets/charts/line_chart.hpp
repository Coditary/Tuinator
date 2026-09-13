#pragma once

#include <tuinator/render/style_resolver.hpp>
#include <tuinator/widgets/charts/chart_common.hpp>
#include <tuinator/widgets/charts/chart_widget.hpp>
#include <tuinator/widgets/charts/chart_widget_paint.hpp>
#include <tuinator/widgets/widget.hpp>

#include <string>
#include <vector>

namespace tuinator {

enum class LineChartMode {
    Line,
    Area,
    Sparkline,
    Scatter,
    Step,
};

struct LineChartSeries {
    std::string label;
    std::vector<double> values;
    Style style{};
};

struct LineChartOptions {
    ChartGlyphStyle style = ChartGlyphStyle::Dots;
    std::string custom_glyph;
    LineChartMode mode = LineChartMode::Line;
    std::string title;
    double min_value = 0.0;
    double max_value = 0.0;
    bool show_axes = true;
    bool show_grid = true;
    bool show_legend = true;
    bool mirror = false;
    int min_width = 24;
    int min_height = 8;
    Style title_style{};
    Style axis_style{};
    Style grid_style{};
    Style legend_style{};
};

class LineChart : public Widget, public ChartWidget {
  public:
    LineChart(std::vector<LineChartSeries> series = {}, LineChartOptions options = {});

    const std::vector<LineChartSeries>& series() const { return series_; }
    const LineChartOptions& options() const { return options_; }

    void set_series(std::vector<LineChartSeries> series);
    void set_options(LineChartOptions options);
    void set_style(ChartGlyphStyle style);
    void set_mode(LineChartMode mode);
    void push_value(std::size_t series_index, double value, std::size_t max_points = 0);

    std::string_view widget_type_name() const override { return "LineChart"; }
    void apply_stylesheet(const StyleResolver& styles) override;

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;

  private:
    struct PlotArea {
        int left = 0;
        int top = 0;
        int width = 0;
        int height = 0;
        int title_rows = 0;
        int legend_rows = 0;
    };

    PlotArea compute_plot() const;
    double value_max() const;
    double value_min() const;
    double value_span() const;
    double sample_series(const LineChartSeries& series, double x) const;

    void paint_grid(Canvas& canvas, const PlotArea& plot, double min_v, double max_v) const;
    void paint_series(Canvas& canvas, const PlotArea& plot, const LineChartSeries& series) const;
    void plot_point(Canvas& canvas, int x, int y, const Style& style) const;
    void plot_braille_point(Canvas& canvas, int x, int y, const Style& style) const;
    void draw_line_segment(Canvas& canvas, int x0, int y0, int x1, int y1, const Style& style) const;

    std::vector<LineChartSeries> series_;
    LineChartOptions options_;
    mutable ChartPaintSupport paint_{};
};

} // namespace tuinator
