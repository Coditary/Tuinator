#pragma once

#include <cstdint>
#include <optional>

namespace tuinator {

enum class Color : std::uint8_t {
    Default = 0,
    Black,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
};

struct Rgb {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;

    constexpr Rgb() = default;
    constexpr Rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b) : r(r), g(g), b(b) {}

    static constexpr Rgb hex(std::uint32_t value) {
        return {
            static_cast<std::uint8_t>((value >> 16U) & 0xFFU),
            static_cast<std::uint8_t>((value >> 8U) & 0xFFU),
            static_cast<std::uint8_t>(value & 0xFFU),
        };
    }

    constexpr bool operator==(const Rgb& other) const {
        return r == other.r && g == other.g && b == other.b;
    }

    constexpr bool operator!=(const Rgb& other) const { return !(*this == other); }
};

struct Style {
    Color foreground = Color::Default;
    Color background = Color::Default;
    std::optional<Rgb> foreground_rgb;
    std::optional<Rgb> background_rgb;
    bool bold = false;
    bool dim = false;
    bool reverse = false;
};

constexpr Rgb rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    return Rgb{r, g, b};
}

inline Style style_fg(Rgb color) {
    Style style{};
    style.foreground_rgb = color;
    return style;
}

inline Style style_bg(Rgb color) {
    Style style{};
    style.background_rgb = color;
    return style;
}

inline Style style_fg_bg(Rgb foreground, Rgb background) {
    Style style{};
    style.foreground_rgb = foreground;
    style.background_rgb = background;
    return style;
}

} // namespace tuinator
