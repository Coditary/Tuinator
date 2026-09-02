#pragma once

#include <tuinator/render/style.hpp>

#include <cstdint>
#include <optional>

namespace tuinator {

/// Maps an RGB value to the closest standard ANSI palette color.
Color nearest_ansi_color(Rgb rgb);

/// A color that carries both RGB (for true color) and an ANSI fallback.
struct ColorValue {
    Color ansi = Color::Default;
    std::optional<Rgb> rgb;

    static ColorValue from_hex(std::uint32_t hex);
    static ColorValue from_rgb(Rgb value);
    static ColorValue from_ansi(Color color);

    Style foreground_style(Style base = {}) const;
    Style background_style(Style base = {}) const;

    /// Same color mapped to the nearest ANSI palette entry only (no RGB).
    Style ansi_foreground_style(Style base = {}) const;
    Style ansi_background_style(Style base = {}) const;
};

Style style_hex_fg(std::uint32_t hex, Style base = {});
Style style_hex_bg(std::uint32_t hex, Style base = {});
Style style_hex_fg_bg(std::uint32_t fg_hex, std::uint32_t bg_hex, Style base = {});

} // namespace tuinator
