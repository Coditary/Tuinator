#include <tuinator/render/text.hpp>
#include <tuinator/widgets/charts/chart_widget.hpp>
#include <tuinator/widgets/charts/histogram.hpp>

#include <algorithm>
#include <cmath>

namespace tuinator {

Histogram::Histogram(std::vector<HistogramBin> bins, HistogramOptions options)
    : bins_(std::move(bins)), options_(std::move(options)) {}

void Histogram::set_bins(std::vector<HistogramBin> bins) {
    bins_ = std::move(bins);
    mark_dirty();
}

void Histogram::set_options(HistogramOptions options) {
    options_ = std::move(options);
    mark_dirty();
}

void Histogram::apply_stylesheet(const StyleResolver& styles) {
    apply_chart_stylesheet(*this, styles,
                           {&options_.min_width, &options_.min_height, &options_.show_axis, &options_.show_grid,
                            &options_.style, &options_.custom_glyph});
    mark_layout_dirty();
}

Size Histogram::preferred_size() const {
    return {
        std::max(options_.min_width, static_cast<int>(bins_.size()) * 2 + 6),
        std::max(options_.min_height, 10 + (options_.title.empty() ? 0 : 1)),
    };
}

void Histogram::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    paint_.prepare(ctx, *this, options_.title_style, options_.axis_style, options_.grid_style, options_.count_style);
    chart_paint_background(ctx, *this, bounds_.size());
    if (bins_.empty()) {
        return;
    }

    const int footer_rows = options_.show_labels ? 1 : 0;
    const ChartPlotArea plot = chart_compute_plot(bounds_, options_.title, footer_rows, options_.show_axis);

    if (!options_.title.empty()) {
        canvas.draw_text({0, 0}, options_.title, paint_.styles.title);
    }

    double max_v = options_.max_value;
    if (max_v <= options_.min_value) {
        for (const HistogramBin& bin : bins_) {
            max_v = std::max(max_v, bin.count);
        }
        max_v = chart_auto_max(std::max(1.0, max_v));
    }

    const double min_v = options_.min_value;
    const double span = std::max(1e-6, max_v - min_v);

    chart_paint_horizontal_grid(canvas, plot, min_v, max_v, paint_.styles.axis, paint_.styles.grid,
                                options_.show_axis);

    const int count = static_cast<int>(bins_.size());
    const int bar_width = std::max(1, plot.width / std::max(1, count));

    for (int i = 0; i < count; ++i) {
        const HistogramBin& bin = bins_[static_cast<std::size_t>(i)];
        const int x = plot.left + i * bar_width;
        const int filled = std::clamp(static_cast<int>((bin.count - min_v) / span * plot.height), 0, plot.height);
        const int y = plot.top + plot.height - filled;

        for (int row = y; row < plot.top + plot.height; ++row) {
            for (int col = 0; col < bar_width; ++col) {
                chart_paint_glyph_cell(canvas, x + col, row, options_.style, options_.custom_glyph, bin.style);
            }
        }

        if (options_.show_counts && filled > 0) {
            canvas.draw_text({x, std::max(plot.top, y - 1)}, chart_format_value(bin.count), paint_.styles.value);
        }

        if (options_.show_labels) {
            const std::size_t bytes = text_byte_length_for_width(bin.label, bar_width);
            canvas.draw_text({x, plot.top + plot.height}, bin.label.substr(0, bytes), paint_.styles.axis);
        }
    }
}

} // namespace tuinator
