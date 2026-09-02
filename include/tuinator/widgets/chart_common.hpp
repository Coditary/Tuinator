#pragma once

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

} // namespace tuinator
