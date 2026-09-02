#pragma once

#include <tuinator/render/border_style.hpp>

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
/// True when the terminal locale/TERM can render UTF-8 text (independent of border style).
bool supports_unicode_text();
/// Glyph set for nvim-style file icons; stays Unicode when UTF-8 is available.
GlyphSet detect_file_icon_glyph_set();
BorderGlyphs border_glyphs_for(GlyphSet set);
BorderGlyphs border_glyphs_for(BorderStyle style);
BorderStyle border_style_from_glyph_set(GlyphSet set);
BorderGlyphs ascii_border_glyphs();
BorderGlyphs unicode_border_glyphs();
BorderGlyphs unicode_heavy_border_glyphs();
BorderGlyphs unicode_double_border_glyphs();
BorderGlyphs unicode_rounded_border_glyphs();

/// Neovim `fillchars=diff:╱` filler used by diffview.nvim (see LineIcon::DiffFill).
std::string diff_fill_glyph(GlyphSet glyphs = GlyphSet::Auto);

/// Gitsigns-style sign column bar (see LineIcon::DiffSign).
std::string diff_sign_glyph(GlyphSet glyphs = GlyphSet::Auto);

} // namespace tuinator
