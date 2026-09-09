#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/render/style_resolver.hpp>
#include <tuinator/widgets/widget.hpp>

#include <initializer_list>
#include <string>
#include <vector>

namespace tuinator {

enum class ProgressBarLayout {
    Plain,
    Bracketed,
    Blocks,
    Dots,
    Pill,
    Labeled,
    FilledLabel,
    Tqdm,
    BrailleMetric,
    BrailleWave,
    Pulse,
    Shimmer,
    Bounce,
    SlideBlock,
    MovingDot,
    TaskRow,
};

enum class ProgressBarPercentPosition {
    None,
    Right,
    Inside,
};

enum class ProgressBarMode {
    Determinate,
    Indeterminate,
};

struct ProgressBarGradientStop {
    float position = 0.0f;
    Rgb color{};
};

struct ProgressBarGlyphs {
    std::string fill = "=";
    std::string head;
    std::string empty = " ";
    std::string left_cap;
    std::string right_cap;
};

struct ProgressBarStats {
    int current = 0;
    int total = 100;
    std::string unit = "step";
    double rate = 0.0;
    int elapsed_ms = 0;
};

struct ProgressBarOptions {
    ProgressBarLayout layout = ProgressBarLayout::Plain;
    ProgressBarGlyphs glyphs{};
    std::string label;
    std::string inside_label;
    std::string metric_value;
    ProgressBarStats stats{};
    bool show_percent = false;
    ProgressBarPercentPosition percent_position = ProgressBarPercentPosition::None;
    bool segmented = true;
    int min_width = 20;
    int animation_phase = 0;
    int pulse_width = 4;
    bool indeterminate = false;
    bool completed = false;
    std::string eta_text;
    std::vector<ProgressBarGradientStop> gradient_stops;
    Style fill_style{};
    Style track_style{};
    Style label_style{};
    Style percent_style{};
    Style value_style{};
    Style head_style{};
    Style inside_label_style{};
    Style eta_style{};
};

ProgressBarOptions progress_bar_preset(ProgressBarLayout layout, const Style& fill, const Style& track);
ProgressBarOptions progress_bar_filled_label(const Style& fill, const Style& track, std::string inside_label,
                                             ProgressBarMode mode = ProgressBarMode::Determinate);
std::vector<ProgressBarGradientStop>
progress_bar_gradient(std::initializer_list<std::pair<float, std::uint32_t>> hex_stops);
ProgressBarGlyphs progress_bar_glyphs_for(ProgressBarLayout layout);

class ProgressBar : public Widget {
  public:
    ProgressBar(double value = 0.0, ProgressBarOptions options = {});

    ProgressBar(double value, Style fill_style, Style track_style, int min_width = 20);

    double value() const { return value_; }
    const ProgressBarOptions& options() const { return options_; }

    void set_value(double value);
    void set_options(ProgressBarOptions options);
    void set_layout(ProgressBarLayout layout);
    void set_label(std::string label);
    void set_inside_label(std::string label);
    void set_metric_value(std::string value);
    void set_stats(ProgressBarStats stats);
    void set_show_percent(bool show);
    void set_animation_phase(int phase);
    void set_indeterminate(bool indeterminate);
    void set_completed(bool completed);

    std::string_view widget_type_name() const override { return "ProgressBar"; }
    void apply_stylesheet(const StyleResolver& styles) override;

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;

  private:
    void prepare_paint_options(const PaintContext& ctx) const;
    int bar_column_count() const;
    int rendered_bar_width(int bar_width) const;
    void paint_glyph_bar(Canvas& canvas) const;
    void paint_filled_label(Canvas& canvas) const;
    void paint_tqdm(Canvas& canvas) const;
    void paint_braille_metric(Canvas& canvas) const;
    void paint_braille_wave(Canvas& canvas) const;
    void paint_pulse(Canvas& canvas) const;
    void paint_shimmer(Canvas& canvas) const;
    void paint_bounce(Canvas& canvas) const;
    void paint_slide_block(Canvas& canvas) const;
    void paint_moving_dot(Canvas& canvas) const;
    void paint_task_row(Canvas& canvas) const;
    void paint_shimmer_bar(Canvas& canvas, int x, int y, int bar_width, const Style* fill_override = nullptr) const;
    void paint_bar_at(Canvas& canvas, int x, int y, int bar_width, const Style* fill_override = nullptr) const;
    void fill_rect_gradient(Canvas& canvas, int x, int y, int width, int height, char ch, const Style& base,
                            int bar_width) const;
    Style fill_style_at(const Style& base, int column, int bar_width) const;

    double value_;
    ProgressBarOptions options_;
    mutable ProgressBarOptions paint_options_;
};

} // namespace tuinator
