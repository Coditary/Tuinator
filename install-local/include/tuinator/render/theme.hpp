#pragma once

#include <tuinator/render/border_style.hpp>
#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/style.hpp>

#include <optional>

namespace tuinator {

struct Theme {
    Style heading;
    Style label;
    Style muted;
    Style button;
    Style button_focused;
    Style text_input;
    Style text_input_focused;
    Style border;
    Style accent;
    Style success;
    Style danger;
    BorderGlyphs glyphs = ascii_border_glyphs();
};

struct ThemeOptions {
    GlyphSet glyphs = GlyphSet::Auto;
    std::optional<BorderStyle> border_style = std::nullopt;
};

Theme dark_theme(ThemeOptions options = {});
Theme light_theme(ThemeOptions options = {});

} // namespace tuinator
