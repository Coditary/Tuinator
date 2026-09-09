#pragma once

#include <tuinator/render/style_resolver.hpp>
#include <tuinator/widgets/charts/chart_common.hpp>
#include <tuinator/widgets/charts/chart_widget.hpp>
#include <tuinator/widgets/charts/chart_widget_paint.hpp>
#include <tuinator/widgets/widget.hpp>

#include <string>
#include <vector>

namespace tuinator {

struct OhlcBar {
    std::string label;
    double open = 0.0;
    double high = 0.0;
    double low = 0.0;
    double close = 0.0;
    double volume = 0.0;
    Style up_style{};
    Style down_style{};
};

struct CandlestickChartOptions {
    std::string title;
    double min_value = 0.0;
    double max_value = 0.0;
    bool show_axis = true;
    bool show_grid = true;
    bool show_labels = true;
    bool show_volume = false;
    int bar_width = 3;
    int bar_gap = 1;
    bool compact_layout = true;
    int min_width = 28;
    int min_height = 10;
    Style title_style{};
    Style axis_style{};
    Style grid_style{};
    Style up_style{};
    Style down_style{};
    Style wick_style{};
};

class CandlestickChart : public Widget, public ChartWidget {
  public:
    CandlestickChart(std::vector<OhlcBar> bars = {}, CandlestickChartOptions options = {});

    const std::vector<OhlcBar>& bars() const { return bars_; }
    const CandlestickChartOptions& options() const { return options_; }

    void set_bars(std::vector<OhlcBar> bars);
    void set_options(CandlestickChartOptions options);

    std::string_view widget_type_name() const override { return "CandlestickChart"; }
    void apply_stylesheet(const StyleResolver& styles) override;

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;

  private:
    struct PlotArea {
        int left = 0;
        int top = 0;
        int width = 0;
        int height = 0;
        int title_rows = 0;
        int label_rows = 0;
        int volume_rows = 0;
    };

    PlotArea compute_plot() const;
    double price_min() const;
    double price_max() const;
    double volume_max() const;

    std::vector<OhlcBar> bars_;
    CandlestickChartOptions options_;
    mutable ChartPaintSupport paint_{};
};

} // namespace tuinator
