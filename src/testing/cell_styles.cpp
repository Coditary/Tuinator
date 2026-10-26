#include <tuinator/testing/cell_styles.hpp>

#include <cstdio>
#include <sstream>
#include <unordered_map>

namespace tuinator::testing {

namespace {

constexpr Rgb kDefaultForeground{200, 200, 200};
constexpr Rgb kDefaultBackground{30, 30, 30};

Rgb ansi_color(Color color) {
    switch (color) {
    case Color::Black: return {0, 0, 0};
    case Color::Red: return {220, 50, 47};
    case Color::Green: return {80, 200, 120};
    case Color::Yellow: return {220, 200, 50};
    case Color::Blue: return {80, 120, 220};
    case Color::Magenta: return {200, 80, 200};
    case Color::Cyan: return {80, 200, 220};
    case Color::White: return {230, 230, 230};
    default: return kDefaultForeground;
    }
}

Rgb resolve_foreground_raw(const Style& style) {
    if (style.foreground_rgb.has_value()) {
        return *style.foreground_rgb;
    }
    if (style.foreground != Color::Default) {
        return ansi_color(style.foreground);
    }
    return kDefaultForeground;
}

Rgb resolve_background_raw(const Style& style) {
    if (style.background_rgb.has_value()) {
        return *style.background_rgb;
    }
    if (style.background != Color::Default) {
        return ansi_color(style.background);
    }
    return kDefaultBackground;
}

struct RgbHash {
    std::size_t operator()(Rgb rgb) const {
        return (static_cast<std::size_t>(rgb.r) << 16U) | (static_cast<std::size_t>(rgb.g) << 8U) | rgb.b;
    }
};

struct RgbEq {
    bool operator()(Rgb a, Rgb b) const { return a.r == b.r && a.g == b.g && a.b == b.b; }
};

std::optional<Rgb> sample_background_at(const MemoryTerminalBackend& backend, int x, int y) {
    const auto& cells = backend.cells();
    if (y < 0 || y >= static_cast<int>(cells.size())) {
        return std::nullopt;
    }
    if (x < 0 || x >= static_cast<int>(cells[static_cast<std::size_t>(y)].size())) {
        return std::nullopt;
    }
    return resolve_cell_style(cells[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)]).background;
}

} // namespace

ResolvedStyle resolve_cell_style(const MemoryTerminalBackend::Cell& cell) {
    ResolvedStyle out;
    out.foreground = resolve_foreground_raw(cell.style);
    out.background = resolve_background_raw(cell.style);
    if (cell.style.reverse) {
        std::swap(out.foreground, out.background);
    }
    out.bold = cell.style.bold;
    out.dim = cell.style.dim;
    out.reverse = cell.style.reverse;
    return out;
}

std::string rgb_to_hex(Rgb rgb) {
    char buffer[8];
    std::snprintf(buffer, sizeof(buffer), "#%02x%02x%02x", rgb.r, rgb.g, rgb.b);
    return buffer;
}

RegionStyleAnalysis analyze_region(const MemoryTerminalBackend& backend, Rect region) {
    RegionStyleAnalysis analysis;
    const auto& cells = backend.cells();
    if (cells.empty() || region.width <= 0 || region.height <= 0) {
        return analysis;
    }

    std::unordered_map<Rgb, int, RgbHash, RgbEq> bg_counts;
    std::unordered_map<Rgb, int, RgbHash, RgbEq> fg_counts;

    const int y_end = std::min(region.bottom(), static_cast<int>(cells.size()));
    for (int y = std::max(0, region.y); y < y_end; ++y) {
        const int row_width = static_cast<int>(cells[static_cast<std::size_t>(y)].size());
        const int x_end = std::min(region.right(), row_width);
        for (int x = std::max(0, region.x); x < x_end; ++x) {
            const ResolvedStyle resolved = resolve_cell_style(cells[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)]);
            ++bg_counts[resolved.background];
            ++fg_counts[resolved.foreground];
            ++analysis.cells_sampled;
        }
    }

    analysis.uniform_background = bg_counts.size() <= 1;
    analysis.uniform_foreground = fg_counts.size() <= 1;

    for (const auto& entry : bg_counts) {
        analysis.distinct_backgrounds.push_back(entry.first);
    }
    for (const auto& entry : fg_counts) {
        analysis.distinct_foregrounds.push_back(entry.first);
    }

    const int center_y = region.y + region.height / 2;
    const int left_x = region.x + 1;
    const int right_x = region.right() - 2;
    analysis.left_background = sample_background_at(backend, left_x, center_y);
    analysis.right_background = sample_background_at(backend, right_x, center_y);

    return analysis;
}

std::string region_style_json(const RegionStyleAnalysis& analysis) {
    std::ostringstream out;
    out << '{';
    out << "\"cells_sampled\":" << analysis.cells_sampled;
    out << ",\"uniform_background\":" << (analysis.uniform_background ? "true" : "false");
    out << ",\"uniform_foreground\":" << (analysis.uniform_foreground ? "true" : "false");

    out << ",\"backgrounds\":[";
    for (std::size_t i = 0; i < analysis.distinct_backgrounds.size(); ++i) {
        if (i > 0) {
            out << ',';
        }
        out << '"' << rgb_to_hex(analysis.distinct_backgrounds[i]) << '"';
    }
    out << ']';

    out << ",\"foregrounds\":[";
    for (std::size_t i = 0; i < analysis.distinct_foregrounds.size(); ++i) {
        if (i > 0) {
            out << ',';
        }
        out << '"' << rgb_to_hex(analysis.distinct_foregrounds[i]) << '"';
    }
    out << ']';

    out << ",\"edges\":{";
    if (analysis.left_background.has_value()) {
        out << "\"left_background\":\"" << rgb_to_hex(*analysis.left_background) << '"';
    }
    if (analysis.right_background.has_value()) {
        if (analysis.left_background.has_value()) {
            out << ',';
        }
        out << "\"right_background\":\"" << rgb_to_hex(*analysis.right_background) << '"';
    }
    out << '}';
    out << '}';
    return out.str();
}

} // namespace tuinator::testing
