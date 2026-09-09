#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/widgets/widget.hpp>

#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace tuinator {

enum class BigTextKind {
    Block,
    Half,
    Narrow,
    Quadrant,
    Third,
    Sextant,
    Quarter,
    Octant,
    Braille,
    Shade,
    Letter,
    Banner,
    Outline,
    Stacked,
    Isometric,
    Slant,
    Standard,
    Small,
    Big,
    Doom,
    FigletShadow,
};

enum class BigTextGradientAxis {
    Vertical,
    Horizontal,
};

struct BigTextGradientStop {
    float position = 0.0f;
    Rgb color{};
};

struct BigTextLook {
    const char* id = "";
    const char* title = "";
    BigTextKind kind = BigTextKind::Block;
    int scale = 1;
    std::vector<BigTextGradientStop> gradient_stops;
    BigTextGradientAxis gradient_axis = BigTextGradientAxis::Vertical;
    std::optional<Style> shadow_style;
    int shadow_dx = 1;
    int shadow_dy = 1;
    int shadow_layers = 0;
    bool rainbow = false;
};

struct BigTextPreset {
    const char* id = "";
    const char* title = "";
    BigTextKind kind = BigTextKind::Block;
    std::vector<std::string> aliases;
};

struct BigTextOptions {
    BigTextKind kind = BigTextKind::Block;
    int letter_spacing = 1;
    int scale = 1;
    bool rainbow = false;
    std::vector<Style> palette;
    std::string underline;
    std::vector<BigTextGradientStop> gradient_stops;
    BigTextGradientAxis gradient_axis = BigTextGradientAxis::Vertical;
    /// Color of drop-shadow cells. Unset = dimmed copy of the glyph style.
    std::optional<Style> shadow_style;
    int shadow_dx = 1;
    int shadow_dy = 1;
    /// Stacked copies. 0 = kind default (2 for stacked, 1 for isometric).
    int shadow_layers = 0;
};

const std::vector<BigTextPreset>& all_big_text_styles();
const BigTextPreset* big_text_style_named(std::string_view name);
BigTextKind big_text_kind_named(std::string_view name, BigTextKind fallback = BigTextKind::Block);

std::vector<BigTextGradientStop> big_text_gradient(std::initializer_list<std::pair<float, std::uint32_t>> hex_stops);
void apply_big_text_look(BigTextOptions& options, const BigTextLook& look);
const std::vector<BigTextLook>& all_big_text_looks();
const BigTextLook* big_text_look_named(std::string_view name);

class BigText : public Widget {
  public:
    explicit BigText(std::string text, Style style = {});
    BigText(std::string text, BigTextKind kind, Style style = {});
    BigText(std::string text, const BigTextOptions& options, Style style = {});
    BigText(std::string text, std::string_view style_name, Style style = {});

    const std::string& text() const { return text_; }
    void set_text(std::string text);

    BigTextKind kind() const { return options_.kind; }
    void set_kind(BigTextKind kind);

    const BigTextOptions& options() const { return options_; }
    void set_options(BigTextOptions options);

    const Style& style() const { return style_; }
    void set_style(Style style);
    void set_shadow_style(Style style);

    std::string_view widget_type_name() const override { return "BigText"; }

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;

  private:
    void rebuild() const;

    std::string text_;
    BigTextOptions options_;
    Style style_;

    struct Cell {
        std::string glyph;
        int letter = 0;
        bool shadow = false;
    };

    mutable bool dirty_cache_ = true;
    mutable int cache_width_ = 0;
    mutable int cache_height_ = 0;
    mutable std::vector<Cell> cache_cells_;
};

} // namespace tuinator
