#include <tuinator/render/theme.hpp>

namespace tuinator {

namespace {

Style make_style(Color fg, Color bg = Color::Default, bool bold = false, bool dim = false) {
    Style style{};
    style.foreground = fg;
    style.background = bg;
    style.bold = bold;
    style.dim = dim;
    return style;
}

Theme make_theme(
    Style heading,
    Style label,
    Style muted,
    Style button,
    Style button_focused,
    Style text_input,
    Style text_input_focused,
    Style border,
    Style accent,
    Style success,
    Style danger,
    BorderGlyphs glyphs) {
    Theme theme{};
    theme.heading = heading;
    theme.label = label;
    theme.muted = muted;
    theme.button = button;
    theme.button_focused = button_focused;
    theme.text_input = text_input;
    theme.text_input_focused = text_input_focused;
    theme.border = border;
    theme.accent = accent;
    theme.success = success;
    theme.danger = danger;
    theme.glyphs = std::move(glyphs);
    return theme;
}

} // namespace

Theme dark_theme(ThemeOptions options) {
    return make_theme(
        make_style(Color::Cyan, Color::Default, true),
        make_style(Color::White),
        make_style(Color::Yellow, Color::Default, false, true),
        make_style(Color::Green),
        make_style(Color::Black, Color::Cyan, true),
        make_style(Color::White, Color::Blue),
        make_style(Color::White, Color::Blue, true),
        make_style(Color::Cyan),
        make_style(Color::Magenta, Color::Default, true),
        make_style(Color::Green),
        make_style(Color::Red),
        border_glyphs_for(options.glyphs));
}

Theme light_theme(ThemeOptions options) {
    return make_theme(
        make_style(Color::Blue, Color::Default, true),
        make_style(Color::Black),
        make_style(Color::Blue, Color::Default, false, true),
        make_style(Color::Blue),
        make_style(Color::White, Color::Blue, true),
        make_style(Color::Black, Color::White),
        make_style(Color::Black, Color::Cyan, true),
        make_style(Color::Blue),
        make_style(Color::Magenta, Color::Default, true),
        make_style(Color::Green),
        make_style(Color::Red),
        border_glyphs_for(options.glyphs));
}

} // namespace tuinator
