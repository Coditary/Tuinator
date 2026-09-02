#include <tuinator/widgets/chart_common.hpp>

#include <tuinator/render/glyphs.hpp>

#include <algorithm>
#include <cmath>

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
        case ChartGlyphStyle::Dots:
            return "O";
        case ChartGlyphStyle::FineDots:
        case ChartGlyphStyle::SmallDots:
            return ".";
        case ChartGlyphStyle::Blocks:
            return "#";
        case ChartGlyphStyle::Braille:
            return ":";
        default:
            break;
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

} // namespace tuinator
