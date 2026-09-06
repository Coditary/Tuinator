#pragma once

#include <tuinator/widgets/charts/chart_common.hpp>
#include <tuinator/widgets/widget.hpp>

#include <string>
#include <vector>

namespace tuinator {

struct WaterfallStep {
    std::string label;
    double delta = 0.0;
    Style up_style{};
    Style down_style{};
};

struct WaterfallChartOptions {
    ChartGlyphStyle style = ChartGlyphStyle::Blocks;
    std::string title;
    double baseline = 0.0;
    bool show_axis = true;
    bool show_grid = true;
    bool show_labels = true;
    bool show_connectors = true;
    int bar_width = 3;
    int bar_gap = 1;
    bool compact_layout = true;
    int min_width = 28;
    int min_height = 10;
    Style title_style{};
    Style axis_style{};
    Style grid_style{};
    Style connector_style{};
    Style total_style{};
};

class WaterfallChart : public Widget {
  public:
    WaterfallChart(std::vector<WaterfallStep> steps = {}, WaterfallChartOptions options = {});

    const std::vector<WaterfallStep>& steps() const { return steps_; }
    const WaterfallChartOptions& options() const { return options_; }

    void set_steps(std::vector<WaterfallStep> steps);
    void set_options(WaterfallChartOptions options);

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;

  private:
    struct Segment {
        double start = 0.0;
        double end = 0.0;
        bool is_total = false;
    };

    std::vector<Segment> compute_segments() const;
    double range_min(const std::vector<Segment>& segments) const;
    double range_max(const std::vector<Segment>& segments) const;

    std::vector<WaterfallStep> steps_;
    WaterfallChartOptions options_;
};

} // namespace tuinator
