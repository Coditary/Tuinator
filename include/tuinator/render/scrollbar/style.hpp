#pragma once

#include <tuinator/render/style.hpp>

#include <optional>
#include <string>

namespace tuinator {

struct Theme;

struct ScrollbarGlyphs {
    std::string vertical_track = "|";
    std::string horizontal_track = "-";
    std::string thumb = "█";
    std::string arrow_up = "▲";
    std::string arrow_down = "▼";
    std::string arrow_left = "◄";
    std::string arrow_right = "►";
    std::string corner = "+";
};

struct ScrollbarGlyphPatch {
    std::optional<std::string> vertical_track;
    std::optional<std::string> horizontal_track;
    std::optional<std::string> thumb;
    std::optional<std::string> arrow_up;
    std::optional<std::string> arrow_down;
    std::optional<std::string> arrow_left;
    std::optional<std::string> arrow_right;
    std::optional<std::string> corner;
};

struct ScrollbarPalette {
    Style track{};
    Style thumb{};
    Style arrow{};
};

struct ScrollbarPalettePatch {
    std::optional<Style> track;
    std::optional<Style> thumb;
    std::optional<Style> arrow;
};

ScrollbarGlyphs apply_glyph_patch(const ScrollbarGlyphs& base, const ScrollbarGlyphPatch& patch);
ScrollbarPalette apply_palette_patch(const ScrollbarPalette& base, const ScrollbarPalettePatch& patch);

enum class ScrollbarStyleKind {
    Classic,
    Ascii,
    Minimal,
    Bold,
    Thin,
};

class ScrollbarStyle {
public:
    static ScrollbarStyle classic();
    static ScrollbarStyle ascii();
    static ScrollbarStyle minimal();
    static ScrollbarStyle bold();
    static ScrollbarStyle thin();
    static ScrollbarStyle from_kind(ScrollbarStyleKind kind);

    ScrollbarStyle derive() const;

    ScrollbarStyle patch_glyphs(const ScrollbarGlyphPatch& patch) const;
    ScrollbarStyle patch_palette(const ScrollbarPalettePatch& patch) const;

    ScrollbarStyle with_thumb_glyph(std::string glyph) const;
    ScrollbarStyle with_track_glyph(std::string vertical, std::string horizontal) const;
    ScrollbarStyle with_thumb_style(Style style) const;
    ScrollbarStyle with_track_style(Style style) const;
    ScrollbarStyle with_arrow_style(Style style) const;

    ScrollbarStyle themed(const Theme& theme) const;

    ScrollbarGlyphs glyphs() const;
    ScrollbarPalette palette() const;

    ScrollbarStyleKind kind() const { return kind_; }

private:
    enum class State {
        Preset,
        Derived,
    };

    ScrollbarStyle(ScrollbarStyleKind kind, State state);

    ScrollbarGlyphs resolve_glyphs() const;
    ScrollbarPalette resolve_palette() const;

    State state_ = State::Preset;
    ScrollbarStyleKind kind_ = ScrollbarStyleKind::Classic;
    ScrollbarGlyphs glyphs_{};
    ScrollbarPalette palette_{};
    ScrollbarGlyphPatch glyph_patch_{};
    ScrollbarPalettePatch palette_patch_{};
};

namespace ScrollbarStyles {

ScrollbarStyle classic();
ScrollbarStyle ascii();
ScrollbarStyle minimal();
ScrollbarStyle bold();
ScrollbarStyle thin();
ScrollbarStyle from_kind(ScrollbarStyleKind kind);
ScrollbarStyle themed(const Theme& theme, ScrollbarStyle style);

} // namespace ScrollbarStyles

ScrollbarGlyphs ascii_scrollbar_glyphs();
ScrollbarGlyphs unicode_scrollbar_glyphs();

} // namespace tuinator
