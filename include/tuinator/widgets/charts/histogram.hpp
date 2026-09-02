#pragma once

#include <tuinator/widgets/charts/chart_common.hpp>
#include <tuinator/widgets/widget.hpp>

#include <string>
#include <vector>

namespace tuinator {

struct HistogramBin {
    std::string label;
    double count = 0.0;
    Style style{};
};

struct HistogramOptions {
    ChartGlyphStyle style = ChartGlyphStyle::Blocks;
    std::string custom_glyph;
    std::string title;
    double min_value = 0.0;
    double max_value = 0.0;
    bool show_axis = true;
    bool show_grid = true;
    bool show_labels = true;
    bool show_counts = false;
    int min_width = 24;
    int min_height = 8;
    Style title_style{};
    Style axis_style{};
    Style grid_style{};
    Style count_style{};
};

class Histogram : public Widget {
public:
    Histogram(std::vector<HistogramBin> bins = {}, HistogramOptions options = {});

    const std::vector<HistogramBin>& bins() const { return bins_; }
    const HistogramOptions& options() const { return options_; }

    void set_bins(std::vector<HistogramBin> bins);
    void set_options(HistogramOptions options);

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;

private:
    std::vector<HistogramBin> bins_;
    HistogramOptions options_;
};

} // namespace tuinator
