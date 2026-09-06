#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/text.hpp>
#include <tuinator/widgets/charts/chart_common.hpp>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace tuinator {

const ChartGlyphStyleInfo* chart_glyph_style_named(const char* id) {
    for (const ChartGlyphStyleInfo& info : all_chart_glyph_styles()) {
        if (info.id == id) {
            return &info;
        }
    }
    return nullptr;
}

const std::vector<ChartGlyphStyleInfo>& all_chart_glyph_styles() {
    static const std::vector<ChartGlyphStyleInfo> styles = {
        {"dots", "Thick dots", ChartGlyphStyle::Dots, "●"},
        {"fine", "Fine dots", ChartGlyphStyle::FineDots, "·"},
        {"small", "Small dots", ChartGlyphStyle::SmallDots, "."},
        {"stars", "Stars", ChartGlyphStyle::Stars, "*"},
        {"hash", "Hash", ChartGlyphStyle::Hash, "#"},
        {"plus", "Plus", ChartGlyphStyle::Plus, "+"},
        {"blocks", "Blocks", ChartGlyphStyle::Blocks, "█"},
        {"braille", "Braille", ChartGlyphStyle::Braille, "⠿"},
    };
    return styles;
}

std::string chart_glyph_for(ChartGlyphStyle style, const std::string& custom) {
    if (style == ChartGlyphStyle::Custom && !custom.empty()) {
        return custom;
    }

    if (detect_glyph_set() == GlyphSet::Ascii) {
        switch (style) {
        case ChartGlyphStyle::Dots: return "O";
        case ChartGlyphStyle::FineDots:
        case ChartGlyphStyle::SmallDots: return ".";
        case ChartGlyphStyle::Blocks: return "#";
        case ChartGlyphStyle::Braille: return ":";
        default: break;
        }
    }

    for (const ChartGlyphStyleInfo& info : all_chart_glyph_styles()) {
        if (info.style == style) {
            return info.glyph;
        }
    }

    return custom.empty() ? "●" : custom;
}

std::string chart_utf8_from(char32_t cp) {
    std::string out;
    if (cp < 0x80) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    return out;
}

std::string chart_braille_from_dots(const bool dots[8]) {
    static const int bits[8] = {0, 3, 1, 4, 2, 5, 6, 7};
    int mask = 0;
    for (int i = 0; i < 8; ++i) {
        if (dots[i]) {
            mask |= 1 << bits[i];
        }
    }
    return chart_utf8_from(static_cast<char32_t>(0x2800 + mask));
}

double chart_auto_max(double value) {
    if (value <= 0.0) {
        return 1.0;
    }
    const double magnitude = std::pow(10.0, std::floor(std::log10(value)));
    const double normalized = value / magnitude;
    double nice = 1.0;
    if (normalized <= 1.0) {
        nice = 1.0;
    } else if (normalized <= 2.0) {
        nice = 2.0;
    } else if (normalized <= 5.0) {
        nice = 5.0;
    } else {
        nice = 10.0;
    }
    return nice * magnitude;
}

double chart_auto_min(double value, double /*max_value*/) {
    if (value >= 0.0) {
        return 0.0;
    }
    return -chart_auto_max(std::abs(value));
}

std::string chart_format_value(double value) {
    std::ostringstream out;
    if (std::abs(value) >= 100.0) {
        out << std::fixed << std::setprecision(0) << value;
    } else {
        out << std::fixed << std::setprecision(1) << value;
    }
    return out.str();
}

ChartPlotArea chart_compute_plot(const Rect& bounds, const std::string& title, int footer_rows, bool show_axis,
                                 int axis_width) {
    ChartPlotArea plot{};
    plot.title_rows = title.empty() ? 0 : 1;
    plot.footer_rows = footer_rows;
    plot.left = show_axis ? axis_width : 0;
    plot.top = plot.title_rows;
    plot.width = std::max(1, bounds.width - plot.left);
    plot.height = std::max(1, bounds.height - plot.top - plot.footer_rows);
    return plot;
}

void chart_paint_horizontal_grid(Canvas& canvas, const ChartPlotArea& plot, double min_v, double max_v,
                                 const Style& axis_style, const Style& grid_style, bool show_axis, int grid_lines) {
    if (plot.height <= 1 || grid_lines <= 0) {
        return;
    }

    for (int i = 0; i <= grid_lines; ++i) {
        const int y = plot.top + plot.height - 1 - (i * (plot.height - 1) / grid_lines);
        canvas.draw_hline(plot.left, y, plot.width, grid_style);

        if (show_axis) {
            const double value = min_v + (max_v - min_v) * static_cast<double>(i) / grid_lines;
            canvas.draw_text({0, y}, chart_format_value(value), axis_style);
        }
    }
}

void chart_paint_glyph_cell(Canvas& canvas, int x, int y, ChartGlyphStyle style, const std::string& custom_glyph,
                            const Style& cell_style) {
    if (style == ChartGlyphStyle::Braille) {
        bool dots[8] = {};
        for (int i = 0; i < 8; ++i) {
            dots[i] = true;
        }
        canvas.draw_text({x, y}, chart_braille_from_dots(dots), cell_style);
        return;
    }

    canvas.draw_text({x, y}, chart_glyph_for(style, custom_glyph), cell_style);
}

Style chart_blend_styles(const Style& low, const Style& high, double t) {
    t = std::clamp(t, 0.0, 1.0);
    Style out = low;

    if (low.foreground_rgb && high.foreground_rgb) {
        const Rgb a = *low.foreground_rgb;
        const Rgb b = *high.foreground_rgb;
        out.foreground_rgb = Rgb{
            static_cast<std::uint8_t>(a.r + (b.r - a.r) * t),
            static_cast<std::uint8_t>(a.g + (b.g - a.g) * t),
            static_cast<std::uint8_t>(a.b + (b.b - a.b) * t),
        };
        return out;
    }

    out.foreground = t < 0.5 ? low.foreground : high.foreground;
    return out;
}

} // namespace tuinator
