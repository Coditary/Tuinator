#pragma once

#include <tuinator/render/style_resolver.hpp>
#include <tuinator/widgets/charts/chart_common.hpp>
#include <tuinator/widgets/charts/chart_widget.hpp>
#include <tuinator/widgets/charts/chart_widget_paint.hpp>
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

class GaugeChart : public Widget, public ChartWidget {
  public:
    GaugeChart(double value = 0.0, GaugeChartOptions options = {});

    double value() const { return value_; }
    const GaugeChartOptions& options() const { return options_; }

    void set_value(double value);
    void set_options(GaugeChartOptions options);

    std::string_view widget_type_name() const override { return "GaugeChart"; }
    void apply_stylesheet(const StyleResolver& styles) override;

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;

  private:
    void paint_arc(Canvas& canvas, int cx, int cy, int radius, const Style& fill_style, const Style& track_style,
                   const Style& tick_style) const;
    void paint_horizontal(Canvas& canvas, int x, int y, int width, const Style& fill_style,
                          const Style& track_style) const;

    double value_ = 0.0;
    GaugeChartOptions options_;
    mutable ChartPaintSupport paint_{};
};

} // namespace tuinator
