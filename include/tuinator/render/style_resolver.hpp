#pragma once

#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/render/widget_options.hpp>

#include <string>
#include <vector>

namespace tuinator {

class Stylesheet;
class Widget;

/// Resolves widget styles from the active theme and optional stylesheet.
class StyleResolver {
  public:
    StyleResolver(const Theme& theme, const Stylesheet* stylesheet);

    Style text(const Widget& widget, Style fallback = {}) const;
    Style border(const Widget& widget, Style fallback = {}) const;
    Style title(const Widget& widget, Style fallback = {}) const;
    Style divider(const Widget& widget, Style fallback = {}) const;
    Style focused(const Widget& widget, Style fallback = {}) const;
    Style selected(const Widget& widget, Style fallback = {}) const;

    BorderGlyphs border_glyphs(const Widget& widget) const;
    GlyphSet glyph_set(const Widget& widget) const;
    WidgetOptions options(const Widget& widget) const;

  private:
    const Theme& theme_;
    const Stylesheet* stylesheet_;
};

void apply_stylesheet_to_tree(Widget& root, const StyleResolver& styles,
                              std::vector<std::string>* option_warnings = nullptr);

} // namespace tuinator
