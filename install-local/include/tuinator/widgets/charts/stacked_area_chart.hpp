#pragma once

#include <tuinator/widgets/charts/chart_common.hpp>
#include <tuinator/widgets/widget.hpp>

#include <string>
#include <vector>

namespace tuinator {

struct StackedAreaSeries {
    std::string label;
    std::vector<double> values;
    Style style{};
};

struct StackedAreaChartOptions {
    ChartGlyphStyle style = ChartGlyphStyle::Blocks;
    std::string title;
    double min_value = 0.0;
    double max_value = 0.0;
    bool show_axes = true;
    bool show_grid = true;
    bool show_legend = true;
    int min_width = 24;
    int min_height = 8;
    Style title_style{};
    Style axis_style{};
    Style grid_style{};
    Style legend_style{};
};

class StackedAreaChart : public Widget {
public:
    StackedAreaChart(std::vector<StackedAreaSeries> series = {}, StackedAreaChartOptions options = {});

    const std::vector<StackedAreaSeries>& series() const { return series_; }
    const StackedAreaChartOptions& options() const { return options_; }

    void set_series(std::vector<StackedAreaSeries> series);
    void set_options(StackedAreaChartOptions options);

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
    double stacked_max() const;

    std::vector<StackedAreaSeries> series_;
    StackedAreaChartOptions options_;
};

} // namespace tuinator
