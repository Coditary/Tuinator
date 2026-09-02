#pragma once

#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/style.hpp>

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
};

Theme dark_theme(ThemeOptions options = {});
Theme light_theme(ThemeOptions options = {});

} // namespace tuinator
