#pragma once

#include <tuinator/render/paint_context.hpp>
#include <tuinator/render/style_resolver.hpp>
#include <tuinator/widgets/charts/chart_common.hpp>
#include <tuinator/widgets/widget.hpp>

#include <string>

namespace tuinator {

/// Marker capability for chart widgets (BarChart, LineChart, …).
class ChartWidget {
  public:
    virtual ~ChartWidget() = default;
};

struct ChartStyleBundle {
    Style title{};
    Style axis{};
    Style grid{};
    Style value{};
};

std::optional<ChartGlyphStyle> chart_glyph_style_from_string(std::string_view name);

struct ChartStylesheetPatch {
    int* min_width = nullptr;
    int* min_height = nullptr;
    bool* show_axis = nullptr;
    bool* show_grid = nullptr;
    ChartGlyphStyle* glyph_style = nullptr;
    std::string* custom_glyph = nullptr;
};

void apply_chart_stylesheet(Widget& widget, const StyleResolver& styles, ChartStylesheetPatch patch);

void prepare_chart_paint(const PaintContext& ctx, const Widget& widget, const Style& title, const Style& axis,
                         const Style& grid, const Style& value, ChartStyleBundle& out_styles);

/// Erase the widget bounds before painting chart content (handles empty data and partial redraws).
void chart_paint_background(PaintContext& ctx, const Widget& widget, Size size);

ChartStyleBundle resolve_chart_styles(const PaintContext& ctx, const Widget& widget, const Style& title,
                                      const Style& axis, const Style& grid, const Style& value);

/// Resolve a chart option style through the stylesheet text role.
Style resolve_chart_text_style(const PaintContext& ctx, const Widget& widget, const Style& fallback);

/// Resolve fill/bar/series option styles through the stylesheet border role.
Style resolve_chart_accent_style(const PaintContext& ctx, const Widget& widget, const Style& fallback);

ChartWidget* as_chart_widget(Widget* widget);
const ChartWidget* as_chart_widget(const Widget* widget);

} // namespace tuinator
