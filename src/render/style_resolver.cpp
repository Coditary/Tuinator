#include <tuinator/render/style_resolver.hpp>
#include <tuinator/render/stylesheet.hpp>
#include <tuinator/render/widget_option_schema.hpp>
#include <tuinator/widgets/widget.hpp>

namespace tuinator {

StyleResolver::StyleResolver(const Theme& theme, const Stylesheet* stylesheet)
    : theme_(theme), stylesheet_(stylesheet) {}

Style StyleResolver::text(const Widget& widget, Style fallback) const {
    return stylesheet_ != nullptr ? stylesheet_->resolve_text_style(widget, fallback) : fallback;
}

Style StyleResolver::border(const Widget& widget, Style fallback) const {
    return stylesheet_ != nullptr ? stylesheet_->resolve_border_style(widget, fallback) : fallback;
}

Style StyleResolver::title(const Widget& widget, Style fallback) const {
    return stylesheet_ != nullptr ? stylesheet_->resolve_title_style(widget, fallback) : fallback;
}

Style StyleResolver::divider(const Widget& widget, Style fallback) const {
    return stylesheet_ != nullptr ? stylesheet_->resolve_divider_style(widget, fallback) : fallback;
}

Style StyleResolver::focused(const Widget& widget, Style fallback) const {
    return stylesheet_ != nullptr ? stylesheet_->resolve_focused_style(widget, fallback) : fallback;
}

Style StyleResolver::selected(const Widget& widget, Style fallback) const {
    return stylesheet_ != nullptr ? stylesheet_->resolve_selected_style(widget, fallback) : fallback;
}

BorderGlyphs StyleResolver::border_glyphs(const Widget& widget) const {
    return stylesheet_ != nullptr ? stylesheet_->resolve_border_glyphs(widget, theme_) : theme_.glyphs;
}

WidgetOptions StyleResolver::options(const Widget& widget) const {
    return stylesheet_ != nullptr ? stylesheet_->resolve_options(widget) : WidgetOptions{};
}

GlyphSet StyleResolver::glyph_set(const Widget& widget) const {
    if (stylesheet_ != nullptr) {
        const GlyphSet resolved = stylesheet_->resolve_glyph_set(widget, theme_);
        if (resolved != GlyphSet::Auto) {
            return resolved;
        }
    }

    const BorderGlyphs ascii = ascii_border_glyphs();
    if (theme_.glyphs.horizontal == ascii.horizontal && theme_.glyphs.vertical == ascii.vertical) {
        return GlyphSet::Ascii;
    }
    return GlyphSet::Unicode;
}

void apply_stylesheet_to_tree(Widget& root, const StyleResolver& styles, std::vector<std::string>* option_warnings) {
    root.for_each_descendant([&](Widget* widget) {
        widget->apply_stylesheet(styles);
        if (option_warnings != nullptr) {
            const std::vector<std::string> warnings = validate_widget_options(*widget, styles.options(*widget));
            option_warnings->insert(option_warnings->end(), warnings.begin(), warnings.end());
        }
    });
}

} // namespace tuinator
