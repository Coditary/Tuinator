#pragma once

#include <tuinator/core/geometry.hpp>
#include <tuinator/render/canvas.hpp>
#include <tuinator/render/style.hpp>

#include <string>
#include <vector>

namespace tuinator {

enum class ChartGlyphStyle {
    Dots,
    FineDots,
    SmallDots,
    Stars,
    Hash,
    Plus,
    Blocks,
    Braille,
    Custom,
};

struct ChartGlyphStyleInfo {
    const char* id = "";
    const char* title = "";
    ChartGlyphStyle style = ChartGlyphStyle::Dots;
    const char* glyph = "";
};

const ChartGlyphStyleInfo* chart_glyph_style_named(const char* id);
const std::vector<ChartGlyphStyleInfo>& all_chart_glyph_styles();
std::string chart_glyph_for(ChartGlyphStyle style, const std::string& custom = {});

std::string chart_utf8_from(char32_t codepoint);
std::string chart_braille_from_dots(const bool dots[8]);

double chart_auto_max(double value);
double chart_auto_min(double value, double max_value);
std::string chart_format_value(double value);

struct ChartPlotArea {
    int left = 0;
    int top = 0;
    int width = 0;
    int height = 0;
    int title_rows = 0;
    int footer_rows = 0;
};

ChartPlotArea chart_compute_plot(
    const Rect& bounds,
    const std::string& title,
    int footer_rows,
    bool show_axis,
    int axis_width = 6);

void chart_paint_horizontal_grid(
    Canvas& canvas,
    const ChartPlotArea& plot,
    double min_v,
    double max_v,
    const Style& axis_style,
    const Style& grid_style,
    bool show_axis = true,
    int grid_lines = 4);

void chart_paint_glyph_cell(
    Canvas& canvas,
    int x,
    int y,
    ChartGlyphStyle style,
    const std::string& custom_glyph,
    const Style& cell_style);

Style chart_blend_styles(const Style& low, const Style& high, double t);

} // namespace tuinator
