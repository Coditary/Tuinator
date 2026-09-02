#include <tuinator/render/color.hpp>

#include <climits>

namespace tuinator {

namespace {

Color nearest_from_rgb(Rgb rgb) {
    constexpr Color order[] = {
        Color::Black, Color::Red, Color::Green, Color::Yellow,
        Color::Blue, Color::Magenta, Color::Cyan, Color::White,
    };

    constexpr Rgb samples[] = {
        {0, 0, 0},
        {220, 50, 47},
        {80, 200, 120},
        {220, 200, 50},
        {80, 120, 220},
        {200, 80, 200},
        {80, 200, 220},
        {230, 230, 230},
    };

    Color best = Color::White;
    int best_distance = INT32_MAX;

    for (std::size_t i = 0; i < sizeof(order) / sizeof(order[0]); ++i) {
        const Rgb& sample = samples[i];
        const int dr = static_cast<int>(rgb.r) - static_cast<int>(sample.r);
        const int dg = static_cast<int>(rgb.g) - static_cast<int>(sample.g);
        const int db = static_cast<int>(rgb.b) - static_cast<int>(sample.b);
        const int distance = dr * dr + dg * dg + db * db;
        if (distance < best_distance) {
            best_distance = distance;
            best = order[i];
        }
    }

    return best;
}

} // namespace

Color nearest_ansi_color(Rgb rgb) {
    return nearest_from_rgb(rgb);
}

ColorValue ColorValue::from_hex(std::uint32_t hex) {
    const Rgb value = Rgb::hex(hex);
    return {nearest_from_rgb(value), value};
}

ColorValue ColorValue::from_rgb(Rgb value) {
    return {nearest_from_rgb(value), value};
}

ColorValue ColorValue::from_ansi(Color color) {
    return {color, std::nullopt};
}

Style ColorValue::foreground_style(Style base) const {
    base.foreground = ansi;
    base.foreground_rgb = rgb;
    return base;
}

Style ColorValue::background_style(Style base) const {
    base.background = ansi;
    base.background_rgb = rgb;
    return base;
}

Style ColorValue::ansi_foreground_style(Style base) const {
    base.foreground = ansi;
    base.foreground_rgb = std::nullopt;
    return base;
}

Style ColorValue::ansi_background_style(Style base) const {
    base.background = ansi;
    base.background_rgb = std::nullopt;
    return base;
}

Style style_hex_fg(std::uint32_t hex, Style base) {
    return ColorValue::from_hex(hex).foreground_style(base);
}

Style style_hex_bg(std::uint32_t hex, Style base) {
    return ColorValue::from_hex(hex).background_style(base);
}

Style style_hex_fg_bg(std::uint32_t fg_hex, std::uint32_t bg_hex, Style base) {
    const ColorValue fg = ColorValue::from_hex(fg_hex);
    const ColorValue bg = ColorValue::from_hex(bg_hex);
    base.foreground = fg.ansi;
    base.background = bg.ansi;
    base.foreground_rgb = fg.rgb;
    base.background_rgb = bg.rgb;
    return base;
}

} // namespace tuinator
