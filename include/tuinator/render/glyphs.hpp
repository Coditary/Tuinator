#pragma once

#include <string>

namespace tuinator {

/// Border character set used for boxes, panels, windows, and dividers.
enum class GlyphSet {
    Auto,
    Ascii,
    Unicode,
    UnicodeRounded,
};

struct BorderGlyphs {
    std::string top_left;
    std::string top_right;
    std::string bottom_left;
    std::string bottom_right;
    std::string horizontal;
    std::string vertical;
    std::string resize_handle;
};

GlyphSet detect_glyph_set();
BorderGlyphs border_glyphs_for(GlyphSet set);
BorderGlyphs ascii_border_glyphs();
BorderGlyphs unicode_border_glyphs();
BorderGlyphs unicode_rounded_border_glyphs();

} // namespace tuinator
