#include <tuinator/core/event.hpp>
#include <tuinator/render/text.hpp>
#include <tuinator/widgets/controls/checkbox.hpp>

#include <algorithm>
#include <string>
#include <variant>

namespace tuinator {

namespace {

bool style_is_empty(const Style& style) {
    return style.foreground == Color::Default && !style.foreground_rgb.has_value() &&
           style.background == Color::Default && !style.background_rgb.has_value() && !style.bold && !style.dim &&
           !style.reverse;
}

void merge_style(Style& target, const std::optional<Style>& overlay) {
    if (!overlay.has_value()) {
        return;
    }
    if (overlay->foreground != Color::Default) {
        target.foreground = overlay->foreground;
    }
    if (overlay->foreground_rgb.has_value()) {
        target.foreground_rgb = overlay->foreground_rgb;
    }
    if (overlay->background != Color::Default) {
        target.background = overlay->background;
    }
    if (overlay->background_rgb.has_value()) {
        target.background_rgb = overlay->background_rgb;
    }
    if (overlay->bold) {
        target.bold = true;
    }
    if (overlay->dim) {
        target.dim = true;
    }
    if (overlay->reverse) {
        target.reverse = true;
    }
}

Style make_label_style(Color color) {
    Style style;
    style.foreground = color;
    return style;
}

} // namespace

const std::vector<CheckboxStyle>& all_checkbox_styles() {
    static const std::vector<CheckboxStyle> styles = {
        {
            "basic",
            "Basic",
            {"□", "☑", " "},
            std::nullopt,
            std::nullopt,
            std::nullopt,
            {"default", "square"},
        },
        {
            "info",
            "Info",
            {"□", "☑", " "},
            std::nullopt,
            std::nullopt,
            make_label_style(Color::Cyan),
            {"information"},
        },
        {
            "warning",
            "Warning",
            {"□", "☑", " "},
            std::nullopt,
            std::nullopt,
            make_label_style(Color::Yellow),
            {},
        },
        {
            "error",
            "Error",
            {"□", "☑", " "},
            std::nullopt,
            std::nullopt,
            make_label_style(Color::Red),
            {"danger"},
        },
        {
            "checkmark",
            "Check mark",
            {"☐", "✅", " "},
            std::nullopt,
            std::nullopt,
            std::nullopt,
            {"emoji", "done"},
        },
        {
            "circle",
            "Circle",
            {"○", "●", " "},
            std::nullopt,
            std::nullopt,
            std::nullopt,
            {"dot"},
        },
        {
            "diamond",
            "Diamond",
            {"◇", "◆", " "},
            std::nullopt,
            std::nullopt,
            std::nullopt,
            {"gem"},
        },
        {
            "ascii",
            "ASCII",
            {"[ ]", "[X]", " "},
            std::nullopt,
            std::nullopt,
            std::nullopt,
            {"brackets"},
        },
        {
            "asterisk",
            "Asterisk",
            {"[ ]", "[*]", " "},
            std::nullopt,
            std::nullopt,
            std::nullopt,
            {"star"},
        },
        {
            "plus",
            "Plus",
            {"[ ]", "[+]", " "},
            std::nullopt,
            std::nullopt,
            std::nullopt,
            {"add"},
        },
        {
            "xo",
            "X/O",
            {"( )", "(O)", " "},
            std::nullopt,
            std::nullopt,
            std::nullopt,
            {"paren"},
        },
    };
    return styles;
}

const CheckboxStyle* checkbox_style_named(std::string_view name) {
    for (const CheckboxStyle& style : all_checkbox_styles()) {
        if (name == style.id || name == style.title) {
            return &style;
        }
        for (const std::string& alias : style.aliases) {
            if (name == alias) {
                return &style;
            }
        }
    }
    return nullptr;
}

CheckboxOptions checkbox_options_default(const Theme& theme) {
    CheckboxOptions options;
    options.glyphs = {"□", "☑", " "};
    options.marker_style = theme.label;
    options.marker_checked_style = theme.label;
    options.label_style = theme.label;
    options.label_checked_style = theme.label;
    options.focused_style = theme.label;
    options.focused_style.background_rgb = Rgb::hex(0x505050);
    options.focused_style.foreground = Color::Default;
    options.focused_style.bold = false;
    options.focused_style.dim = false;
    options.focused_style.reverse = false;
    return options;
}

void apply_checkbox_style(CheckboxOptions& options, const CheckboxStyle& style, const Theme& theme) {
    options.glyphs = style.glyphs;
    merge_style(options.marker_style, style.marker_style);
    merge_style(options.marker_checked_style, style.marker_checked_style);
    merge_style(options.label_style, style.label_style);

    if (style_is_empty(options.marker_style)) {
        options.marker_style = theme.label;
    }
    if (style_is_empty(options.marker_checked_style)) {
        options.marker_checked_style = options.marker_style;
    }
    if (style_is_empty(options.label_style)) {
        options.label_style = theme.label;
    }
    if (style_is_empty(options.label_checked_style)) {
        options.label_checked_style = options.label_style;
    }
}

Checkbox::Checkbox(std::string label, bool checked, CheckboxOptions options, std::function<void(bool)> on_change)
    : label_(std::move(label)), checked_(checked), options_(std::move(options)), on_change_(std::move(on_change)) {
    if (style_is_empty(options_.marker_style)) {
        options_.marker_style = dark_theme().label;
    }
    if (style_is_empty(options_.marker_checked_style)) {
        options_.marker_checked_style = options_.marker_style;
    }
    if (style_is_empty(options_.label_style)) {
        options_.label_style = options_.marker_style;
    }
    if (style_is_empty(options_.label_checked_style)) {
        options_.label_checked_style = options_.label_style;
    }
    if (style_is_empty(options_.focused_style)) {
        options_.focused_style = options_.label_style;
        options_.focused_style.background_rgb = Rgb::hex(0x505050);
        options_.focused_style.foreground = Color::Default;
        options_.focused_style.bold = false;
        options_.focused_style.dim = false;
        options_.focused_style.reverse = false;
    }
}

Checkbox::Checkbox(std::string label, bool checked, std::string_view style_name, std::function<void(bool)> on_change,
                   const Theme& theme)
    : Checkbox(std::move(label), checked, checkbox_options_default(theme), std::move(on_change)) {
    if (const CheckboxStyle* style = checkbox_style_named(style_name)) {
        apply_checkbox_style(options_, *style, theme);
    }
}

void Checkbox::set_label(std::string label) {
    label_ = std::move(label);
    mark_dirty();
}

void Checkbox::set_checked(bool checked) {
    if (checked_ == checked) {
        return;
    }

    checked_ = checked;
    mark_dirty();
}

void Checkbox::set_options(CheckboxOptions options) {
    options_ = std::move(options);
    mark_dirty();
}

void Checkbox::set_glyphs(CheckboxGlyphs glyphs) {
    options_.glyphs = std::move(glyphs);
    mark_dirty();
}

Size Checkbox::preferred_size() const {
    const std::string& marker = checked_ ? options_.glyphs.checked : options_.glyphs.unchecked;
    return {
        text_display_width(marker) + text_display_width(options_.glyphs.gap) + text_display_width(label_),
        1,
    };
}

void Checkbox::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    Style clear_style;
    canvas.fill_rect({0, 0, bounds_.width, bounds_.height}, ' ', clear_style);

    const std::string& marker = checked_ ? options_.glyphs.checked : options_.glyphs.unchecked;

    const StyleResolver& styles = ctx.styles();
    Style marker_style = checked_ ? options_.marker_checked_style : options_.marker_style;
    marker_style = styles.text(*this, marker_style);
    Style label_style =
        checked_ && !style_is_empty(options_.label_checked_style) ? options_.label_checked_style : options_.label_style;
    label_style = styles.text(*this, label_style);

    if (is_focused() && options_.highlight_row_on_focus) {
        const Style row = styles.focused(*this, options_.focused_style);
        const int text_width =
            text_display_width(marker) + text_display_width(options_.glyphs.gap) + text_display_width(label_);
        const int highlight_width = std::min(bounds_.width, text_width);
        canvas.fill_rect({0, 0, highlight_width, 1}, ' ', row);
        marker_style.background = row.background;
        marker_style.background_rgb = row.background_rgb;
        label_style.background = row.background;
        label_style.background_rgb = row.background_rgb;
    }

    int column = 0;
    canvas.draw_text({column, 0}, marker, marker_style);
    column += text_display_width(marker);
    if (!options_.glyphs.gap.empty()) {
        canvas.draw_text({column, 0}, options_.glyphs.gap, marker_style);
        column += text_display_width(options_.glyphs.gap);
    }
    canvas.draw_text({column, 0}, label_, label_style);
}

bool Checkbox::handle_event(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (mouse->action != MouseAction::Click && mouse->action != MouseAction::Release) {
            return false;
        }

        if (!contains_point(mouse->position)) {
            return false;
        }

        toggle();
        return true;
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key || !is_focused()) {
        return false;
    }

    if (key->key == Key::Enter || key->character == ' ' || key->character == '\r') {
        toggle();
        return true;
    }

    return false;
}

void Checkbox::toggle() {
    checked_ = !checked_;
    if (on_change_) {
        on_change_(checked_);
    }
    mark_dirty();
}

} // namespace tuinator
