#include <tuinator/render/stylesheet.hpp>
#include <tuinator/render/widget_options.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/widget.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace tuinator {

namespace stylesheet_detail {

enum class SelectorKind {
    Universal,
    Type,
    Id,
    Class,
};

enum class PseudoState {
    None,
    Focused,
    Selected,
    Checked,
    Disabled,
    Open,
    Hover,
};

struct SelectorToken {
    SelectorKind kind = SelectorKind::Universal;
    std::string value;
    PseudoState pseudo = PseudoState::None;
};

struct Rule {
    std::vector<SelectorToken> selector;
    StylePatch text;
    StylePatch border;
    StylePatch title;
    StylePatch divider;
    StylePatch focused;
    StylePatch selected;
    WidgetOptions options;
    std::optional<GlyphSet> glyph_set;
    std::optional<BorderStyle> border_style;
};

struct ResolvedRule {
    StylePatch text;
    StylePatch border;
    StylePatch title;
    StylePatch divider;
    StylePatch focused;
    StylePatch selected;
    WidgetOptions options;
    std::optional<GlyphSet> glyph_set;
    std::optional<BorderStyle> border_style;
};

std::string trim(std::string_view text) {
    std::size_t start = 0;
    while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) {
        ++start;
    }

    std::size_t end = text.size();
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
        --end;
    }

    return std::string(text.substr(start, end - start));
}

bool starts_with(std::string_view text, std::string_view prefix) {
    return text.size() >= prefix.size() && text.substr(0, prefix.size()) == prefix;
}

std::string read_file(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("Failed to open stylesheet: " + path.string());
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

const std::unordered_map<std::string, Color>& color_names() {
    static const std::unordered_map<std::string, Color> names = {
        {"default", Color::Default}, {"black", Color::Black},   {"red", Color::Red},
        {"green", Color::Green},     {"yellow", Color::Yellow}, {"blue", Color::Blue},
        {"magenta", Color::Magenta}, {"cyan", Color::Cyan},     {"white", Color::White},
    };
    return names;
}

std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

std::optional<Color> parse_named_color(std::string_view name) {
    const auto it = color_names().find(lowercase(std::string(name)));
    if (it == color_names().end()) {
        return std::nullopt;
    }
    return it->second;
}

std::optional<Rgb> parse_hex_color(std::string_view value) {
    if (!starts_with(value, "#")) {
        return std::nullopt;
    }

    value.remove_prefix(1);
    if (value.size() != 3 && value.size() != 6) {
        return std::nullopt;
    }

    auto hex_digit = [](char ch) -> int {
        if (ch >= '0' && ch <= '9') {
            return ch - '0';
        }
        if (ch >= 'a' && ch <= 'f') {
            return 10 + ch - 'a';
        }
        if (ch >= 'A' && ch <= 'F') {
            return 10 + ch - 'A';
        }
        return -1;
    };

    if (value.size() == 3) {
        const int r = hex_digit(value[0]);
        const int g = hex_digit(value[1]);
        const int b = hex_digit(value[2]);
        if (r < 0 || g < 0 || b < 0) {
            return std::nullopt;
        }
        return Rgb{static_cast<std::uint8_t>(r * 17), static_cast<std::uint8_t>(g * 17),
                   static_cast<std::uint8_t>(b * 17)};
    }

    auto read_byte = [&](std::size_t index) -> std::optional<std::uint8_t> {
        const int high = hex_digit(value[index]);
        const int low = hex_digit(value[index + 1]);
        if (high < 0 || low < 0) {
            return std::nullopt;
        }
        return static_cast<std::uint8_t>((high << 4) | low);
    };

    const std::optional<std::uint8_t> r = read_byte(0);
    const std::optional<std::uint8_t> g = read_byte(2);
    const std::optional<std::uint8_t> b = read_byte(4);
    if (!r || !g || !b) {
        return std::nullopt;
    }
    return Rgb{*r, *g, *b};
}

void merge_patch_into(StylePatch& target, const StylePatch& source) {
    if (source.foreground) {
        target.foreground = source.foreground;
    }
    if (source.background) {
        target.background = source.background;
    }
    if (source.foreground_rgb) {
        target.foreground_rgb = source.foreground_rgb;
    }
    if (source.background_rgb) {
        target.background_rgb = source.background_rgb;
    }
    if (source.bold) {
        target.bold = source.bold;
    }
    if (source.dim) {
        target.dim = source.dim;
    }
    if (source.reverse) {
        target.reverse = source.reverse;
    }
}

StylePatch parse_color_tokens(std::string_view value,
                              const std::unordered_map<std::string, StylePatch>* theme_tokens = nullptr) {
    StylePatch patch;
    std::string remaining = trim(value);

    while (!remaining.empty()) {
        std::size_t space = remaining.find(' ');
        const std::string token = trim(space == std::string::npos ? remaining : remaining.substr(0, space));
        if (space == std::string::npos) {
            remaining.clear();
        } else {
            remaining = trim(remaining.substr(space + 1));
        }

        const std::string lowered = lowercase(token);
        if (lowered == "bold") {
            patch.bold = true;
            continue;
        }
        if (lowered == "dim") {
            patch.dim = true;
            continue;
        }
        if (lowered == "reverse") {
            patch.reverse = true;
            continue;
        }

        if (!token.empty() && token.front() == '$') {
            if (theme_tokens == nullptr) {
                throw std::runtime_error("Unknown theme token: " + token);
            }
            const auto it = theme_tokens->find(lowercase(token.substr(1)));
            if (it == theme_tokens->end()) {
                throw std::runtime_error("Unknown theme token: " + token);
            }
            merge_patch_into(patch, it->second);
            continue;
        }

        if (const std::optional<Rgb> rgb = parse_hex_color(token)) {
            patch.foreground_rgb = rgb;
            continue;
        }

        if (const std::optional<Color> named = parse_named_color(token)) {
            patch.foreground = named;
            continue;
        }

        throw std::runtime_error("Unknown color token: " + token);
    }

    return patch;
}

std::optional<PseudoState> parse_pseudo_state(std::string_view name) {
    const std::string lowered = lowercase(trim(name));
    if (lowered == "focused" || lowered == "focus") {
        return PseudoState::Focused;
    }
    if (lowered == "selected") {
        return PseudoState::Selected;
    }
    if (lowered == "checked") {
        return PseudoState::Checked;
    }
    if (lowered == "disabled") {
        return PseudoState::Disabled;
    }
    if (lowered == "open") {
        return PseudoState::Open;
    }
    if (lowered == "hover") {
        return PseudoState::Hover;
    }
    return std::nullopt;
}

WidgetPseudoState to_widget_pseudo(PseudoState pseudo) {
    switch (pseudo) {
    case PseudoState::Focused: return WidgetPseudoState::Focused;
    case PseudoState::Selected: return WidgetPseudoState::Selected;
    case PseudoState::Checked: return WidgetPseudoState::Checked;
    case PseudoState::Disabled: return WidgetPseudoState::Disabled;
    case PseudoState::Open: return WidgetPseudoState::Open;
    case PseudoState::Hover: return WidgetPseudoState::Hover;
    case PseudoState::None: return WidgetPseudoState::Focused;
    }
    return WidgetPseudoState::Focused;
}

std::optional<GlyphSet> parse_glyph_set(std::string_view value) {
    const std::string lowered = lowercase(trim(value));
    if (lowered == "auto") {
        return GlyphSet::Auto;
    }
    if (lowered == "ascii") {
        return GlyphSet::Ascii;
    }
    if (lowered == "unicode") {
        return GlyphSet::Unicode;
    }
    if (lowered == "unicode-rounded" || lowered == "rounded") {
        return GlyphSet::UnicodeRounded;
    }
    return std::nullopt;
}

std::optional<BorderStyle> parse_border_style(std::string_view value) {
    const std::string lowered = lowercase(trim(value));
    if (lowered == "ascii") {
        return BorderStyle::Ascii;
    }
    if (lowered == "light") {
        return BorderStyle::Light;
    }
    if (lowered == "heavy") {
        return BorderStyle::Heavy;
    }
    if (lowered == "double") {
        return BorderStyle::Double;
    }
    if (lowered == "rounded") {
        return BorderStyle::Rounded;
    }
    return std::nullopt;
}

bool parse_bool(std::string_view value) {
    const std::string lowered = lowercase(trim(value));
    if (lowered == "true" || lowered == "yes" || lowered == "1") {
        return true;
    }
    if (lowered == "false" || lowered == "no" || lowered == "0") {
        return false;
    }
    throw std::runtime_error("Expected boolean value, got: " + std::string(value));
}

int selector_specificity(const SelectorToken& token) {
    int specificity = 0;
    switch (token.kind) {
    case SelectorKind::Universal: specificity = 0; break;
    case SelectorKind::Type: specificity = 1; break;
    case SelectorKind::Class: specificity = 10; break;
    case SelectorKind::Id: specificity = 100; break;
    }
    if (token.pseudo != PseudoState::None) {
        specificity += 20;
    }
    return specificity;
}

int rule_specificity(const Rule& rule) {
    int specificity = 0;
    for (const SelectorToken& token : rule.selector) {
        specificity += selector_specificity(token);
    }
    return specificity;
}

bool matches_token(const Widget& widget, const SelectorToken& token) {
    bool matches = false;
    switch (token.kind) {
    case SelectorKind::Universal: matches = true; break;
    case SelectorKind::Type: matches = widget.widget_type_name() == token.value; break;
    case SelectorKind::Id: matches = widget.widget_id() == token.value; break;
    case SelectorKind::Class: matches = widget.has_widget_class(token.value); break;
    default: matches = false; break;
    }

    if (!matches) {
        return false;
    }

    if (token.pseudo == PseudoState::None) {
        return true;
    }

    return widget_matches_pseudo(widget, to_widget_pseudo(token.pseudo));
}

bool matches_selector_chain(const Widget& widget, const std::vector<SelectorToken>& tokens, std::size_t index) {
    if (!matches_token(widget, tokens[index])) {
        return false;
    }
    if (index == 0) {
        return true;
    }

    for (const Widget* parent = widget.parent_widget(); parent != nullptr; parent = parent->parent_widget()) {
        if (matches_selector_chain(*parent, tokens, index - 1)) {
            return true;
        }
    }

    return false;
}

bool patch_has_color(const StylePatch& patch) {
    return patch.foreground || patch.foreground_rgb || patch.background || patch.background_rgb || patch.bold ||
           patch.dim || patch.reverse;
}

GlyphSet glyph_set_from_theme(const Theme& theme) {
    const BorderGlyphs ascii = ascii_border_glyphs();
    if (theme.glyphs.horizontal == ascii.horizontal && theme.glyphs.vertical == ascii.vertical) {
        return GlyphSet::Ascii;
    }
    return GlyphSet::Unicode;
}

struct ParsedSheet {
    ThemeOptions theme_options{};
    std::unordered_map<std::string, StylePatch> theme_tokens;
    std::vector<Rule> rules;
};

class StylesheetParser {
  public:
    explicit StylesheetParser(std::string_view text) : text_(text) {}

    ParsedSheet parse_sheet() {
        ParsedSheet sheet;
        sheet_ = &sheet;
        while (!eof()) {
            skip_ws_and_comments();
            if (eof()) {
                break;
            }

            if (peek() == '@') {
                parse_at_rule(sheet);
                continue;
            }

            sheet.rules.push_back(parse_rule());
        }
        sheet_ = nullptr;
        return sheet;
    }

  private:
    std::string_view text_;
    std::size_t pos_ = 0;
    ParsedSheet* sheet_ = nullptr;

    bool eof() const { return pos_ >= text_.size(); }

    char peek() const { return eof() ? '\0' : text_[pos_]; }

    char consume() {
        if (eof()) {
            throw std::runtime_error("Unexpected end of stylesheet");
        }
        return text_[pos_++];
    }

    void skip_ws_and_comments() {
        while (!eof()) {
            if (std::isspace(static_cast<unsigned char>(peek()))) {
                ++pos_;
                continue;
            }

            if (peek() == '#') {
                const char next = pos_ + 1 < text_.size() ? text_[pos_ + 1] : '\0';
                if (next == '\0' || std::isspace(static_cast<unsigned char>(next))) {
                    while (!eof() && peek() != '\n') {
                        ++pos_;
                    }
                    continue;
                }
            }

            break;
        }
    }

    void expect(char ch) {
        skip_ws_and_comments();
        if (consume() != ch) {
            throw std::runtime_error(std::string("Expected '") + ch + "'");
        }
    }

    std::string read_until(char stop) {
        std::string value;
        while (!eof() && peek() != stop) {
            value.push_back(consume());
        }
        return trim(value);
    }

    SelectorToken parse_selector_token(std::string_view token) {
        SelectorToken parsed;
        std::string base_storage;
        std::string_view base = token;
        const std::size_t colon = token.find(':');
        if (colon != std::string_view::npos) {
            base_storage = trim(token.substr(0, colon));
            base = base_storage;
            if (base.empty()) {
                throw std::runtime_error("Invalid selector token: " + std::string(token));
            }
            const std::optional<PseudoState> pseudo = parse_pseudo_state(token.substr(colon + 1));
            if (!pseudo) {
                throw std::runtime_error("Unknown pseudo-class in selector: " + std::string(token));
            }
            parsed.pseudo = *pseudo;
        }

        if (base == "*") {
            parsed.kind = SelectorKind::Universal;
            return parsed;
        }

        if (!base.empty() && base.front() == '#') {
            parsed.kind = SelectorKind::Id;
            parsed.value = std::string(base.substr(1));
            return parsed;
        }

        if (!base.empty() && base.front() == '.') {
            parsed.kind = SelectorKind::Class;
            parsed.value = std::string(base.substr(1));
            return parsed;
        }

        parsed.kind = SelectorKind::Type;
        parsed.value = std::string(base);
        return parsed;
    }

    Rule parse_rule() {
        const std::string selector_text = read_until('{');
        expect('{');

        Rule rule;
        std::istringstream selector_stream(selector_text);
        std::string token;
        while (selector_stream >> token) {
            rule.selector.push_back(parse_selector_token(token));
        }

        if (rule.selector.empty()) {
            throw std::runtime_error("Empty selector");
        }

        while (!eof()) {
            skip_ws_and_comments();
            if (peek() == '}') {
                consume();
                break;
            }

            const std::string property = read_until(':');
            expect(':');
            const std::string value = read_until(';');
            expect(';');
            apply_property(rule, property, value);
        }

        return rule;
    }

    const std::unordered_map<std::string, StylePatch>* theme_tokens() const {
        return sheet_ != nullptr ? &sheet_->theme_tokens : nullptr;
    }

    void apply_property(Rule& rule, std::string_view property, std::string_view value) {
        const std::string name = lowercase(trim(property));
        const std::string trimmed_value = trim(value);

        if (name == "color" || name == "foreground") {
            rule.text = parse_color_tokens(trimmed_value, theme_tokens());
            return;
        }
        if (name == "background" || name == "bg") {
            StylePatch patch = parse_color_tokens(trimmed_value, theme_tokens());
            rule.text.background = patch.foreground;
            rule.text.background_rgb = patch.foreground_rgb;
            return;
        }
        if (name == "bold") {
            rule.text.bold = parse_bool(trimmed_value);
            return;
        }
        if (name == "dim") {
            rule.text.dim = parse_bool(trimmed_value);
            return;
        }
        if (name == "reverse") {
            rule.text.reverse = parse_bool(trimmed_value);
            return;
        }
        if (name == "border-color") {
            rule.border = parse_color_tokens(trimmed_value, theme_tokens());
            return;
        }
        if (name == "title-color") {
            rule.title = parse_color_tokens(trimmed_value, theme_tokens());
            return;
        }
        if (name == "divider-color") {
            rule.divider = parse_color_tokens(trimmed_value, theme_tokens());
            return;
        }
        if (name == "focused-color") {
            rule.focused = parse_color_tokens(trimmed_value, theme_tokens());
            return;
        }
        if (name == "focused-background" || name == "focused-bg") {
            StylePatch patch = parse_color_tokens(trimmed_value, theme_tokens());
            rule.focused.background = patch.foreground;
            rule.focused.background_rgb = patch.foreground_rgb;
            return;
        }
        if (name == "selected-color") {
            rule.selected = parse_color_tokens(trimmed_value, theme_tokens());
            return;
        }
        if (name == "selected-background" || name == "selected-bg") {
            StylePatch patch = parse_color_tokens(trimmed_value, theme_tokens());
            rule.selected.background = patch.foreground;
            rule.selected.background_rgb = patch.foreground_rgb;
            return;
        }
        if (name == "glyph-set" || name == "glyph_set") {
            if (const std::optional<GlyphSet> glyphs = parse_glyph_set(trimmed_value)) {
                rule.glyph_set = glyphs;
            } else {
                throw std::runtime_error("Unknown glyph-set: " + trimmed_value);
            }
            return;
        }
        if (name == "border-style" || name == "border_style") {
            if (const std::optional<BorderStyle> style = parse_border_style(trimmed_value)) {
                rule.border_style = style;
            } else {
                throw std::runtime_error("Unknown border-style: " + trimmed_value);
            }
            return;
        }

        rule.options.set(name, trimmed_value);
    }

    void parse_at_rule(ParsedSheet& sheet) {
        expect('@');
        const std::string name = read_until('{');
        expect('{');

        if (lowercase(trim(name)) != "theme") {
            throw std::runtime_error("Unknown at-rule: @" + name);
        }

        while (!eof()) {
            skip_ws_and_comments();
            if (peek() == '}') {
                consume();
                break;
            }

            const std::string property = read_until(':');
            expect(':');
            const std::string value = read_until(';');
            expect(';');
            apply_theme_property(sheet, property, value);
        }
    }

    void apply_theme_property(ParsedSheet& sheet, std::string_view property, std::string_view value) {
        const std::string name = lowercase(trim(property));
        const std::string trimmed_value = trim(value);

        if (name == "glyph-set" || name == "glyph_set") {
            if (const std::optional<GlyphSet> glyphs = parse_glyph_set(trimmed_value)) {
                sheet.theme_options.glyphs = *glyphs;
            } else {
                throw std::runtime_error("Unknown glyph-set: " + trimmed_value);
            }
            return;
        }

        if (name == "border-style" || name == "border_style") {
            if (const std::optional<BorderStyle> style = parse_border_style(trimmed_value)) {
                sheet.theme_options.border_style = style;
            } else {
                throw std::runtime_error("Unknown border-style: " + trimmed_value);
            }
            return;
        }

        sheet.theme_tokens[name] = parse_color_tokens(trimmed_value, &sheet.theme_tokens);
    }
};

} // namespace stylesheet_detail

struct Stylesheet::Impl {
    ThemeOptions theme_options{};
    std::unordered_map<std::string, StylePatch> theme_tokens;
    std::vector<stylesheet_detail::Rule> rules;
};

void apply_style_patch(Style& style, const StylePatch& patch) {
    if (patch.foreground) {
        style.foreground = *patch.foreground;
        style.foreground_rgb.reset();
    }
    if (patch.background) {
        style.background = *patch.background;
        style.background_rgb.reset();
    }
    if (patch.foreground_rgb) {
        style.foreground_rgb = patch.foreground_rgb;
    }
    if (patch.background_rgb) {
        style.background_rgb = patch.background_rgb;
    }
    if (patch.bold) {
        style.bold = *patch.bold;
    }
    if (patch.dim) {
        style.dim = *patch.dim;
    }
    if (patch.reverse) {
        style.reverse = *patch.reverse;
    }
}

void merge_style_patch(StylePatch& target, const StylePatch& source) {
    if (source.foreground) {
        target.foreground = source.foreground;
    }
    if (source.background) {
        target.background = source.background;
    }
    if (source.foreground_rgb) {
        target.foreground_rgb = source.foreground_rgb;
    }
    if (source.background_rgb) {
        target.background_rgb = source.background_rgb;
    }
    if (source.bold) {
        target.bold = source.bold;
    }
    if (source.dim) {
        target.dim = source.dim;
    }
    if (source.reverse) {
        target.reverse = source.reverse;
    }
}

Stylesheet::Stylesheet() : impl_(std::make_unique<Impl>()) {}

Stylesheet::Stylesheet(const Stylesheet& other) : impl_(std::make_unique<Impl>(*other.impl_)) {}

Stylesheet::Stylesheet(Stylesheet&& other) noexcept : impl_(std::move(other.impl_)) {}

Stylesheet::~Stylesheet() = default;

Stylesheet& Stylesheet::operator=(const Stylesheet& other) {
    if (this != &other) {
        *impl_ = *other.impl_;
    }
    return *this;
}

Stylesheet& Stylesheet::operator=(Stylesheet&& other) noexcept {
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

ThemeOptions Stylesheet::theme_options() const { return impl_->theme_options; }

Stylesheet Stylesheet::load_from_string(std::string_view text) {
    Stylesheet sheet;
    stylesheet_detail::StylesheetParser parser(text);
    const stylesheet_detail::ParsedSheet parsed = parser.parse_sheet();
    sheet.impl_->theme_options = parsed.theme_options;
    sheet.impl_->theme_tokens = parsed.theme_tokens;
    sheet.impl_->rules = parsed.rules;
    return sheet;
}

Stylesheet Stylesheet::load_from_file(const std::filesystem::path& path) {
    return load_from_string(stylesheet_detail::read_file(path));
}

namespace {

stylesheet_detail::ResolvedRule resolve_rules(const std::vector<stylesheet_detail::Rule>& rules, const Widget& widget) {
    std::vector<const stylesheet_detail::Rule*> matches;
    for (const stylesheet_detail::Rule& rule : rules) {
        if (stylesheet_detail::matches_selector_chain(widget, rule.selector, rule.selector.size() - 1)) {
            matches.push_back(&rule);
        }
    }

    std::stable_sort(
        matches.begin(), matches.end(), [](const stylesheet_detail::Rule* left, const stylesheet_detail::Rule* right) {
            return stylesheet_detail::rule_specificity(*left) < stylesheet_detail::rule_specificity(*right);
        });

    stylesheet_detail::ResolvedRule resolved;
    for (const stylesheet_detail::Rule* rule : matches) {
        merge_style_patch(resolved.text, rule->text);
        merge_style_patch(resolved.border, rule->border);
        merge_style_patch(resolved.title, rule->title);
        merge_style_patch(resolved.divider, rule->divider);
        merge_style_patch(resolved.focused, rule->focused);
        merge_style_patch(resolved.selected, rule->selected);
        merge_widget_options(resolved.options, rule->options);
        if (rule->glyph_set) {
            resolved.glyph_set = rule->glyph_set;
        }
        if (rule->border_style) {
            resolved.border_style = rule->border_style;
        }
    }

    return resolved;
}

} // namespace

Style Stylesheet::resolve_text_style(const Widget& widget, Style fallback) const {
    Style style = fallback;
    apply_style_patch(style, resolve_rules(impl_->rules, widget).text);
    return style;
}

Style Stylesheet::resolve_border_style(const Widget& widget, Style fallback) const {
    const stylesheet_detail::ResolvedRule resolved = resolve_rules(impl_->rules, widget);
    Style style = fallback;
    if (stylesheet_detail::patch_has_color(resolved.border)) {
        apply_style_patch(style, resolved.border);
    } else {
        apply_style_patch(style, resolved.text);
    }
    return style;
}

Style Stylesheet::resolve_title_style(const Widget& widget, Style fallback) const {
    const stylesheet_detail::ResolvedRule resolved = resolve_rules(impl_->rules, widget);
    Style style = fallback;
    if (stylesheet_detail::patch_has_color(resolved.title)) {
        apply_style_patch(style, resolved.title);
    } else {
        apply_style_patch(style, resolved.text);
    }
    return style;
}

Style Stylesheet::resolve_divider_style(const Widget& widget, Style fallback) const {
    const stylesheet_detail::ResolvedRule resolved = resolve_rules(impl_->rules, widget);
    Style style = fallback;
    if (stylesheet_detail::patch_has_color(resolved.divider)) {
        apply_style_patch(style, resolved.divider);
    } else {
        apply_style_patch(style, resolved.text);
    }
    return style;
}

Style Stylesheet::resolve_focused_style(const Widget& widget, Style fallback) const {
    const stylesheet_detail::ResolvedRule resolved = resolve_rules(impl_->rules, widget);
    Style style = fallback;
    if (stylesheet_detail::patch_has_color(resolved.focused)) {
        apply_style_patch(style, resolved.focused);
    }
    return style;
}

Style Stylesheet::resolve_selected_style(const Widget& widget, Style fallback) const {
    const stylesheet_detail::ResolvedRule resolved = resolve_rules(impl_->rules, widget);
    Style style = fallback;
    if (stylesheet_detail::patch_has_color(resolved.selected)) {
        apply_style_patch(style, resolved.selected);
    }
    return style;
}

BorderGlyphs Stylesheet::resolve_border_glyphs(const Widget& widget, const Theme& theme) const {
    const stylesheet_detail::ResolvedRule resolved = resolve_rules(impl_->rules, widget);
    if (resolved.border_style) {
        return border_glyphs_for(*resolved.border_style);
    }

    GlyphSet glyphs = resolved.glyph_set ? *resolved.glyph_set : stylesheet_detail::glyph_set_from_theme(theme);
    if (glyphs == GlyphSet::Auto) {
        glyphs = detect_glyph_set();
    }
    return border_glyphs_for(glyphs);
}

GlyphSet Stylesheet::resolve_glyph_set(const Widget& widget, const Theme& theme) const {
    const stylesheet_detail::ResolvedRule resolved = resolve_rules(impl_->rules, widget);
    if (resolved.glyph_set) {
        return *resolved.glyph_set;
    }
    return stylesheet_detail::glyph_set_from_theme(theme);
}

WidgetOptions Stylesheet::resolve_options(const Widget& widget) const {
    return resolve_rules(impl_->rules, widget).options;
}

Style Stylesheet::resolve_theme_token(std::string_view name, Style fallback) const {
    const auto it = impl_->theme_tokens.find(stylesheet_detail::lowercase(std::string(name)));
    if (it == impl_->theme_tokens.end()) {
        return fallback;
    }

    Style style = fallback;
    apply_style_patch(style, it->second);
    return style;
}

} // namespace tuinator
