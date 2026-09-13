#include <tuinator/render/text.hpp>
#include <tuinator/widgets/charts/chart_widget.hpp>
#include <tuinator/widgets/charts/heatmap.hpp>

#include <algorithm>
#include <cmath>

namespace tuinator {

Heatmap::Heatmap(std::vector<std::vector<double>> values, std::vector<std::string> row_labels,
                 std::vector<std::string> col_labels, HeatmapOptions options)
    : values_(std::move(values)), row_labels_(std::move(row_labels)), col_labels_(std::move(col_labels)),
      options_(std::move(options)) {}

void Heatmap::set_values(std::vector<std::vector<double>> values) {
    values_ = std::move(values);
    mark_dirty();
}

void Heatmap::set_labels(std::vector<std::string> row_labels, std::vector<std::string> col_labels) {
    row_labels_ = std::move(row_labels);
    col_labels_ = std::move(col_labels);
    mark_dirty();
}

void Heatmap::set_options(HeatmapOptions options) {
    options_ = std::move(options);
    mark_dirty();
}

void Heatmap::apply_stylesheet(const StyleResolver& styles) {
    apply_chart_stylesheet(*this, styles,
                           {&options_.min_width, &options_.min_height, nullptr, nullptr, &options_.style, nullptr});
    mark_layout_dirty();
}

double Heatmap::value_min() const {
    if (options_.max_value > options_.min_value && options_.min_value != 0.0) {
        return options_.min_value;
    }

    double min_v = 0.0;
    for (const auto& row : values_) {
        for (double value : row) {
            min_v = std::min(min_v, value);
        }
    }
    return min_v;
}

double Heatmap::value_max() const {
    if (options_.max_value > options_.min_value) {
        return options_.max_value;
    }

    double max_v = 0.0;
    for (const auto& row : values_) {
        for (double value : row) {
            max_v = std::max(max_v, value);
        }
    }
    return chart_auto_max(std::max(1.0, max_v));
}

Size Heatmap::preferred_size() const {
    const int rows = static_cast<int>(values_.size());
    const int cols = rows > 0 ? static_cast<int>(values_.front().size()) : 0;
    const int gap = std::max(0, options_.cell_gap);
    const int stride = 1 + gap;
    const int grid_width = cols > 0 ? (cols - 1) * stride + 1 : 0;

    int label_width = 0;
    for (const std::string& label : row_labels_) {
        label_width = std::max(label_width, text_display_width(label));
    }

    const int title_rows = options_.title.empty() ? 0 : 1;
    const int col_label_row = options_.show_col_labels ? 1 : 0;
    const int legend_row = options_.show_legend ? 1 : 0;

    return {
        std::max(options_.min_width, grid_width + label_width + 2),
        std::max(options_.min_height, rows + title_rows + col_label_row + legend_row),
    };
}

void Heatmap::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    chart_paint_background(ctx, *this, bounds_.size());
    if (values_.empty()) {
        return;
    }

    const int rows = static_cast<int>(values_.size());
    const int cols = static_cast<int>(values_.front().size());
    if (cols <= 0) {
        return;
    }

    int label_width = 0;
    if (options_.show_row_labels) {
        for (const std::string& label : row_labels_) {
            label_width = std::max(label_width, text_display_width(label));
        }
        label_width = std::min(label_width + 1, bounds_.width / 3);
    }

    paint_.prepare(ctx, *this, options_.title_style, options_.label_style, {}, options_.label_style);

    int top = 0;
    if (!options_.title.empty()) {
        canvas.draw_text({0, top}, options_.title, paint_.styles.title);
        ++top;
    }

    const int col_label_row = options_.show_col_labels ? 1 : 0;
    const int legend_row = options_.show_legend ? 1 : 0;
    top += col_label_row;

    const int grid_top = top;
    const int gap = std::max(0, options_.cell_gap);
    const int stride = 1 + gap;
    const int grid_left = label_width;
    const int grid_right = bounds_.width;

    const double min_v = value_min();
    const double max_v = value_max();
    const double span = std::max(1e-6, max_v - min_v);

    Style low = resolve_chart_accent_style(ctx, *this, options_.low_style);
    Style high = resolve_chart_accent_style(ctx, *this, options_.high_style);
    if (low.foreground == Color::Default) {
        low.foreground = Color::Blue;
    }
    if (high.foreground == Color::Default) {
        high.foreground = Color::Red;
    }

    if (options_.show_col_labels) {
        for (int col = 0; col < cols; ++col) {
            const int col_x = grid_left + col * stride;
            if (col_x >= grid_right) {
                break;
            }
            const std::string label = col < static_cast<int>(col_labels_.size())
                                          ? col_labels_[static_cast<std::size_t>(col)]
                                          : std::to_string(col);
            const int label_cols = std::max(0, std::min(1, grid_right - col_x));
            const std::size_t bytes = text_byte_length_for_width(label, label_cols);
            canvas.draw_text({col_x, grid_top - col_label_row}, label.substr(0, bytes), paint_.styles.axis);
        }
    }

    for (int row = 0; row < rows; ++row) {
        const int y = grid_top + row;
        if (y >= bounds_.height - legend_row) {
            break;
        }
        if (options_.show_row_labels && row < static_cast<int>(row_labels_.size())) {
            canvas.draw_text({0, y}, row_labels_[static_cast<std::size_t>(row)], paint_.styles.axis);
        }

        for (int col = 0; col < cols && col < static_cast<int>(values_[static_cast<std::size_t>(row)].size()); ++col) {
            const int cell_x = grid_left + col * stride;
            if (cell_x >= grid_right) {
                break;
            }
            const double value = values_[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)];
            const double t = (value - min_v) / span;
            const Style cell_style = chart_blend_styles(low, high, t);

            chart_paint_glyph_cell(canvas, cell_x, y, options_.style, {}, cell_style);
        }
    }

    if (options_.show_legend) {
        const int y = bounds_.height - 1;
        canvas.draw_text({0, y}, chart_format_value(min_v), low);
        const std::string max_label = chart_format_value(max_v);
        canvas.draw_text({std::max(0, bounds_.width - text_display_width(max_label)), y}, max_label, high);
    }
}

} // namespace tuinator
