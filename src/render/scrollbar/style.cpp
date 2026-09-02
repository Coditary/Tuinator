#include <tuinator/render/scrollbar/style.hpp>

#include <tuinator/render/theme.hpp>

#include <utility>

namespace tuinator {

namespace {

ScrollbarGlyphPatch glyph_patch_for(ScrollbarStyleKind kind) {
    ScrollbarGlyphPatch patch;
    switch (kind) {
    case ScrollbarStyleKind::Ascii:
        patch.vertical_track = "|";
        patch.horizontal_track = "-";
        patch.thumb = "#";
        patch.arrow_up = "^";
        patch.arrow_down = "v";
        patch.arrow_left = "<";
        patch.arrow_right = ">";
        patch.corner = "+";
        break;
    case ScrollbarStyleKind::Minimal:
        patch.vertical_track = "·";
        patch.horizontal_track = "·";
        patch.thumb = "█";
        patch.arrow_up = "▴";
        patch.arrow_down = "▾";
        patch.arrow_left = "◂";
        patch.arrow_right = "▸";
        patch.corner = "·";
        break;
    case ScrollbarStyleKind::Thin:
        patch.vertical_track = "│";
        patch.horizontal_track = "─";
        patch.thumb = "┃";
        patch.arrow_up = "▴";
        patch.arrow_down = "▾";
        patch.arrow_left = "◂";
        patch.arrow_right = "▸";
        patch.corner = "┴";
        break;
    default:
        break;
    }
    return patch;
}

Style dim_style() {
    Style style;
    style.dim = true;
    return style;
}

Style bold_style() {
    Style style;
    style.bold = true;
    return style;
}

ScrollbarPalettePatch palette_patch_for(ScrollbarStyleKind kind) {
    ScrollbarPalettePatch patch;
    switch (kind) {
    case ScrollbarStyleKind::Ascii:
        patch.track = dim_style();
        patch.arrow = dim_style();
        break;
    case ScrollbarStyleKind::Minimal:
        patch.track = dim_style();
        patch.arrow = dim_style();
        break;
    case ScrollbarStyleKind::Bold:
        patch.track = dim_style();
        patch.thumb = bold_style();
        patch.arrow = dim_style();
        break;
    case ScrollbarStyleKind::Thin:
        patch.track = dim_style();
        patch.arrow = dim_style();
        break;
    default:
        break;
    }
    return patch;
}

ScrollbarPalette themed_palette_for(const Theme& theme, ScrollbarStyleKind kind) {
    ScrollbarPalette palette;

    switch (kind) {
    case ScrollbarStyleKind::Bold:
        palette.thumb = theme.accent;
        palette.track = theme.muted;
        palette.arrow = theme.muted;
        break;
    case ScrollbarStyleKind::Minimal:
        palette.thumb = theme.label;
        palette.track = theme.muted;
        palette.arrow = theme.muted;
        break;
    case ScrollbarStyleKind::Thin:
        palette.thumb = theme.accent;
        palette.track = theme.border;
        palette.arrow = theme.muted;
        break;
    case ScrollbarStyleKind::Ascii:
        palette.thumb = theme.accent;
        palette.track = theme.muted;
        palette.arrow = theme.muted;
        break;
    case ScrollbarStyleKind::Classic:
    default:
        palette.thumb = theme.accent;
        palette.track = theme.muted;
        palette.arrow = theme.muted;
        break;
    }

    return palette;
}

} // namespace

ScrollbarGlyphs apply_glyph_patch(const ScrollbarGlyphs& base, const ScrollbarGlyphPatch& patch) {
    ScrollbarGlyphs glyphs = base;
    if (patch.vertical_track) {
        glyphs.vertical_track = *patch.vertical_track;
    }
    if (patch.horizontal_track) {
        glyphs.horizontal_track = *patch.horizontal_track;
    }
    if (patch.thumb) {
        glyphs.thumb = *patch.thumb;
    }
    if (patch.arrow_up) {
        glyphs.arrow_up = *patch.arrow_up;
    }
    if (patch.arrow_down) {
        glyphs.arrow_down = *patch.arrow_down;
    }
    if (patch.arrow_left) {
        glyphs.arrow_left = *patch.arrow_left;
    }
    if (patch.arrow_right) {
        glyphs.arrow_right = *patch.arrow_right;
    }
    if (patch.corner) {
        glyphs.corner = *patch.corner;
    }
    return glyphs;
}

ScrollbarPalette apply_palette_patch(const ScrollbarPalette& base, const ScrollbarPalettePatch& patch) {
    ScrollbarPalette palette = base;
    if (patch.track) {
        palette.track = *patch.track;
    }
    if (patch.thumb) {
        palette.thumb = *patch.thumb;
    }
    if (patch.arrow) {
        palette.arrow = *patch.arrow;
    }
    return palette;
}

ScrollbarGlyphs ascii_scrollbar_glyphs() {
    return apply_glyph_patch(unicode_scrollbar_glyphs(), glyph_patch_for(ScrollbarStyleKind::Ascii));
}

ScrollbarGlyphs unicode_scrollbar_glyphs() {
    return ScrollbarGlyphs{};
}

ScrollbarStyle::ScrollbarStyle(ScrollbarStyleKind kind, State state) : state_(state), kind_(kind) {}

ScrollbarStyle ScrollbarStyle::classic() {
    return {ScrollbarStyleKind::Classic, State::Preset};
}

ScrollbarStyle ScrollbarStyle::ascii() {
    return {ScrollbarStyleKind::Ascii, State::Preset};
}

ScrollbarStyle ScrollbarStyle::minimal() {
    return {ScrollbarStyleKind::Minimal, State::Preset};
}

ScrollbarStyle ScrollbarStyle::bold() {
    return {ScrollbarStyleKind::Bold, State::Preset};
}

ScrollbarStyle ScrollbarStyle::thin() {
    return {ScrollbarStyleKind::Thin, State::Preset};
}

ScrollbarStyle ScrollbarStyle::from_kind(ScrollbarStyleKind kind) {
    switch (kind) {
    case ScrollbarStyleKind::Ascii:
        return ascii();
    case ScrollbarStyleKind::Minimal:
        return minimal();
    case ScrollbarStyleKind::Bold:
        return bold();
    case ScrollbarStyleKind::Thin:
        return thin();
    case ScrollbarStyleKind::Classic:
    default:
        return classic();
    }
}

ScrollbarGlyphs ScrollbarStyle::resolve_glyphs() const {
    if (state_ == State::Derived) {
        return glyphs_;
    }

    ScrollbarGlyphs glyphs = unicode_scrollbar_glyphs();
    if (kind_ != ScrollbarStyleKind::Classic) {
        glyphs = apply_glyph_patch(glyphs, glyph_patch_for(kind_));
    }
    glyphs = apply_glyph_patch(glyphs, glyph_patch_);
    return glyphs;
}

ScrollbarPalette ScrollbarStyle::resolve_palette() const {
    if (state_ == State::Derived) {
        return palette_;
    }

    ScrollbarPalette palette;
    if (kind_ != ScrollbarStyleKind::Classic) {
        palette = apply_palette_patch(palette, palette_patch_for(kind_));
    }
    palette = apply_palette_patch(palette, palette_patch_);
    return palette;
}

ScrollbarStyle ScrollbarStyle::derive() const {
    ScrollbarStyle derived(kind_, State::Derived);
    derived.glyphs_ = resolve_glyphs();
    derived.palette_ = resolve_palette();
    return derived;
}

ScrollbarStyle ScrollbarStyle::patch_glyphs(const ScrollbarGlyphPatch& patch) const {
    ScrollbarStyle next = derive();
    next.glyphs_ = apply_glyph_patch(next.glyphs_, patch);
    return next;
}

ScrollbarStyle ScrollbarStyle::patch_palette(const ScrollbarPalettePatch& patch) const {
    ScrollbarStyle next = derive();
    next.palette_ = apply_palette_patch(next.palette_, patch);
    return next;
}

ScrollbarStyle ScrollbarStyle::with_thumb_glyph(std::string glyph) const {
    ScrollbarGlyphPatch patch;
    patch.thumb = std::move(glyph);
    return patch_glyphs(patch);
}

ScrollbarStyle ScrollbarStyle::with_track_glyph(std::string vertical, std::string horizontal) const {
    ScrollbarGlyphPatch patch;
    patch.vertical_track = std::move(vertical);
    patch.horizontal_track = std::move(horizontal);
    return patch_glyphs(patch);
}

ScrollbarStyle ScrollbarStyle::with_thumb_style(Style style) const {
    ScrollbarPalettePatch patch;
    patch.thumb = std::move(style);
    return patch_palette(patch);
}

ScrollbarStyle ScrollbarStyle::with_track_style(Style style) const {
    ScrollbarPalettePatch patch;
    patch.track = std::move(style);
    return patch_palette(patch);
}

ScrollbarStyle ScrollbarStyle::with_arrow_style(Style style) const {
    ScrollbarPalettePatch patch;
    patch.arrow = std::move(style);
    return patch_palette(patch);
}

ScrollbarStyle ScrollbarStyle::themed(const Theme& theme) const {
    ScrollbarStyle next = derive();
    const bool bold_thumb = next.palette_.thumb.bold;
    const bool dim_track = next.palette_.track.dim;
    const bool dim_arrow = next.palette_.arrow.dim;

    const ScrollbarPalette themed = themed_palette_for(theme, kind_);
    next.palette_.track = themed.track;
    next.palette_.arrow = themed.arrow;
    next.palette_.thumb = themed.thumb;

    next.palette_.track.dim = dim_track;
    next.palette_.arrow.dim = dim_arrow;
    next.palette_.thumb.bold = bold_thumb;
    return next;
}

ScrollbarGlyphs ScrollbarStyle::glyphs() const {
    return resolve_glyphs();
}

ScrollbarPalette ScrollbarStyle::palette() const {
    return resolve_palette();
}

namespace ScrollbarStyles {

ScrollbarStyle classic() {
    return ScrollbarStyle::classic();
}

ScrollbarStyle ascii() {
    return ScrollbarStyle::ascii();
}

ScrollbarStyle minimal() {
    return ScrollbarStyle::minimal();
}

ScrollbarStyle bold() {
    return ScrollbarStyle::bold();
}

ScrollbarStyle thin() {
    return ScrollbarStyle::thin();
}

ScrollbarStyle from_kind(ScrollbarStyleKind kind) {
    return ScrollbarStyle::from_kind(kind);
}

ScrollbarStyle themed(const Theme& theme, ScrollbarStyle style) {
    return style.themed(theme);
}

} // namespace ScrollbarStyles

} // namespace tuinator
