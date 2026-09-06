#include <tuinator/render/text.hpp>
#include <tuinator/widgets/charts/waterfall_chart.hpp>

#include <algorithm>
#include <cmath>

namespace tuinator {

WaterfallChart::WaterfallChart(std::vector<WaterfallStep> steps, WaterfallChartOptions options)
    : steps_(std::move(steps)), options_(std::move(options)) {}

void WaterfallChart::set_steps(std::vector<WaterfallStep> steps) {
    steps_ = std::move(steps);
    mark_dirty();
}

void WaterfallChart::set_options(WaterfallChartOptions options) {
    options_ = std::move(options);
    mark_dirty();
}

std::vector<WaterfallChart::Segment> WaterfallChart::compute_segments() const {
    std::vector<Segment> segments;
    segments.reserve(steps_.size());

    double cursor = options_.baseline;
    for (const WaterfallStep& step : steps_) {
        Segment segment{};
        segment.start = cursor;
        segment.end = cursor + step.delta;
        segments.push_back(segment);
        cursor = segment.end;
    }

    if (!segments.empty()) {
        Segment total{};
        total.start = options_.baseline;
        total.end = cursor;
        total.is_total = true;
        segments.push_back(total);
    }

    return segments;
}

double WaterfallChart::range_min(const std::vector<Segment>& segments) const {
    double min_v = options_.baseline;
    for (const Segment& segment : segments) {
        min_v = std::min(min_v, std::min(segment.start, segment.end));
    }
    return chart_auto_min(min_v, range_max(segments));
}

double WaterfallChart::range_max(const std::vector<Segment>& segments) const {
    double max_v = options_.baseline;
    for (const Segment& segment : segments) {
        max_v = std::max(max_v, std::max(segment.start, segment.end));
    }
    return chart_auto_max(std::max(1.0, max_v));
}

Size WaterfallChart::preferred_size() const {
    const int segments = static_cast<int>(steps_.size()) + (steps_.empty() ? 0 : 1);
    const int compact_width =
        segments * std::max(1, options_.bar_width) + std::max(0, segments - 1) * options_.bar_gap + 8;
    return {
        std::max(options_.min_width, compact_width),
        std::max(options_.min_height, 10 + (options_.title.empty() ? 0 : 1)),
    };
}

void WaterfallChart::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0 || steps_.empty()) {
        return;
    }

    const int footer_rows = options_.show_labels ? 1 : 0;
    const ChartPlotArea plot = chart_compute_plot(bounds_, options_.title, footer_rows, options_.show_axis);

    if (!options_.title.empty()) {
        canvas.draw_text({0, 0}, options_.title, options_.title_style);
    }

    const std::vector<Segment> segments = compute_segments();
    const double min_v = range_min(segments);
    const double max_v = range_max(segments);
    const double span = std::max(1e-6, max_v - min_v);

    chart_paint_horizontal_grid(canvas, plot, min_v, max_v, options_.axis_style, options_.grid_style,
                                options_.show_axis);

    const int count = static_cast<int>(segments.size());
    const int body_w = std::max(1, options_.bar_width);
    const int gap = std::max(0, options_.bar_gap);
    const int slot = body_w + gap;
    const int total_w = count * slot - gap;
    const int start_x = options_.compact_layout ? plot.left + std::max(0, (plot.width - total_w) / 2) : plot.left;
    const int stretch_slot = options_.compact_layout ? slot : std::max(slot, plot.width / std::max(1, count));

    for (int i = 0; i < count; ++i) {
        const Segment& segment = segments[static_cast<std::size_t>(i)];
        const int x = start_x + i * stretch_slot;

        const int y0 =
            plot.top + plot.height - 1 - static_cast<int>((segment.start - min_v) / span * (plot.height - 1) + 0.5);
        const int y1 =
            plot.top + plot.height - 1 - static_cast<int>((segment.end - min_v) / span * (plot.height - 1) + 0.5);

        Style style = options_.total_style;
        if (!segment.is_total && i < static_cast<int>(steps_.size())) {
            const WaterfallStep& step = steps_[static_cast<std::size_t>(i)];
            style = step.delta >= 0.0 ? step.up_style : step.down_style;
            if (style.foreground == Color::Default) {
                style.foreground = step.delta >= 0.0 ? Color::Green : Color::Red;
            }
        } else if (style.foreground == Color::Default) {
            style.foreground = Color::Cyan;
        }

        for (int y = std::min(y0, y1); y <= std::max(y0, y1); ++y) {
            for (int col = 0; col < body_w; ++col) {
                chart_paint_glyph_cell(canvas, x + col, y, options_.style, {}, style);
            }
        }

        if (options_.show_connectors && i + 1 < count && !segment.is_total) {
            const int connector_y =
                plot.top + plot.height - 1 - static_cast<int>((segment.end - min_v) / span * (plot.height - 1) + 0.5);
            canvas.draw_hline(x + body_w, connector_y, gap + body_w, options_.connector_style);
        }

        if (options_.show_labels) {
            std::string label;
            if (segment.is_total) {
                label = "Total";
            } else if (i < static_cast<int>(steps_.size())) {
                label = steps_[static_cast<std::size_t>(i)].label;
            }
            const int label_slot = options_.compact_layout ? slot : stretch_slot;
            canvas.draw_text({x, plot.top + plot.height}, label.substr(0, static_cast<std::size_t>(label_slot)),
                             options_.axis_style);
        }
    }
}

} // namespace tuinator
