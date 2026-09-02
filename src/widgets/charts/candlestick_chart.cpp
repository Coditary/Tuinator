#include <tuinator/widgets/charts/candlestick_chart.hpp>

#include <tuinator/render/text.hpp>

#include <algorithm>
#include <cmath>

namespace tuinator {

CandlestickChart::CandlestickChart(std::vector<OhlcBar> bars, CandlestickChartOptions options)
    : bars_(std::move(bars)),
      options_(std::move(options)) {}

void CandlestickChart::set_bars(std::vector<OhlcBar> bars) {
    bars_ = std::move(bars);
    mark_dirty();
}

void CandlestickChart::set_options(CandlestickChartOptions options) {
    options_ = std::move(options);
    mark_dirty();
}

CandlestickChart::PlotArea CandlestickChart::compute_plot() const {
    PlotArea plot{};
    plot.title_rows = options_.title.empty() ? 0 : 1;
    plot.label_rows = options_.show_labels ? 1 : 0;
    plot.volume_rows = options_.show_volume ? 2 : 0;
    plot.left = options_.show_axis ? 6 : 0;
    plot.top = plot.title_rows;
    plot.width = std::max(1, bounds_.width - plot.left);
    plot.height = std::max(1, bounds_.height - plot.top - plot.label_rows - plot.volume_rows);
    return plot;
}

double CandlestickChart::price_min() const {
    if (options_.max_value > options_.min_value && options_.min_value != 0.0) {
        return options_.min_value;
    }

    double min_v = bars_.empty() ? 0.0 : bars_.front().low;
    for (const OhlcBar& bar : bars_) {
        min_v = std::min(min_v, bar.low);
    }
    return chart_auto_min(min_v, price_max());
}

double CandlestickChart::price_max() const {
    if (options_.max_value > options_.min_value) {
        return options_.max_value;
    }

    double max_v = 0.0;
    for (const OhlcBar& bar : bars_) {
        max_v = std::max(max_v, bar.high);
    }
    return chart_auto_max(std::max(1.0, max_v));
}

double CandlestickChart::volume_max() const {
    double max_v = 0.0;
    for (const OhlcBar& bar : bars_) {
        max_v = std::max(max_v, bar.volume);
    }
    return chart_auto_max(std::max(1.0, max_v));
}

Size CandlestickChart::preferred_size() const {
    const int count = static_cast<int>(bars_.size());
    const int compact_width = count * std::max(1, options_.bar_width)
        + std::max(0, count - 1) * options_.bar_gap + 8;
    return {
        std::max(options_.min_width, compact_width),
        std::max(options_.min_height, 12 + (options_.title.empty() ? 0 : 1)),
    };
}

void CandlestickChart::paint(Canvas& canvas) const {
    if (bounds_.width <= 0 || bounds_.height <= 0 || bars_.empty()) {
        return;
    }

    const PlotArea plot = compute_plot();
    if (!options_.title.empty()) {
        canvas.draw_text({0, 0}, options_.title, options_.title_style);
    }

    const double min_v = price_min();
    const double max_v = price_max();
    const double span = std::max(1e-6, max_v - min_v);

    const int price_height = options_.show_volume ? std::max(4, plot.height * 2 / 3) : plot.height;
    ChartPlotArea price_plot{plot.left, plot.top, plot.width, price_height, plot.title_rows, 0};
    chart_paint_horizontal_grid(
        canvas,
        price_plot,
        min_v,
        max_v,
        options_.axis_style,
        options_.grid_style,
        options_.show_axis);

    const int count = static_cast<int>(bars_.size());
    const int body_w = std::max(1, options_.bar_width);
    const int gap = std::max(0, options_.bar_gap);
    const int slot = body_w + gap;
    const int total_w = count * slot - gap;
    const int start_x = options_.compact_layout
        ? plot.left + std::max(0, (plot.width - total_w) / 2)
        : plot.left;
    const int stretch_slot = options_.compact_layout
        ? slot
        : std::max(slot, plot.width / std::max(1, count));

    auto price_to_y = [&](double price) {
        return plot.top + price_height - 1
            - static_cast<int>((price - min_v) / span * (price_height - 1) + 0.5);
    };

    for (int i = 0; i < count; ++i) {
        const OhlcBar& bar = bars_[static_cast<std::size_t>(i)];
        const int slot_x = start_x + i * stretch_slot;
        const int x = slot_x + body_w / 2;
        const bool up = bar.close >= bar.open;
        Style body = up ? bar.up_style : bar.down_style;
        if (body.foreground == Color::Default) {
            body = up ? options_.up_style : options_.down_style;
        }
        if (body.foreground == Color::Default) {
            body.foreground = up ? Color::Green : Color::Red;
        }

        const int y_high = price_to_y(bar.high);
        const int y_low = price_to_y(bar.low);
        const int y_open = price_to_y(bar.open);
        const int y_close = price_to_y(bar.close);

        Style wick = options_.wick_style;
        if (wick.foreground == Color::Default) {
            wick = body;
        }

        for (int y = y_high; y <= y_low; ++y) {
            canvas.draw_text({x, y}, "|", wick);
        }

        const int body_top = std::min(y_open, y_close);
        const int body_bottom = std::max(y_open, y_close);
        const int body_left = slot_x;
        const int body_right = slot_x + body_w - 1;
        for (int y = body_top; y <= std::max(body_top, body_bottom); ++y) {
            for (int bx = body_left; bx <= body_right; ++bx) {
                canvas.draw_text({bx, y}, "█", body);
            }
        }

        if (options_.show_labels) {
            const int label_slot = options_.compact_layout ? slot : stretch_slot;
            canvas.draw_text(
                {slot_x, plot.top + plot.height - plot.volume_rows},
                bar.label.substr(0, static_cast<std::size_t>(label_slot)),
                options_.axis_style);
        }

        if (options_.show_volume && bar.volume > 0.0) {
            const double vol_max = volume_max();
            const int vol_height = std::clamp(
                static_cast<int>(bar.volume / vol_max * (plot.volume_rows - 1)),
                0,
                plot.volume_rows - 1);
            const int vol_base = plot.top + plot.height - 1;
            for (int vy = 0; vy <= vol_height; ++vy) {
                canvas.draw_text({x, vol_base - vy}, "▂", body);
            }
        }
    }
}

} // namespace tuinator
