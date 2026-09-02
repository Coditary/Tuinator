#include <tuinator/widgets/charts/line_chart.hpp>

#include <tuinator/render/text.hpp>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

namespace tuinator {

namespace {

constexpr int kAxisWidth = 6;

struct DataExtent {
    double min_v = 0.0;
    double max_v = 0.0;
};

DataExtent compute_data_extent(const std::vector<LineChartSeries>& series, double floor) {
    DataExtent extent;
    extent.min_v = 0.0;
    extent.max_v = floor;
    for (const LineChartSeries& item : series) {
        for (double value : item.values) {
            extent.min_v = std::min(extent.min_v, value);
            extent.max_v = std::max(extent.max_v, value);
        }
    }
    return extent;
}

} // namespace

LineChart::LineChart(std::vector<LineChartSeries> series, LineChartOptions options)
    : series_(std::move(series)),
      options_(std::move(options)) {}

void LineChart::set_series(std::vector<LineChartSeries> series) {
    series_ = std::move(series);
    mark_dirty();
}

void LineChart::set_options(LineChartOptions options) {
    options_ = std::move(options);
    mark_dirty();
}

void LineChart::set_style(ChartGlyphStyle style) {
    options_.style = style;
    mark_dirty();
}

void LineChart::set_mode(LineChartMode mode) {
    options_.mode = mode;
    mark_dirty();
}

void LineChart::push_value(std::size_t series_index, double value, std::size_t max_points) {
    if (series_index >= series_.size()) {
        return;
    }

    series_[series_index].values.push_back(value);
    if (max_points > 0 && series_[series_index].values.size() > max_points) {
        const std::size_t overflow = series_[series_index].values.size() - max_points;
        series_[series_index].values.erase(
            series_[series_index].values.begin(),
            series_[series_index].values.begin() + static_cast<std::ptrdiff_t>(overflow));
    }
    mark_dirty();
}

double LineChart::value_max() const {
    if (options_.max_value > options_.min_value) {
        return options_.max_value;
    }

    const DataExtent extent = compute_data_extent(series_, options_.min_value);
    double max_v = extent.max_v;
    if (options_.mirror) {
        max_v = std::max(max_v, std::abs(extent.min_v));
    }

    return chart_auto_max(std::max(1.0, max_v));
}

double LineChart::value_min() const {
    if (options_.max_value > options_.min_value && options_.min_value != 0.0) {
        return options_.min_value;
    }

    if (options_.mirror) {
        return -value_max();
    }

    const DataExtent extent = compute_data_extent(series_, options_.min_value);
    return chart_auto_min(extent.min_v, value_max());
}

double LineChart::value_span() const {
    return std::max(1e-6, value_max() - value_min());
}

LineChart::PlotArea LineChart::compute_plot() const {
    PlotArea plot{};
    plot.title_rows = options_.title.empty() ? 0 : 1;
    plot.legend_rows = (options_.show_legend && options_.mode != LineChartMode::Sparkline
        && series_.size() > 1)
        ? 1
        : 0;

    const bool axes = options_.show_axes && options_.mode != LineChartMode::Sparkline;
    plot.left = axes ? kAxisWidth : 0;
    plot.top = plot.title_rows;
    plot.width = std::max(1, bounds_.width - plot.left);
    plot.height = std::max(1, bounds_.height - plot.top - plot.legend_rows);
    return plot;
}

Size LineChart::preferred_size() const {
    return {options_.min_width, options_.min_height + (options_.title.empty() ? 0 : 1)};
}

double LineChart::sample_series(const LineChartSeries& series, double x) const {
    if (series.values.empty()) {
        return 0.0;
    }
    if (series.values.size() == 1) {
        return series.values.front();
    }

    const double index = x * static_cast<double>(series.values.size() - 1);
    const int i0 = std::clamp(static_cast<int>(std::floor(index)), 0, static_cast<int>(series.values.size()) - 1);
    const int i1 = std::min(i0 + 1, static_cast<int>(series.values.size()) - 1);
    const double t = index - static_cast<double>(i0);
    return series.values[static_cast<std::size_t>(i0)] * (1.0 - t)
        + series.values[static_cast<std::size_t>(i1)] * t;
}

void LineChart::plot_point(Canvas& canvas, int x, int y, const Style& style) const {
    if (options_.style == ChartGlyphStyle::Braille) {
        plot_braille_point(canvas, x, y, style);
        return;
    }

    canvas.draw_text({x, y}, chart_glyph_for(options_.style, options_.custom_glyph), style);
}

void LineChart::plot_braille_point(Canvas& canvas, int x, int y, const Style& style) const {
    bool dots[8] = {};
    dots[0] = true;
    canvas.draw_text({x, y}, chart_braille_from_dots(dots), style);
}

void LineChart::draw_line_segment(
    Canvas& canvas,
    int x0,
    int y0,
    int x1,
    int y1,
    const Style& style) const {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    int x = x0;
    int y = y0;

    while (true) {
        plot_point(canvas, x, y, style);
        if (x == x1 && y == y1) {
            break;
        }
        const int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }
}

void LineChart::paint_grid(Canvas& canvas, const PlotArea& plot, double min_v, double max_v) const {
    if (!options_.show_grid || plot.height <= 1) {
        return;
    }

    const int lines = options_.mode == LineChartMode::Sparkline ? 0 : 4;
    for (int i = 0; i <= lines; ++i) {
        const int y = plot.top + plot.height - 1 - (i * (plot.height - 1) / std::max(1, lines));
        canvas.draw_hline(plot.left, y, plot.width, options_.grid_style);

        if (options_.show_axes && options_.mode != LineChartMode::Sparkline) {
            const double value = min_v + (max_v - min_v) * static_cast<double>(i) / std::max(1, lines);
            std::ostringstream label;
            label << std::fixed << std::setprecision(1) << value;
            canvas.draw_text({0, y}, label.str(), options_.axis_style);
        }
    }

    if (options_.mirror) {
        const int mid_y = plot.top + plot.height / 2;
        canvas.draw_hline(plot.left, mid_y, plot.width, options_.axis_style);
    }
}

void LineChart::paint_series(Canvas& canvas, const PlotArea& plot, const LineChartSeries& series) const {
    if (series.values.empty() || plot.width <= 0 || plot.height <= 0) {
        return;
    }

    const double min_v = value_min();
    const double span = value_span();

    auto value_to_y = [&](double value) {
        const double ratio = (value - min_v) / span;
        return plot.top + plot.height - 1 - static_cast<int>(ratio * (plot.height - 1) + 0.5);
    };

    if (options_.mode == LineChartMode::Scatter) {
        const int count = static_cast<int>(series.values.size());
        for (int i = 0; i < count; ++i) {
            const int x = plot.left + (count == 1 ? plot.width / 2 : i * (plot.width - 1) / (count - 1));
            const int y = value_to_y(series.values[static_cast<std::size_t>(i)]);
            plot_point(canvas, x, y, series.style);
        }
        return;
    }

    if (options_.mode == LineChartMode::Area) {
        for (int x = 0; x < plot.width; ++x) {
            const double sample_x = static_cast<double>(x) / std::max(1, plot.width - 1);
            const int y = value_to_y(sample_series(series, sample_x));
            const int base_y = value_to_y(min_v);
            for (int row = std::min(y, base_y); row <= std::max(y, base_y); ++row) {
                plot_point(canvas, plot.left + x, row, series.style);
            }
        }
        return;
    }

    int prev_x = -1;
    int prev_y = -1;

    for (int x = 0; x < plot.width; ++x) {
        const double sample_x = static_cast<double>(x) / std::max(1, plot.width - 1);
        double value = sample_series(series, sample_x);

        if (options_.mode == LineChartMode::Step && prev_x >= 0) {
            const int step_y = value_to_y(value);
            draw_line_segment(canvas, prev_x, prev_y, plot.left + x, prev_y, series.style);
            draw_line_segment(canvas, plot.left + x, prev_y, plot.left + x, step_y, series.style);
            prev_x = plot.left + x;
            prev_y = step_y;
            continue;
        }

        const int y = value_to_y(value);
        const int px = plot.left + x;

        if (options_.mode == LineChartMode::Line || options_.mode == LineChartMode::Sparkline) {
            if (prev_x >= 0) {
                draw_line_segment(canvas, prev_x, prev_y, px, y, series.style);
            } else {
                plot_point(canvas, px, y, series.style);
            }
            prev_x = px;
            prev_y = y;
        }
    }
}

void LineChart::paint(Canvas& canvas) const {
    if (bounds_.width <= 0 || bounds_.height <= 0 || series_.empty()) {
        return;
    }

    const PlotArea plot = compute_plot();
    const double min_v = value_min();
    const double max_v = value_max();

    if (!options_.title.empty()) {
        canvas.draw_text({0, 0}, options_.title, options_.title_style);
    }

    if (options_.mode != LineChartMode::Sparkline) {
        paint_grid(canvas, plot, min_v, max_v);
    }

    for (const LineChartSeries& item : series_) {
        paint_series(canvas, plot, item);
    }

    if (options_.show_legend && options_.mode != LineChartMode::Sparkline && series_.size() > 1) {
        int x = 0;
        const int y = plot.top + plot.height;
        for (const LineChartSeries& item : series_) {
            if (item.label.empty()) {
                continue;
            }
            canvas.draw_text({x, y}, item.label + " ", item.style.foreground == Color::Default
                ? options_.legend_style
                : item.style);
            x += text_display_width(item.label) + 1;
        }
    }
}

} // namespace tuinator
