#pragma once

#include <tuinator/render/border_style.hpp>
#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/render/widget_options.hpp>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace tuinator {

class Widget;

/// Partial style values that can be merged into a full Style.
struct StylePatch {
    std::optional<Color> foreground;
    std::optional<Color> background;
    std::optional<Rgb> foreground_rgb;
    std::optional<Rgb> background_rgb;
    std::optional<bool> bold;
    std::optional<bool> dim;
    std::optional<bool> reverse;
};

void apply_style_patch(Style& style, const StylePatch& patch);
void merge_style_patch(StylePatch& target, const StylePatch& source);

/// CSS-like stylesheet for hierarchical widget styling.
class Stylesheet {
  public:
    Stylesheet();
    Stylesheet(const Stylesheet& other);
    Stylesheet(Stylesheet&& other) noexcept;
    ~Stylesheet();

    Stylesheet& operator=(const Stylesheet& other);
    Stylesheet& operator=(Stylesheet&& other) noexcept;

    static Stylesheet load_from_string(std::string_view text);
    static Stylesheet load_from_file(const std::filesystem::path& path);

    ThemeOptions theme_options() const;

    Style resolve_text_style(const Widget& widget, Style fallback = {}) const;
    Style resolve_border_style(const Widget& widget, Style fallback = {}) const;
    Style resolve_title_style(const Widget& widget, Style fallback = {}) const;
    Style resolve_divider_style(const Widget& widget, Style fallback = {}) const;
    Style resolve_focused_style(const Widget& widget, Style fallback = {}) const;
    Style resolve_selected_style(const Widget& widget, Style fallback = {}) const;

    BorderGlyphs resolve_border_glyphs(const Widget& widget, const Theme& theme) const;
    GlyphSet resolve_glyph_set(const Widget& widget, const Theme& theme) const;

    WidgetOptions resolve_options(const Widget& widget) const;

    Style resolve_theme_token(std::string_view name, Style fallback = {}) const;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace tuinator
