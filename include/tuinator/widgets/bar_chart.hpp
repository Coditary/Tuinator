#pragma once

#include <tuinator/widgets/chart_common.hpp>
#include <tuinator/widgets/widget.hpp>

#include <functional>
#include <string>
#include <vector>

namespace tuinator {

enum class BarChartOrientation {
    Vertical,
    Horizontal,
};

struct BarChartBar {
    std::string label;
    double value = 0.0;
    Style style{};
};

struct BarChartOptions {
    ChartGlyphStyle style = ChartGlyphStyle::Blocks;
    std::string custom_glyph;
    std::string title;
    std::string value_label;
    BarChartOrientation orientation = BarChartOrientation::Vertical;
    double min_value = 0.0;
    double max_value = 0.0;
    bool show_values = true;
    bool show_axis = true;
    bool show_grid = true;
    bool show_labels = true;
    bool interactive = false;
    int bar_gap = 1;
    int min_width = 24;
    int min_height = 8;
    Style title_style{};
    Style axis_style{};
    Style grid_style{};
    Style value_style{};
};

class BarChart : public Widget {
public:
    BarChart(std::vector<BarChartBar> bars = {}, BarChartOptions options = {});

    const std::vector<BarChartBar>& bars() const { return bars_; }
    const BarChartOptions& options() const { return options_; }

    void set_bars(std::vector<BarChartBar> bars);
    void set_options(BarChartOptions options);
    void set_style(ChartGlyphStyle style);
    void set_interactive(bool interactive);
    void set_on_change(std::function<void(const std::vector<BarChartBar>&)> callback);

    Size preferred_size() const override;
    void paint(Canvas& canvas) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return options_.interactive; }
    bool captures_pointer() const override { return options_.interactive; }
    bool pointer_active() const override { return dragging_; }

private:
    struct PlotArea {
        int left = 0;
        int top = 0;
        int width = 0;
        int height = 0;
        int title_rows = 0;
        int label_rows = 0;
    };

    PlotArea compute_plot() const;
    double value_max() const;
    double value_min() const;
    double value_span() const;
    int bar_index_at(Point local, const PlotArea& plot) const;
    void set_bar_value(int index, double value);

    void paint_vertical(Canvas& canvas, const PlotArea& plot) const;
    void paint_horizontal(Canvas& canvas, const PlotArea& plot) const;
    void paint_grid_vertical(Canvas& canvas, const PlotArea& plot, double min_v, double max_v) const;
    void paint_grid_horizontal(Canvas& canvas, const PlotArea& plot, double min_v, double max_v) const;
    void fill_vertical_bar(Canvas& canvas, int x, int y, int width, int height, const Style& style) const;
    void fill_horizontal_bar(Canvas& canvas, int x, int y, int width, int height, const Style& style) const;

    std::vector<BarChartBar> bars_;
    BarChartOptions options_;
    std::function<void(const std::vector<BarChartBar>&)> on_change_;

    bool dragging_ = false;
    int drag_bar_ = -1;
};

} // namespace tuinator
