#pragma once

#include <tuinator/widgets/charts/chart_common.hpp>
#include <tuinator/widgets/widget.hpp>

#include <string>

namespace tuinator {

enum class GaugeStyle {
    Arc,
    Semicircle,
    Horizontal,
};

struct GaugeChartOptions {
    GaugeStyle style = GaugeStyle::Arc;
    ChartGlyphStyle glyph = ChartGlyphStyle::Blocks;
    std::string title;
    std::string unit;
    double min_value = 0.0;
    double max_value = 100.0;
    bool show_value = true;
    bool show_ticks = true;
    int diameter = 14;
    Style title_style{};
    Style track_style{};
    Style fill_style{};
    Style value_style{};
    Style tick_style{};
};

class GaugeChart : public Widget {
public:
    GaugeChart(double value = 0.0, GaugeChartOptions options = {});

    double value() const { return value_; }
    const GaugeChartOptions& options() const { return options_; }

    void set_value(double value);
    void set_options(GaugeChartOptions options);

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;

private:
    void paint_arc(Canvas& canvas, int cx, int cy, int radius) const;
    void paint_horizontal(Canvas& canvas, int x, int y, int width) const;

    double value_ = 0.0;
    GaugeChartOptions options_;
};

} // namespace tuinator
