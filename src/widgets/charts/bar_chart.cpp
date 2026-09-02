#include <tuinator/widgets/charts/bar_chart.hpp>

#include <tuinator/core/event.hpp>
#include <tuinator/render/text.hpp>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <variant>

namespace tuinator {

namespace {

constexpr int kAxisWidth = 6;

std::string format_axis_value(double value) {
    std::ostringstream out;
    if (std::abs(value) >= 100.0) {
        out << std::fixed << std::setprecision(0) << value;
    } else if (std::abs(value) >= 10.0) {
        out << std::fixed << std::setprecision(1) << value;
    } else {
        out << std::fixed << std::setprecision(1) << value;
    }
    return out.str();
}

} // namespace

BarChart::BarChart(std::vector<BarChartBar> bars, BarChartOptions options)
    : bars_(std::move(bars)),
      options_(std::move(options)) {}

void BarChart::set_bars(std::vector<BarChartBar> bars) {
    bars_ = std::move(bars);
    mark_dirty();
}

void BarChart::set_options(BarChartOptions options) {
    options_ = std::move(options);
    mark_dirty();
}

void BarChart::set_style(ChartGlyphStyle style) {
    options_.style = style;
    mark_dirty();
}

void BarChart::set_interactive(bool interactive) {
    options_.interactive = interactive;
    mark_dirty();
}

void BarChart::set_on_change(std::function<void(const std::vector<BarChartBar>&)> callback) {
    on_change_ = std::move(callback);
}

double BarChart::value_max() const {
    if (options_.max_value > options_.min_value) {
        return options_.max_value;
    }

    double max_v = options_.min_value;
    for (const BarChartBar& bar : bars_) {
        max_v = std::max(max_v, bar.value);
    }
    return chart_auto_max(std::max(1.0, max_v));
}

double BarChart::value_min() const {
    if (options_.max_value > options_.min_value && options_.min_value != 0.0) {
        return options_.min_value;
    }
    return 0.0;
}

double BarChart::value_span() const {
    return std::max(1e-6, value_max() - value_min());
}

BarChart::PlotArea BarChart::compute_plot() const {
    PlotArea plot{};
    plot.title_rows = options_.title.empty() ? 0 : 1;
    plot.label_rows = (options_.show_labels && options_.orientation == BarChartOrientation::Vertical)
        ? 1
        : 0;

    plot.left = options_.show_axis ? kAxisWidth : 0;
    plot.top = plot.title_rows;
    plot.width = std::max(1, bounds_.width - plot.left);
    plot.height = std::max(1, bounds_.height - plot.top - plot.label_rows);
    return plot;
}

Size BarChart::preferred_size() const {
    int label_width = 0;
    for (const BarChartBar& bar : bars_) {
        label_width = std::max(label_width, text_display_width(bar.label));
    }

    if (options_.orientation == BarChartOrientation::Horizontal) {
        const int rows = static_cast<int>(bars_.size());
        return {
            std::max(options_.min_width, label_width + 20),
            std::max(options_.min_height, rows * 2 + (options_.title.empty() ? 0 : 1)),
        };
    }

    return {
        std::max(options_.min_width, static_cast<int>(bars_.size()) * 3 + kAxisWidth),
        std::max(options_.min_height, 10 + (options_.title.empty() ? 0 : 1)),
    };
}

void BarChart::paint_grid_vertical(
    Canvas& canvas,
    const PlotArea& plot,
    double min_v,
    double max_v) const {
    if (!options_.show_grid || plot.height <= 1) {
        return;
    }

    const int lines = 4;
    for (int i = 0; i <= lines; ++i) {
        const int y = plot.top + plot.height - 1 - (i * (plot.height - 1) / lines);
        canvas.draw_hline(plot.left, y, plot.width, options_.grid_style);

        if (options_.show_axis) {
            const double value = min_v + (max_v - min_v) * static_cast<double>(i) / lines;
            const std::string label = format_axis_value(value);
            canvas.draw_text({0, y}, label, options_.axis_style);
        }
    }
}

void BarChart::paint_grid_horizontal(
    Canvas& canvas,
    const PlotArea& plot,
    double min_v,
    double max_v) const {
    if (!options_.show_grid || plot.width <= 1) {
        return;
    }

    const int lines = 4;
    for (int i = 0; i <= lines; ++i) {
        const int x = plot.left + (i * (plot.width - 1) / lines);
        canvas.draw_vline(x, plot.top, plot.height, options_.grid_style);

        if (options_.show_axis && i == lines) {
            const std::string label = format_axis_value(max_v);
            const int label_x = std::max(0, x - static_cast<int>(label.size()) + 1);
            canvas.draw_text({label_x, plot.top + plot.height}, label, options_.axis_style);
        }
        if (options_.show_axis && i == 0) {
            canvas.draw_text({plot.left, plot.top + plot.height}, format_axis_value(min_v), options_.axis_style);
        }
    }
}

void BarChart::fill_vertical_bar(
    Canvas& canvas,
    int x,
    int y,
    int width,
    int height,
    const Style& style) const {
    if (width <= 0 || height <= 0) {
        return;
    }

    if (options_.style == ChartGlyphStyle::Braille) {
        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                bool dots[8] = {};
                for (int sy = 0; sy < 4; ++sy) {
                    for (int sx = 0; sx < 2; ++sx) {
                        dots[sy * 2 + sx] = true;
                    }
                }
                canvas.draw_text({x + col, y + row}, chart_braille_from_dots(dots), style);
            }
        }
        return;
    }

    const std::string glyph = chart_glyph_for(options_.style, options_.custom_glyph);
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            canvas.draw_text({x + col, y + row}, glyph, style);
        }
    }
}

void BarChart::fill_horizontal_bar(
    Canvas& canvas,
    int x,
    int y,
    int width,
    int height,
    const Style& style) const {
    fill_vertical_bar(canvas, x, y, width, height, style);
}

void BarChart::paint_vertical(Canvas& canvas, const PlotArea& plot) const {
    if (bars_.empty()) {
        return;
    }

    const double min_v = value_min();
    const double max_v = value_max();
    const double span = value_span();

    if (!options_.title.empty()) {
        canvas.draw_text({0, 0}, options_.title, options_.title_style);
    }

    paint_grid_vertical(canvas, plot, min_v, max_v);

    const int count = static_cast<int>(bars_.size());
    const int total_gap = std::max(0, (count - 1) * options_.bar_gap);
    const int bar_width = std::max(1, (plot.width - total_gap) / count);

    for (int i = 0; i < count; ++i) {
        const BarChartBar& bar = bars_[static_cast<std::size_t>(i)];
        const int x = plot.left + i * (bar_width + options_.bar_gap);
        const int filled = std::clamp(
            static_cast<int>((bar.value - min_v) / span * plot.height),
            0,
            plot.height);
        const int y = plot.top + plot.height - filled;

        fill_vertical_bar(canvas, x, y, bar_width, filled, bar.style);

        if (options_.show_values && filled > 0) {
            std::ostringstream value;
            value << std::fixed << std::setprecision(0) << bar.value;
            const int value_y = std::max(plot.top, y - 1);
            canvas.draw_text({x, value_y}, value.str(), options_.value_style.foreground == Color::Default
                ? bar.style
                : options_.value_style);
        }

        if (options_.show_labels) {
            const std::string label = bar.label.substr(0, static_cast<std::size_t>(bar_width + 1));
            canvas.draw_text({x, plot.top + plot.height}, label, options_.axis_style);
        }
    }
}

void BarChart::paint_horizontal(Canvas& canvas, const PlotArea& plot) const {
    if (bars_.empty()) {
        return;
    }

    const double min_v = value_min();
    const double max_v = value_max();
    const double span = value_span();

    if (!options_.title.empty()) {
        canvas.draw_text({0, 0}, options_.title, options_.title_style);
    }

    paint_grid_horizontal(canvas, plot, min_v, max_v);

    int label_width = 0;
    for (const BarChartBar& bar : bars_) {
        label_width = std::max(label_width, text_display_width(bar.label));
    }
    label_width = std::min(label_width, plot.width / 3);

    const int count = static_cast<int>(bars_.size());
    const int row_height = std::max(1, plot.height / std::max(1, count));

    for (int i = 0; i < count; ++i) {
        const BarChartBar& bar = bars_[static_cast<std::size_t>(i)];
        const int y = plot.top + i * row_height;
        const int bar_area_width = std::max(1, plot.width - label_width - 1);
        const int filled = std::clamp(
            static_cast<int>((bar.value - min_v) / span * bar_area_width),
            0,
            bar_area_width);
        const int x = plot.left + label_width + 1;

        if (options_.show_labels) {
            canvas.draw_text({plot.left, y}, bar.label, options_.axis_style);
        }

        fill_horizontal_bar(canvas, x, y, filled, std::max(1, row_height - 1), bar.style);

        if (options_.show_values && filled > 0) {
            std::ostringstream value;
            value << std::fixed << std::setprecision(0) << bar.value;
            canvas.draw_text({x + filled + 1, y}, value.str(), options_.value_style.foreground == Color::Default
                ? bar.style
                : options_.value_style);
        }
    }
}

void BarChart::paint(Canvas& canvas) const {
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    const PlotArea plot = compute_plot();
    if (options_.orientation == BarChartOrientation::Horizontal) {
        paint_horizontal(canvas, plot);
    } else {
        paint_vertical(canvas, plot);
    }
}

int BarChart::bar_index_at(Point local, const PlotArea& plot) const {
    if (bars_.empty()) {
        return -1;
    }

    if (options_.orientation == BarChartOrientation::Horizontal) {
        const int count = static_cast<int>(bars_.size());
        const int row_height = std::max(1, plot.height / std::max(1, count));
        if (local.y < plot.top || local.y >= plot.top + plot.height) {
            return -1;
        }
        return std::clamp((local.y - plot.top) / row_height, 0, count - 1);
    }

    const int count = static_cast<int>(bars_.size());
    const int total_gap = std::max(0, (count - 1) * options_.bar_gap);
    const int bar_width = std::max(1, (plot.width - total_gap) / count);
    if (local.x < plot.left || local.y < plot.top || local.y >= plot.top + plot.height) {
        return -1;
    }

    const int index = (local.x - plot.left) / (bar_width + options_.bar_gap);
    return (index >= 0 && index < count) ? index : -1;
}

void BarChart::set_bar_value(int index, double value) {
    if (index < 0 || index >= static_cast<int>(bars_.size())) {
        return;
    }

    const double max_v = value_max();
    bars_[static_cast<std::size_t>(index)].value =
        std::clamp(value, value_min(), max_v);
    mark_dirty();

    if (on_change_) {
        on_change_(bars_);
    }
}

bool BarChart::handle_event(const Event& event) {
    if (!options_.interactive || bars_.empty()) {
        return false;
    }

    const PlotArea plot = compute_plot();
    const double min_v = value_min();
    const double span = value_span();

    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        const Point local{mouse->position.x - bounds_.x, mouse->position.y - bounds_.y};

        if (dragging_ || (mouse->action == MouseAction::Move && mouse->left_pressed)) {
            if (mouse->action == MouseAction::Release) {
                dragging_ = false;
                drag_bar_ = -1;
                return true;
            }

            if (mouse->action == MouseAction::Move && mouse->left_pressed) {
                if (drag_bar_ < 0) {
                    drag_bar_ = bar_index_at(local, plot);
                    dragging_ = drag_bar_ >= 0;
                }

                if (drag_bar_ >= 0) {
                    if (options_.orientation == BarChartOrientation::Vertical) {
                        const int rel_y = std::clamp(local.y, plot.top, plot.top + plot.height - 1);
                        const double ratio = 1.0 - static_cast<double>(rel_y - plot.top) / std::max(1, plot.height - 1);
                        set_bar_value(drag_bar_, min_v + ratio * span);
                    } else {
                        int label_width = 0;
                        for (const BarChartBar& bar : bars_) {
                            label_width = std::max(label_width, text_display_width(bar.label));
                        }
                        label_width = std::min(label_width, plot.width / 3);
                        const int bar_area_width = std::max(1, plot.width - label_width - 1);
                        const int rel_x = std::clamp(local.x - plot.left - label_width - 1, 0, bar_area_width);
                        const double ratio = static_cast<double>(rel_x) / std::max(1, bar_area_width);
                        set_bar_value(drag_bar_, min_v + ratio * span);
                    }
                    return true;
                }
            }
        }

        if (!contains_point(mouse->position)) {
            return false;
        }

        if (mouse->action == MouseAction::Click || mouse->action == MouseAction::Press) {
            drag_bar_ = bar_index_at(local, plot);
            dragging_ = drag_bar_ >= 0;
            return dragging_;
        }

        if (mouse->action == MouseAction::WheelUp || mouse->action == MouseAction::WheelDown) {
            const int index = bar_index_at(local, plot);
            if (index < 0) {
                return false;
            }
            const double delta = (mouse->action == MouseAction::WheelUp ? 1.0 : -1.0) * span * 0.05;
            set_bar_value(index, bars_[static_cast<std::size_t>(index)].value + delta);
            return true;
        }
    }

    return false;
}

} // namespace tuinator
