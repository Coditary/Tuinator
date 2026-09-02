#include <tuinator/widgets/charts/gauge_chart.hpp>

#include <tuinator/render/text.hpp>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace tuinator {

namespace {

constexpr double kPi = 3.141592653589793;
constexpr double kTerminalAspect = 2.0;

} // namespace

GaugeChart::GaugeChart(double value, GaugeChartOptions options)
    : value_(value),
      options_(std::move(options)) {}

void GaugeChart::set_value(double value) {
    value_ = std::clamp(value, options_.min_value, options_.max_value);
    mark_dirty();
}

void GaugeChart::set_options(GaugeChartOptions options) {
    options_ = std::move(options);
    set_value(value_);
}

Size GaugeChart::preferred_size() const {
    if (options_.style == GaugeStyle::Horizontal) {
        return {std::max(20, options_.diameter * 2), 4 + (options_.title.empty() ? 0 : 1)};
    }

    const int diameter = std::max(8, options_.diameter);
    return {diameter + 2, diameter / 2 + 3 + (options_.title.empty() ? 0 : 1)};
}

void GaugeChart::paint_arc(Canvas& canvas, int cx, int cy, int radius) const {
    const double span = std::max(1e-6, options_.max_value - options_.min_value);
    const double ratio = (value_ - options_.min_value) / span;
    const double start_angle = kPi;
    const double end_angle = 0.0;
    const double value_angle = start_angle + (end_angle - start_angle) * ratio;

    for (int y = cy - radius; y <= cy; ++y) {
        for (int x = cx - radius; x <= cx + radius; ++x) {
            const double dx = static_cast<double>(x - cx);
            const double dy = static_cast<double>(y - cy) * kTerminalAspect;
            const double dist = std::sqrt(dx * dx + dy * dy);
            if (dist > radius || dist < radius - 1.5 || y > cy) {
                continue;
            }

            double angle = std::atan2(dy, dx);
            if (angle > 0.0) {
                angle -= kPi * 2.0;
            }

            const bool filled = angle >= value_angle;
            chart_paint_glyph_cell(
                canvas,
                x,
                y,
                options_.glyph,
                {},
                filled ? options_.fill_style : options_.track_style);
        }
    }

    if (options_.show_ticks) {
        for (int tick = 0; tick <= 4; ++tick) {
            const double t = static_cast<double>(tick) / 4.0;
            const double angle = start_angle + (end_angle - start_angle) * t;
            const int tx = cx + static_cast<int>(std::cos(angle) * (radius + 1));
            const int ty = cy + static_cast<int>(std::sin(angle) * (radius + 1) / kTerminalAspect);
            canvas.draw_text({tx, ty}, "|", options_.tick_style);
        }
    }
}

void GaugeChart::paint_horizontal(Canvas& canvas, int x, int y, int width) const {
    const double span = std::max(1e-6, options_.max_value - options_.min_value);
    const int filled = std::clamp(
        static_cast<int>((value_ - options_.min_value) / span * width),
        0,
        width);

    for (int col = 0; col < width; ++col) {
        chart_paint_glyph_cell(
            canvas,
            x + col,
            y,
            options_.glyph,
            {},
            col < filled ? options_.fill_style : options_.track_style);
    }
}

void GaugeChart::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    int top = 0;
    if (!options_.title.empty()) {
        canvas.draw_text({0, top}, options_.title, options_.title_style);
        ++top;
    }

    if (options_.style == GaugeStyle::Horizontal) {
        const int width = std::max(8, bounds_.width - 2);
        paint_horizontal(canvas, 1, top + 1, width);

        if (options_.show_value) {
            std::ostringstream out;
            out << std::fixed << std::setprecision(0) << value_;
            if (!options_.unit.empty()) {
                out << options_.unit;
            }
            canvas.draw_text({0, top}, out.str(), options_.value_style);
        }
        return;
    }

    const int radius = std::max(3, std::min(options_.diameter / 2, (bounds_.width - 2) / 2));
    const int cx = bounds_.width / 2;
    const int cy = top + radius;
    paint_arc(canvas, cx, cy, radius);

    if (options_.show_value) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(0) << value_;
        if (!options_.unit.empty()) {
            out << options_.unit;
        }
        const std::string text = out.str();
        const int x = std::max(0, cx - text_display_width(text) / 2);
        canvas.draw_text({x, cy - 1}, text, options_.value_style);
    }
}

} // namespace tuinator
