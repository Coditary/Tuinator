#pragma once

#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/core/geometry.hpp>
#include <tuinator/render/style.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace tuinator::testing {

struct ResolvedStyle {
    Rgb foreground{};
    Rgb background{};
    bool bold = false;
    bool dim = false;
    bool reverse = false;
};

ResolvedStyle resolve_cell_style(const MemoryTerminalBackend::Cell& cell);

std::string rgb_to_hex(Rgb rgb);

struct RegionStyleAnalysis {
    int cells_sampled = 0;
    bool uniform_background = true;
    bool uniform_foreground = true;
    std::vector<Rgb> distinct_backgrounds;
    std::vector<Rgb> distinct_foregrounds;
    /// Background at vertical center, left and right interior columns of the region.
    std::optional<Rgb> left_background;
    std::optional<Rgb> right_background;
};

RegionStyleAnalysis analyze_region(const MemoryTerminalBackend& backend, Rect region);

std::string region_style_json(const RegionStyleAnalysis& analysis);

} // namespace tuinator::testing
