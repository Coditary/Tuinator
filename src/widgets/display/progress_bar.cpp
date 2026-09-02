#include <tuinator/widgets/display/progress_bar.hpp>

#include <tuinator/render/color.hpp>
#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/text.hpp>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <initializer_list>
#include <sstream>
#include <string>

namespace tuinator {

namespace {

ProgressBarGlyphs plain_glyphs() {
    return {"=", ">", " ", "", ""};
}

ProgressBarGlyphs bracket_glyphs() {
    ProgressBarGlyphs glyphs = plain_glyphs();
    glyphs.left_cap = "[";
    glyphs.right_cap = "]";
    glyphs.head.clear();
    return glyphs;
}

ProgressBarGlyphs pill_glyphs() {
    ProgressBarGlyphs glyphs = plain_glyphs();
    glyphs.left_cap = "(";
    glyphs.right_cap = ")";
    return glyphs;
}

ProgressBarGlyphs block_glyphs() {
    if (detect_glyph_set() == GlyphSet::Ascii) {
        return {"#", "", "-", "", ""};
    }

    return {
        "\xe2\x96\x88", // █
        "",
        "\xe2\x96\x91", // ░
        "",
        "",
    };
}

ProgressBarGlyphs dot_glyphs() {
    if (detect_glyph_set() == GlyphSet::Ascii) {
        return {"o", "", ".", "", ""};
    }

    return {
        "\xe2\x97\x8f", // ●
        "",
        "\xc2\xb7",     // ·
        "",
        "",
    };
}

ProgressBarGlyphs braille_glyphs() {
    if (detect_glyph_set() == GlyphSet::Ascii) {
        return {"|", "", ":", "", ""};
    }

    return {
        "\xe2\xa3\xbf", // ⣿
        "\xe2\xa3\xbf",
        "\xe2\xa0\x80", // ⠀
        "",
        "",
    };
}

ProgressBarGlyphs tqdm_glyphs() {
    ProgressBarGlyphs glyphs{};
    if (detect_glyph_set() == GlyphSet::Ascii) {
        glyphs.fill = "#";
        glyphs.empty = "-";
    } else {
        glyphs.fill = "\xe2\x96\x88"; // █
        glyphs.empty = "\xe2\x96\x91"; // ░
    }
    return glyphs;
}

std::string percent_text(double value) {
    const int percent = std::clamp(static_cast<int>(std::lround(value * 100.0)), 0, 100);
    return std::to_string(percent) + "%";
}

std::string format_clock(int ms) {
    const int total_seconds = std::max(0, ms) / 1000;
    const int minutes = total_seconds / 60;
    const int seconds = total_seconds % 60;
    std::ostringstream out;
    out << std::setw(2) << std::setfill('0') << minutes << ':'
        << std::setw(2) << std::setfill('0') << seconds;
    return out.str();
}

std::string format_eta_hms(int ms, bool unknown) {
    if (unknown) {
        return "-:--:--";
    }

    const int total_seconds = std::max(0, ms) / 1000;
    const int hours = total_seconds / 3600;
    const int minutes = (total_seconds % 3600) / 60;
    const int seconds = total_seconds % 60;
    std::ostringstream out;
    out << hours << ':'
        << std::setw(2) << std::setfill('0') << minutes << ':'
        << std::setw(2) << std::setfill('0') << seconds;
    return out.str();
}

int bounce_position(int phase, int travel) {
    if (travel <= 0) {
        return 0;
    }

    const int period = travel * 2;
    int t = phase % period;
    if (t < 0) {
        t += period;
    }
    return t <= travel ? t : period - t;
}

int normalized_phase(int phase) {
    const int period = 600;
    int t = phase % period;
    if (t < 0) {
        t += period;
    }
    return t;
}

Style text_only_style(Style style) {
    style.background = Color::Default;
    return style;
}

Rgb lerp_rgb(Rgb from, Rgb to, float amount) {
    const auto mix = [](int a, int b, float t) {
        return static_cast<std::uint8_t>(static_cast<float>(a) + (static_cast<float>(b - a) * t));
    };

    return {mix(from.r, to.r, amount), mix(from.g, to.g, amount), mix(from.b, to.b, amount)};
}

Rgb sample_gradient(const std::vector<ProgressBarGradientStop>& stops, float t) {
    if (stops.empty()) {
        return {};
    }
    if (stops.size() == 1) {
        return stops.front().color;
    }

    const float clamped = std::clamp(t, 0.0f, 1.0f);
    if (clamped <= stops.front().position) {
        return stops.front().color;
    }
    if (clamped >= stops.back().position) {
        return stops.back().color;
    }

    for (std::size_t index = 1; index < stops.size(); ++index) {
        const ProgressBarGradientStop& right = stops[index];
        const ProgressBarGradientStop& left = stops[index - 1];
        if (clamped > right.position) {
            continue;
        }

        const float span = right.position - left.position;
        const float local = span > 0.0f ? (clamped - left.position) / span : 0.0f;
        return lerp_rgb(left.color, right.color, local);
    }

    return stops.back().color;
}

Style apply_gradient_color(const Style& base, Rgb rgb) {
    if (base.background != Color::Default || base.background_rgb.has_value()) {
        return ColorValue::from_rgb(rgb).background_style(base);
    }
    return ColorValue::from_rgb(rgb).foreground_style(base);
}

void draw_label_on_fill(
    Canvas& canvas,
    int x,
    int y,
    std::string_view text,
    int fill_start,
    int fill_end,
    const Style& base_style,
    const Style& fill_style,
    const Style& track_style) {
    int cursor = x;
    for (std::size_t index = 0; index < text.size();) {
        const std::size_t byte_length = text_byte_length_for_width(text.substr(index), 1);
        if (byte_length == 0) {
            break;
        }

        const std::string_view glyph(text.data() + index, byte_length);
        const bool on_fill = cursor >= fill_start && cursor < fill_end;
        Style style = base_style;
        style.foreground = on_fill ? fill_style.foreground : track_style.foreground;
        style.background = on_fill ? fill_style.background : track_style.background;
        if (style.background == Color::Default) {
            style.background = on_fill ? fill_style.background : track_style.background;
        }
        if (style.foreground == Color::Default) {
            style.foreground = on_fill ? Color::Black : Color::White;
            if (!on_fill) {
                style.dim = true;
            }
        }

        canvas.draw_text({cursor, y}, glyph, style);
        cursor += text_display_width(glyph);
        index += byte_length;
    }
}

} // namespace

ProgressBarGlyphs progress_bar_glyphs_for(ProgressBarLayout layout) {
    switch (layout) {
    case ProgressBarLayout::Plain:
        return plain_glyphs();
    case ProgressBarLayout::Bracketed:
    case ProgressBarLayout::Labeled:
        return bracket_glyphs();
    case ProgressBarLayout::Blocks:
    case ProgressBarLayout::FilledLabel:
        return block_glyphs();
    case ProgressBarLayout::Dots:
        return dot_glyphs();
    case ProgressBarLayout::Pill:
        return pill_glyphs();
    case ProgressBarLayout::Tqdm:
        return tqdm_glyphs();
    case ProgressBarLayout::BrailleMetric:
    case ProgressBarLayout::BrailleWave:
        return braille_glyphs();
    case ProgressBarLayout::Pulse:
    case ProgressBarLayout::Shimmer:
    case ProgressBarLayout::TaskRow:
        return tqdm_glyphs();
    case ProgressBarLayout::Bounce:
    case ProgressBarLayout::SlideBlock:
    case ProgressBarLayout::MovingDot:
        return plain_glyphs();
    }

    return plain_glyphs();
}

ProgressBarOptions progress_bar_preset(ProgressBarLayout layout, const Style& fill, const Style& track) {
    ProgressBarOptions options{};
    options.layout = layout;
    options.glyphs = progress_bar_glyphs_for(layout);
    options.fill_style = fill;
    options.track_style = track;
    options.label_style = text_only_style(fill);
    options.percent_style = text_only_style(track);
    options.value_style = text_only_style(fill);
    options.head_style = text_only_style(fill);
    options.head_style.foreground = Color::Red;
    options.inside_label_style.foreground = Color::Black;
    options.inside_label_style.background = Color::Default;
    options.eta_style = text_only_style(track);

    switch (layout) {
    case ProgressBarLayout::Plain:
        options.min_width = 24;
        break;
    case ProgressBarLayout::Bracketed:
        options.min_width = 22;
        options.show_percent = true;
        options.percent_position = ProgressBarPercentPosition::Right;
        break;
    case ProgressBarLayout::Blocks:
    case ProgressBarLayout::Dots:
        options.min_width = 20;
        options.show_percent = true;
        options.percent_position = ProgressBarPercentPosition::Right;
        break;
    case ProgressBarLayout::Pill:
        options.min_width = 22;
        break;
    case ProgressBarLayout::Labeled:
        options.label = "Progress";
        options.min_width = 18;
        options.show_percent = true;
        options.percent_position = ProgressBarPercentPosition::Right;
        break;
    case ProgressBarLayout::FilledLabel:
        options.min_width = 28;
        options.inside_label = "Progress";
        options.fill_style.background = Color::Yellow;
        options.fill_style.foreground = Color::Black;
        options.track_style.background = Color::Default;
        options.track_style.foreground = Color::White;
        options.track_style.dim = true;
        options.inside_label_style.foreground = Color::Black;
        break;
    case ProgressBarLayout::Tqdm:
        options.label = "Processing";
        options.min_width = 48;
        options.stats.total = 100;
        break;
    case ProgressBarLayout::BrailleMetric:
        options.min_width = 32;
        options.segmented = true;
        options.head_style.foreground = Color::Red;
        options.head_style.bold = true;
        break;
    case ProgressBarLayout::BrailleWave:
        options.label = "Preparing...";
        options.min_width = 32;
        break;
    case ProgressBarLayout::Pulse:
        options.min_width = 40;
        options.indeterminate = true;
        options.pulse_width = 4;
        options.track_style.foreground = Color::Red;
        options.track_style.dim = true;
        options.fill_style.foreground = Color::Red;
        options.fill_style.bold = true;
        options.percent_style.foreground = Color::White;
        options.percent_style.dim = true;
        break;
    case ProgressBarLayout::Shimmer:
        options.label = "Loading...";
        options.min_width = 36;
        options.indeterminate = true;
        options.pulse_width = 5;
        options.fill_style.foreground = Color::Magenta;
        options.fill_style.bold = true;
        options.track_style.dim = true;
        break;
    case ProgressBarLayout::Bounce:
        options.min_width = 14;
        options.indeterminate = true;
        options.pulse_width = 3;
        options.fill_style.bold = true;
        break;
    case ProgressBarLayout::SlideBlock:
        options.min_width = 24;
        options.indeterminate = true;
        break;
    case ProgressBarLayout::MovingDot:
        options.min_width = 16;
        options.indeterminate = true;
        break;
    case ProgressBarLayout::TaskRow:
        options.label = "Processing...";
        options.min_width = 56;
        options.show_percent = true;
        options.percent_position = ProgressBarPercentPosition::Right;
        options.fill_style.foreground = Color::Magenta;
        options.label_style.foreground = Color::Green;
        options.percent_style.foreground = Color::Magenta;
        options.eta_style.foreground = Color::Cyan;
        break;
    }

    return options;
}

ProgressBarOptions progress_bar_filled_label(
    const Style& fill,
    const Style& track,
    std::string inside_label,
    ProgressBarMode mode) {
    ProgressBarOptions options = progress_bar_preset(ProgressBarLayout::FilledLabel, fill, track);
    options.inside_label = std::move(inside_label);
    options.inside_label_style.background = Color::Default;
    options.indeterminate = mode == ProgressBarMode::Indeterminate;
    if (options.indeterminate) {
        options.pulse_width = 8;
    }
    return options;
}

std::vector<ProgressBarGradientStop> progress_bar_gradient(
    std::initializer_list<std::pair<float, std::uint32_t>> hex_stops) {
    std::vector<ProgressBarGradientStop> stops;
    stops.reserve(hex_stops.size());
    for (const auto& [position, hex] : hex_stops) {
        stops.push_back({position, Rgb::hex(hex)});
    }
    return stops;
}

ProgressBar::ProgressBar(double value, ProgressBarOptions options)
    : value_(std::clamp(value, 0.0, 1.0)), options_(std::move(options)) {
    if (options_.glyphs.fill.empty()) {
        options_.glyphs = progress_bar_glyphs_for(options_.layout);
    }
}

ProgressBar::ProgressBar(double value, Style fill_style, Style track_style, int min_width)
    : ProgressBar(value, progress_bar_preset(ProgressBarLayout::Plain, fill_style, track_style)) {
    options_.min_width = min_width;
}

void ProgressBar::set_value(double value) {
    value_ = std::clamp(value, 0.0, 1.0);
    mark_dirty();
}

void ProgressBar::set_options(ProgressBarOptions options) {
    options_ = std::move(options);
    if (options_.glyphs.fill.empty()) {
        options_.glyphs = progress_bar_glyphs_for(options_.layout);
    }
    mark_dirty();
}

void ProgressBar::set_layout(ProgressBarLayout layout) {
    options_.layout = layout;
    options_.glyphs = progress_bar_glyphs_for(layout);
    mark_dirty();
}

void ProgressBar::set_label(std::string label) {
    options_.label = std::move(label);
    mark_dirty();
}

void ProgressBar::set_inside_label(std::string label) {
    options_.inside_label = std::move(label);
    mark_dirty();
}

void ProgressBar::set_metric_value(std::string value) {
    options_.metric_value = std::move(value);
    mark_dirty();
}

void ProgressBar::set_stats(ProgressBarStats stats) {
    options_.stats = std::move(stats);
    mark_dirty();
}

void ProgressBar::set_show_percent(bool show) {
    options_.show_percent = show;
    mark_dirty();
}

void ProgressBar::set_animation_phase(int phase) {
    options_.animation_phase = phase;
    mark_dirty();
}

void ProgressBar::set_indeterminate(bool indeterminate) {
    options_.indeterminate = indeterminate;
    mark_dirty();
}

void ProgressBar::set_completed(bool completed) {
    options_.completed = completed;
    mark_dirty();
}

int ProgressBar::bar_column_count() const {
    int reserved = 0;

    if (!options_.label.empty()
        && options_.layout != ProgressBarLayout::BrailleMetric
        && options_.layout != ProgressBarLayout::BrailleWave
        && options_.layout != ProgressBarLayout::Tqdm
        && options_.layout != ProgressBarLayout::FilledLabel
        && options_.layout != ProgressBarLayout::Pulse
        && options_.layout != ProgressBarLayout::Shimmer
        && options_.layout != ProgressBarLayout::Bounce
        && options_.layout != ProgressBarLayout::SlideBlock
        && options_.layout != ProgressBarLayout::MovingDot
        && options_.layout != ProgressBarLayout::TaskRow) {
        reserved += text_display_width(options_.label) + 1;
    }

    reserved += text_display_width(options_.glyphs.left_cap);
    reserved += text_display_width(options_.glyphs.right_cap);

    if (options_.show_percent && options_.percent_position == ProgressBarPercentPosition::Right) {
        reserved += text_display_width(percent_text(value_)) + 1;
    }

    return std::max(4, options_.min_width - reserved);
}

int ProgressBar::rendered_bar_width(int bar_width) const {
    const int fill_width = std::max(1, text_display_width(options_.glyphs.fill));
    const int empty_width = std::max(1, text_display_width(options_.glyphs.empty));
    const int head_width = options_.glyphs.head.empty() ? 0 : text_display_width(options_.glyphs.head);

    const int filled_columns =
        std::clamp(static_cast<int>(value_ * bar_width + 0.5), 0, bar_width);
    const bool use_head = head_width > 0 && filled_columns > 0 && filled_columns < bar_width;
    const int solid_columns = use_head ? filled_columns - 1 : filled_columns;
    const int empty_columns = bar_width - filled_columns;

    return text_display_width(options_.glyphs.left_cap)
        + solid_columns * fill_width
        + (use_head ? head_width : 0)
        + empty_columns * empty_width
        + text_display_width(options_.glyphs.right_cap);
}

Size ProgressBar::preferred_size() const {
    switch (options_.layout) {
    case ProgressBarLayout::BrailleMetric:
    case ProgressBarLayout::BrailleWave:
        return {options_.min_width, 2};
    default:
        break;
    }

    int width = options_.min_width;
    if (!options_.label.empty()) {
        width = std::max(width, text_display_width(options_.label) + 1 + bar_column_count() + 6);
    }

    return {width, 1};
}

Style ProgressBar::fill_style_at(const Style& base, int column, int bar_width) const {
    if (options_.gradient_stops.size() < 2 || bar_width <= 1) {
        return base;
    }

    const float t = static_cast<float>(column) / static_cast<float>(bar_width - 1);
    return apply_gradient_color(base, sample_gradient(options_.gradient_stops, t));
}

void ProgressBar::fill_rect_gradient(
    Canvas& canvas,
    int x,
    int y,
    int width,
    int height,
    char ch,
    const Style& base,
    int bar_width) const {
    if (width <= 0 || height <= 0) {
        return;
    }

    const std::string cell(1, ch);
    for (int row = 0; row < height; ++row) {
        for (int column = 0; column < width; ++column) {
            canvas.draw_text({x + column, y + row}, cell, fill_style_at(base, x + column, bar_width));
        }
    }
}

void ProgressBar::paint_bar_at(Canvas& canvas, int x, int y, int bar_width, const Style* fill_override) const {
    if (bar_width <= 0) {
        return;
    }

    const Style& fill_style = fill_override != nullptr ? *fill_override : options_.fill_style;

    const int fill_width = std::max(1, text_display_width(options_.glyphs.fill));
    const int empty_width = std::max(1, text_display_width(options_.glyphs.empty));
    const int head_width = options_.glyphs.head.empty() ? 0 : text_display_width(options_.glyphs.head);

    int cursor = x;

    if (!options_.glyphs.left_cap.empty()) {
        canvas.draw_text({cursor, y}, options_.glyphs.left_cap, fill_style);
        cursor += text_display_width(options_.glyphs.left_cap);
    }

    const int filled_columns =
        std::clamp(static_cast<int>(value_ * bar_width + 0.5), 0, bar_width);
    const bool use_head =
        head_width > 0 && filled_columns > 0 && filled_columns < bar_width;
    const int solid_columns = use_head ? filled_columns - 1 : filled_columns;

    for (int col = 0; col < bar_width; ++col) {
        if (col < solid_columns) {
            canvas.draw_text({cursor, y}, options_.glyphs.fill, fill_style_at(fill_style, col, bar_width));
            cursor += fill_width;
            continue;
        }

        if (use_head && col == solid_columns) {
            canvas.draw_text({cursor, y}, options_.glyphs.head, fill_style_at(fill_style, col, bar_width));
            cursor += head_width;
            continue;
        }

        canvas.draw_text({cursor, y}, options_.glyphs.empty, options_.track_style);
        cursor += empty_width;
    }

    if (!options_.glyphs.right_cap.empty()) {
        canvas.draw_text({cursor, y}, options_.glyphs.right_cap, fill_style);
    }
}

void ProgressBar::paint_filled_label(Canvas& canvas) const {
    const int width = bounds_.width;
    int fill_start = 0;
    int fill_end = std::clamp(static_cast<int>(value_ * width + 0.5), 0, width);

    if (options_.indeterminate) {
        const int pulse_width = std::max(
            options_.pulse_width,
            std::max(4, width / 4));
        const int travel = std::max(0, width - pulse_width);
        fill_start = bounce_position(normalized_phase(options_.animation_phase), travel);
        fill_end = fill_start + pulse_width;

        canvas.fill_rect({0, 0, width, 1}, ' ', options_.track_style);
        if (pulse_width > 0) {
            fill_rect_gradient(canvas, fill_start, 0, pulse_width, 1, ' ', options_.fill_style, width);
        }
    } else {
        if (fill_end > 0) {
            fill_rect_gradient(canvas, 0, 0, fill_end, 1, ' ', options_.fill_style, width);
        }
        if (fill_end < width) {
            canvas.fill_rect({fill_end, 0, width - fill_end, 1}, ' ', options_.track_style);
        }
    }

    if (options_.inside_label.empty()) {
        return;
    }

    const int label_width = text_display_width(options_.inside_label);
    const int x = std::max(0, (width - label_width) / 2);
    draw_label_on_fill(
        canvas,
        x,
        0,
        options_.inside_label,
        fill_start,
        fill_end,
        options_.inside_label_style,
        options_.fill_style,
        options_.track_style);
}

void ProgressBar::paint_shimmer_bar(
    Canvas& canvas,
    int x,
    int y,
    int bar_width,
    const Style* fill_override) const {
    if (bar_width <= 0) {
        return;
    }

    const Style& fill_style = fill_override != nullptr ? *fill_override : options_.fill_style;

    const int pulse_width = std::max(1, std::min(options_.pulse_width, bar_width));
    const int travel = std::max(0, bar_width - pulse_width);
    const int pos = bounce_position(normalized_phase(options_.animation_phase), travel);

    for (int column = 0; column < bar_width; ++column) {
        canvas.draw_text({x + column, y}, options_.glyphs.empty, options_.track_style);
    }

    Style pulse_style = fill_style;
    pulse_style.bold = true;
    for (int column = 0; column < pulse_width; ++column) {
        const int draw_x = x + pos + column;
        if (draw_x >= x + bar_width) {
            break;
        }
        canvas.draw_text(
            {draw_x, y},
            options_.glyphs.fill,
            fill_style_at(pulse_style, pos + column, bar_width));
    }
}

void ProgressBar::paint_pulse(Canvas& canvas) const {
    const int metrics_width = 17;
    const int bar_width = std::max(4, bounds_.width - metrics_width);
    const bool unicode = detect_glyph_set() != GlyphSet::Ascii;
    const char* track_glyph = unicode ? "\xe2\x94\x80" : "-"; // ─
    const char* pulse_glyph = unicode ? "\xe2\x94\x81" : "="; // ━

    for (int column = 0; column < bar_width; ++column) {
        canvas.draw_text({column, 0}, track_glyph, options_.track_style);
    }

    const int pulse_width = std::max(1, std::min(options_.pulse_width, bar_width));
    const int travel = std::max(0, bar_width - pulse_width);
    const int pos = bounce_position(normalized_phase(options_.animation_phase), travel);
    Style pulse_style = options_.fill_style;
    pulse_style.bold = true;
    for (int column = 0; column < pulse_width; ++column) {
        canvas.draw_text(
            {pos + column, 0},
            pulse_glyph,
            fill_style_at(pulse_style, pos + column, bar_width));
    }

    std::string metrics;
    if (options_.indeterminate || value_ <= 0.0) {
        metrics = " --%  --:--:--";
    } else {
        const ProgressBarStats& stats = options_.stats;
        const int current = stats.total > 0
            ? std::clamp(static_cast<int>(value_ * stats.total + 0.5), 0, stats.total)
            : stats.current;
        const int remaining = std::max(0, stats.total - current);
        const int eta_ms = stats.rate > 0.0
            ? static_cast<int>(remaining / stats.rate * 1000.0)
            : 0;
        metrics = ' ' + percent_text(value_) + "  " + format_eta_hms(eta_ms, stats.rate <= 0.0);
    }

    canvas.draw_text({bar_width, 0}, metrics, options_.percent_style);
}

void ProgressBar::paint_shimmer(Canvas& canvas) const {
    int x = 0;
    if (!options_.label.empty()) {
        canvas.draw_text({x, 0}, options_.label + " ", options_.label_style);
        x += text_display_width(options_.label) + 1;
    }

    const int bar_width = std::max(4, bounds_.width - x);
    paint_shimmer_bar(canvas, x, 0, bar_width);
}

void ProgressBar::paint_bounce(Canvas& canvas) const {
    const int inner_width = std::max(4, bounds_.width - 2);
    const int block_width = std::max(1, std::min(options_.pulse_width, inner_width));
    const int travel = std::max(0, inner_width - block_width);
    const int pos = bounce_position(normalized_phase(options_.animation_phase), travel);

    canvas.draw_text({0, 0}, "[", options_.track_style);
    for (int column = 0; column < inner_width; ++column) {
        const bool filled = column >= pos && column < pos + block_width;
        const char ch = filled ? '=' : ' ';
        canvas.draw_text(
            {1 + column, 0},
            std::string(1, ch),
            filled ? options_.fill_style : options_.track_style);
    }
    canvas.draw_text({1 + inner_width, 0}, "]", options_.track_style);
}

void ProgressBar::paint_slide_block(Canvas& canvas) const {
    const bool unicode = detect_glyph_set() != GlyphSet::Ascii;
    const char* empty_slot = unicode ? "\xe2\x96\xa1" : "[ ]"; // □ or [ ]
    const char* filled_slot = unicode ? "\xe2\x96\xa0" : "[#]"; // ■ or [#]
    const int slot_width = text_display_width(empty_slot);
    const int gap = 1;
    const int slots = std::max(1, (bounds_.width + gap) / (slot_width + gap));
    const int active = normalized_phase(options_.animation_phase) % slots;

    int x = 0;
    for (int slot = 0; slot < slots && x < bounds_.width; ++slot) {
        const bool filled = slot == active;
        canvas.draw_text(
            {x, 0},
            filled ? filled_slot : empty_slot,
            filled ? options_.fill_style : options_.track_style);
        x += slot_width + gap;
    }
}

void ProgressBar::paint_moving_dot(Canvas& canvas) const {
    const int span = std::max(4, std::min(bounds_.width, 16));
    const int pos = bounce_position(normalized_phase(options_.animation_phase), span - 1);
    const bool unicode = detect_glyph_set() != GlyphSet::Ascii;
    const char* dot_glyph = unicode ? "\xe2\x97\x8f" : "o"; // ●

    for (int column = 0; column < span; ++column) {
        if (column == pos) {
            canvas.draw_text({column, 0}, dot_glyph, options_.fill_style);
        } else {
            canvas.draw_text({column, 0}, ".", options_.track_style);
        }
    }
}

void ProgressBar::paint_task_row(Canvas& canvas) const {
    const int label_width = 15;
    const int percent_width = 5;
    const int eta_width = 8;
    const int bar_width = std::max(
        4,
        bounds_.width - label_width - percent_width - eta_width - 2);

    Style label_style = options_.label_style;
    if (options_.completed) {
        label_style.foreground = Color::Blue;
        label_style.dim = true;
    }

    std::string label = options_.label;
    while (text_display_width(label) < label_width) {
        label.push_back(' ');
    }
    const std::size_t label_bytes = text_byte_length_for_width(label, label_width);
    canvas.draw_text({0, 0}, label.substr(0, label_bytes), label_style);

    int x = label_width;
    Style bar_fill = options_.fill_style;
    if (options_.completed || value_ >= 1.0) {
        bar_fill.foreground = Color::Green;
    }

    if (options_.indeterminate) {
        paint_shimmer_bar(canvas, x, 0, bar_width, &bar_fill);
    } else {
        paint_bar_at(canvas, x, 0, bar_width, &bar_fill);
    }

    x += bar_width + 1;
    std::ostringstream percent;
    percent << std::setw(percent_width) << std::setfill(' ') << percent_text(value_);
    canvas.draw_text({x, 0}, percent.str(), options_.percent_style);

    x += percent_width + 1;
    std::string eta = options_.eta_text;
    if (eta.empty()) {
        if (value_ >= 1.0 || options_.completed) {
            eta = format_eta_hms(0, false);
        } else if (options_.stats.rate > 0.0 && options_.stats.total > 0) {
            const int current = std::clamp(
                static_cast<int>(value_ * options_.stats.total + 0.5),
                0,
                options_.stats.total);
            const int remaining = std::max(0, options_.stats.total - current);
            eta = format_eta_hms(static_cast<int>(remaining / options_.stats.rate * 1000.0), false);
        } else {
            eta = format_eta_hms(0, true);
        }
    }

    std::ostringstream eta_out;
    eta_out << std::setw(eta_width) << std::setfill(' ') << eta;
    canvas.draw_text({x, 0}, eta_out.str(), options_.eta_style);
}

void ProgressBar::paint_tqdm(Canvas& canvas) const {
    const int bar_width = std::clamp(12, 8, std::max(8, bounds_.width / 4));
    const int filled = std::clamp(static_cast<int>(value_ * bar_width + 0.5), 0, bar_width);

    std::string bar;
    for (int i = 0; i < bar_width; ++i) {
        bar += i < filled ? options_.glyphs.fill : options_.glyphs.empty;
    }

    const ProgressBarStats& stats = options_.stats;
    const int current = stats.total > 0
        ? std::clamp(static_cast<int>(value_ * stats.total + 0.5), 0, stats.total)
        : stats.current;
    const int remaining = std::max(0, stats.total - current);
    const int eta_ms = stats.rate > 0.0
        ? static_cast<int>(remaining / stats.rate * 1000.0)
        : 0;

    std::ostringstream out;
    out << options_.label << ": "
        << std::setw(3) << std::setfill(' ') << std::lround(value_ * 100.0) << "%|"
        << bar << "| "
        << current << '/' << stats.total
        << " [" << format_clock(stats.elapsed_ms) << '<' << format_clock(eta_ms) << ", "
        << std::fixed << std::setprecision(1) << stats.rate << stats.unit << "/s]";

    canvas.draw_text({0, 0}, out.str(), options_.label_style);
}

void ProgressBar::paint_braille_metric(Canvas& canvas) const {
    if (!options_.label.empty()) {
        canvas.draw_text({0, 0}, options_.label, options_.label_style);
    }

    if (!options_.metric_value.empty()) {
        const int value_width = text_display_width(options_.metric_value);
        const int x = std::max(0, bounds_.width - value_width);
        canvas.draw_text({x, 0}, options_.metric_value, options_.value_style);
    }

    const int bar_y = 1;
    const int width = bounds_.width;
    const int segment_width = std::max(1, text_display_width(options_.glyphs.fill));
    const int gap = options_.segmented ? 1 : 0;
    const int unit = segment_width + gap;
    const int segments = std::max(1, width / unit);
    const int filled_segments = std::clamp(static_cast<int>(value_ * segments + 0.5), 0, segments);

    int x = 0;
    for (int segment = 0; segment < segments && x < width; ++segment) {
        Style style = options_.track_style;
        std::string glyph = options_.glyphs.empty;

        if (segment < filled_segments) {
            glyph = options_.glyphs.fill;
            style = fill_style_at(options_.fill_style, segment, segments);
        }
        if (segment == filled_segments - 1 && filled_segments > 0 && filled_segments < segments) {
            glyph = options_.glyphs.head.empty() ? options_.glyphs.fill : options_.glyphs.head;
            style = fill_style_at(options_.head_style, segment, segments);
        }

        canvas.draw_text({x, bar_y}, glyph, style);
        x += unit;
    }
}

void ProgressBar::paint_braille_wave(Canvas& canvas) const {
    if (!options_.label.empty()) {
        canvas.draw_text({0, 0}, options_.label, options_.label_style);
    }

    static constexpr const char* kWaveGlyphs[] = {
        "\xe2\xa0\x80", // ⠀
        "\xe2\xa0\x92", // ⠒
        "\xe2\xa0\x9f", // ⠟
        "\xe2\xa3\xbf", // ⣿
        "\xe2\xa0\x9f", // ⠟
        "\xe2\xa0\x92", // ⠒
    };

    const int width = bounds_.width;
    const int bar_y = 1;
    const int phase = ((options_.animation_phase % 600) + 600) % 600;
    const int wave_count = static_cast<int>(sizeof(kWaveGlyphs) / sizeof(kWaveGlyphs[0]));

    for (int x = 0; x < width; ++x) {
        const int index = (x + phase / 4) % wave_count;
        const char* glyph = detect_glyph_set() == GlyphSet::Ascii ? ":" : kWaveGlyphs[index];
        canvas.draw_text({x, bar_y}, glyph, options_.fill_style);
    }
}

void ProgressBar::paint_glyph_bar(Canvas& canvas) const {
    int x = 0;
    const int y = 0;

    if (!options_.label.empty()) {
        canvas.draw_text({x, y}, options_.label + " ", options_.label_style);
        x += text_display_width(options_.label) + 1;
    }

    int bar_width = bar_column_count();
    const int available = bounds_.width - x;
    if (options_.show_percent && options_.percent_position == ProgressBarPercentPosition::Right) {
        const int percent_width = text_display_width(percent_text(value_)) + 1;
        bar_width = std::min(bar_width, std::max(0, available - percent_width));
    } else {
        bar_width = std::min(bar_width, available);
    }

    paint_bar_at(canvas, x, y, bar_width);
    x += rendered_bar_width(bar_width);

    if (options_.show_percent && options_.percent_position == ProgressBarPercentPosition::Right) {
        canvas.draw_text({x, y}, " " + percent_text(value_), options_.percent_style);
    }
}

void ProgressBar::paint(Canvas& canvas) const {
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    switch (options_.layout) {
    case ProgressBarLayout::FilledLabel:
        paint_filled_label(canvas);
        return;
    case ProgressBarLayout::Tqdm:
        paint_tqdm(canvas);
        return;
    case ProgressBarLayout::BrailleMetric:
        paint_braille_metric(canvas);
        return;
    case ProgressBarLayout::BrailleWave:
        paint_braille_wave(canvas);
        return;
    case ProgressBarLayout::Pulse:
        paint_pulse(canvas);
        return;
    case ProgressBarLayout::Shimmer:
        paint_shimmer(canvas);
        return;
    case ProgressBarLayout::Bounce:
        paint_bounce(canvas);
        return;
    case ProgressBarLayout::SlideBlock:
        paint_slide_block(canvas);
        return;
    case ProgressBarLayout::MovingDot:
        paint_moving_dot(canvas);
        return;
    case ProgressBarLayout::TaskRow:
        paint_task_row(canvas);
        return;
    default:
        paint_glyph_bar(canvas);
        return;
    }
}

} // namespace tuinator
