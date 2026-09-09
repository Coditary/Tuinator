#include <tuinator/widgets/charts/chart_widget.hpp>

#include <tuinator/render/widget_options.hpp>
#include <tuinator/widgets/charts/bar_chart.hpp>
#include <tuinator/widgets/charts/candlestick_chart.hpp>
#include <tuinator/widgets/charts/gauge_chart.hpp>
#include <tuinator/widgets/charts/heatmap.hpp>
#include <tuinator/widgets/charts/histogram.hpp>
#include <tuinator/widgets/charts/line_chart.hpp>
#include <tuinator/widgets/charts/pie_chart.hpp>
#include <tuinator/widgets/charts/stacked_area_chart.hpp>
#include <tuinator/widgets/charts/waterfall_chart.hpp>

#include <algorithm>
#include <cctype>

namespace tuinator {

namespace {

std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

template <typename Interface, typename WidgetType>
Interface* try_as(Widget* widget) {
    return dynamic_cast<WidgetType*>(widget);
}

template <typename Interface, typename WidgetType>
const Interface* try_as(const Widget* widget) {
    return dynamic_cast<const WidgetType*>(widget);
}

} // namespace

std::optional<ChartGlyphStyle> chart_glyph_style_from_string(std::string_view name) {
    const std::string lowered = lowercase(std::string(name));
    if (lowered == "dots") {
        return ChartGlyphStyle::Dots;
    }
    if (lowered == "fine-dots" || lowered == "finedots") {
        return ChartGlyphStyle::FineDots;
    }
    if (lowered == "small-dots" || lowered == "smalldots") {
        return ChartGlyphStyle::SmallDots;
    }
    if (lowered == "stars") {
        return ChartGlyphStyle::Stars;
    }
    if (lowered == "hash") {
        return ChartGlyphStyle::Hash;
    }
    if (lowered == "plus") {
        return ChartGlyphStyle::Plus;
    }
    if (lowered == "blocks") {
        return ChartGlyphStyle::Blocks;
    }
    if (lowered == "braille") {
        return ChartGlyphStyle::Braille;
    }
    if (lowered == "custom") {
        return ChartGlyphStyle::Custom;
    }
    return std::nullopt;
}

void apply_chart_stylesheet(Widget& widget, const StyleResolver& styles, ChartStylesheetPatch patch) {
    widget.Widget::apply_stylesheet(styles);
    const WidgetOptions opts = styles.options(widget);
    if (patch.min_width != nullptr && opts.has("min-width")) {
        *patch.min_width = opts.int_or("min-width", *patch.min_width);
    }
    if (patch.min_height != nullptr && opts.has("min-height")) {
        *patch.min_height = opts.int_or("min-height", *patch.min_height);
    }
    if (patch.show_axis != nullptr && opts.has("show-axis")) {
        *patch.show_axis = opts.bool_or("show-axis", *patch.show_axis);
    }
    if (patch.show_grid != nullptr && opts.has("show-grid")) {
        *patch.show_grid = opts.bool_or("show-grid", *patch.show_grid);
    }
    if (patch.glyph_style != nullptr && opts.has("glyph-style")) {
        if (const std::optional<ChartGlyphStyle> parsed =
                chart_glyph_style_from_string(opts.string_or("glyph-style", "blocks"))) {
            *patch.glyph_style = *parsed;
        }
    }
    if (patch.custom_glyph != nullptr && opts.has("custom-glyph")) {
        *patch.custom_glyph = opts.string_or("custom-glyph", *patch.custom_glyph);
    }
}

void prepare_chart_paint(const PaintContext& ctx, const Widget& widget, const Style& title, const Style& axis,
                         const Style& grid, const Style& value, ChartStyleBundle& out_styles) {
    out_styles = resolve_chart_styles(ctx, widget, title, axis, grid, value);
}

void chart_paint_background(PaintContext& ctx, const Widget& widget, Size /*size*/) {
    widget.paint_bounds_background(ctx);
}

ChartStyleBundle resolve_chart_styles(const PaintContext& ctx, const Widget& widget, const Style& title,
                                      const Style& axis, const Style& grid, const Style& value) {
    const StyleResolver& styles = ctx.styles();
    return {
        styles.title(widget, title),
        styles.text(widget, axis),
        styles.divider(widget, grid),
        styles.text(widget, value),
    };
}

Style resolve_chart_text_style(const PaintContext& ctx, const Widget& widget, const Style& fallback) {
    return ctx.styles().text(widget, fallback);
}

Style resolve_chart_accent_style(const PaintContext& ctx, const Widget& widget, const Style& fallback) {
    return ctx.styles().border(widget, fallback);
}

ChartWidget* as_chart_widget(Widget* widget) {
    if (try_as<ChartWidget, BarChart>(widget) != nullptr) {
        return try_as<ChartWidget, BarChart>(widget);
    }
    if (try_as<ChartWidget, LineChart>(widget) != nullptr) {
        return try_as<ChartWidget, LineChart>(widget);
    }
    if (try_as<ChartWidget, PieChart>(widget) != nullptr) {
        return try_as<ChartWidget, PieChart>(widget);
    }
    if (try_as<ChartWidget, Histogram>(widget) != nullptr) {
        return try_as<ChartWidget, Histogram>(widget);
    }
    if (try_as<ChartWidget, Heatmap>(widget) != nullptr) {
        return try_as<ChartWidget, Heatmap>(widget);
    }
    if (try_as<ChartWidget, GaugeChart>(widget) != nullptr) {
        return try_as<ChartWidget, GaugeChart>(widget);
    }
    if (try_as<ChartWidget, CandlestickChart>(widget) != nullptr) {
        return try_as<ChartWidget, CandlestickChart>(widget);
    }
    if (try_as<ChartWidget, StackedAreaChart>(widget) != nullptr) {
        return try_as<ChartWidget, StackedAreaChart>(widget);
    }
    if (try_as<ChartWidget, WaterfallChart>(widget) != nullptr) {
        return try_as<ChartWidget, WaterfallChart>(widget);
    }
    return nullptr;
}

const ChartWidget* as_chart_widget(const Widget* widget) {
    return as_chart_widget(const_cast<Widget*>(widget));
}

} // namespace tuinator
