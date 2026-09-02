#include <tuinator/widgets/charts/stacked_area_chart.hpp>

#include <tuinator/render/text.hpp>

#include <algorithm>
#include <cmath>

namespace tuinator {

StackedAreaChart::StackedAreaChart(std::vector<StackedAreaSeries> series, StackedAreaChartOptions options)
    : series_(std::move(series)),
      options_(std::move(options)) {}

void StackedAreaChart::set_series(std::vector<StackedAreaSeries> series) {
    series_ = std::move(series);
    mark_dirty();
}

void StackedAreaChart::set_options(StackedAreaChartOptions options) {
    options_ = std::move(options);
    mark_dirty();
}

StackedAreaChart::PlotArea StackedAreaChart::compute_plot() const {
    PlotArea plot{};
    plot.title_rows = options_.title.empty() ? 0 : 1;
    plot.legend_rows = (options_.show_legend && series_.size() > 1) ? 1 : 0;
    plot.left = options_.show_axes ? 6 : 0;
    plot.top = plot.title_rows;
    plot.width = std::max(1, bounds_.width - plot.left);
    plot.height = std::max(1, bounds_.height - plot.top - plot.legend_rows);
    return plot;
}

double StackedAreaChart::stacked_max() const {
    if (options_.max_value > options_.min_value) {
        return options_.max_value;
    }

    std::size_t points = 0;
    for (const StackedAreaSeries& item : series_) {
        points = std::max(points, item.values.size());
    }

    double max_total = 0.0;
    for (std::size_t i = 0; i < points; ++i) {
        double total = 0.0;
        for (const StackedAreaSeries& item : series_) {
            if (i < item.values.size()) {
                total += std::max(0.0, item.values[i]);
            }
        }
        max_total = std::max(max_total, total);
    }

    return chart_auto_max(std::max(1.0, max_total));
}

Size StackedAreaChart::preferred_size() const {
    return {options_.min_width, options_.min_height + (options_.title.empty() ? 0 : 1)};
}

void StackedAreaChart::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0 || series_.empty()) {
        return;
    }

    const PlotArea plot = compute_plot();
    if (!options_.title.empty()) {
        canvas.draw_text({0, 0}, options_.title, options_.title_style);
    }

    const double min_v = options_.min_value;
    const double max_v = stacked_max();
    const double span = std::max(1e-6, max_v - min_v);

    ChartPlotArea common{
        plot.left,
        plot.top,
        plot.width,
        plot.height,
        plot.title_rows,
        plot.legend_rows,
    };
    chart_paint_horizontal_grid(
        canvas,
        common,
        min_v,
        max_v,
        options_.axis_style,
        options_.grid_style,
        options_.show_axes);

    std::size_t points = 0;
    for (const StackedAreaSeries& item : series_) {
        points = std::max(points, item.values.size());
    }
    if (points == 0) {
        return;
    }

    for (int x = 0; x < plot.width; ++x) {
        const std::size_t index = points == 1
            ? 0
            : static_cast<std::size_t>(x) * (points - 1) / static_cast<std::size_t>(std::max(1, plot.width - 1));

        double cumulative = min_v;
        for (const StackedAreaSeries& item : series_) {
            const double part = index < item.values.size() ? std::max(0.0, item.values[index]) : 0.0;
            const double next = cumulative + part;

            const int y0 = plot.top + plot.height - 1
                - static_cast<int>((cumulative - min_v) / span * (plot.height - 1) + 0.5);
            const int y1 = plot.top + plot.height - 1
                - static_cast<int>((next - min_v) / span * (plot.height - 1) + 0.5);

            for (int y = std::min(y0, y1); y <= std::max(y0, y1); ++y) {
                chart_paint_glyph_cell(
                    canvas,
                    plot.left + x,
                    y,
                    options_.style,
                    {},
                    item.style);
            }

            cumulative = next;
        }
    }

    if (options_.show_legend && series_.size() > 1) {
        int x = 0;
        const int y = plot.top + plot.height;
        for (const StackedAreaSeries& item : series_) {
            if (item.label.empty()) {
                continue;
            }
            canvas.draw_text({x, y}, item.label + " ", item.style);
            x += text_display_width(item.label) + 1;
        }
    }
}

} // namespace tuinator
