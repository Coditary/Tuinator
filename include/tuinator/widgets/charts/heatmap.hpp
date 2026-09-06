#pragma once

#include <tuinator/widgets/charts/chart_common.hpp>
#include <tuinator/widgets/widget.hpp>

#include <string>
#include <vector>

namespace tuinator {

struct HeatmapOptions {
    std::string title;
    double min_value = 0.0;
    double max_value = 0.0;
    bool show_row_labels = true;
    bool show_col_labels = true;
    bool show_legend = true;
    ChartGlyphStyle style = ChartGlyphStyle::Blocks;
    int cell_gap = 1;
    int min_width = 20;
    int min_height = 8;
    Style title_style{};
    Style label_style{};
    Style low_style{};
    Style high_style{};
};

class Heatmap : public Widget {
  public:
    Heatmap(std::vector<std::vector<double>> values = {}, std::vector<std::string> row_labels = {},
            std::vector<std::string> col_labels = {}, HeatmapOptions options = {});

    const std::vector<std::vector<double>>& values() const { return values_; }
    const HeatmapOptions& options() const { return options_; }

    void set_values(std::vector<std::vector<double>> values);
    void set_labels(std::vector<std::string> row_labels, std::vector<std::string> col_labels);
    void set_options(HeatmapOptions options);

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;

  private:
    double value_min() const;
    double value_max() const;

    std::vector<std::vector<double>> values_;
    std::vector<std::string> row_labels_;
    std::vector<std::string> col_labels_;
    HeatmapOptions options_;
};

} // namespace tuinator
