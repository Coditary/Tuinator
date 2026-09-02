#include <tuinator/widgets/charts/pie_chart.hpp>

#include <tuinator/core/event.hpp>
#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/text.hpp>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <variant>

namespace tuinator {

namespace {

constexpr double kTerminalAspect = 2.0;
constexpr double kTwoPi = 6.283185307179586;

std::string utf8_from(char32_t cp) {
    std::string out;
    if (cp < 0x80) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    return out;
}

char32_t braille_from_dots(const bool dots[8]) {
    static const int bits[8] = {0, 3, 1, 4, 2, 5, 6, 7};
    int mask = 0;
    for (int i = 0; i < 8; ++i) {
        if (dots[i]) {
            mask |= 1 << bits[i];
        }
    }
    return static_cast<char32_t>(0x2800 + mask);
}

double normalize_angle(double angle) {
    while (angle < 0.0) {
        angle += kTwoPi;
    }
    while (angle >= kTwoPi) {
        angle -= kTwoPi;
    }
    return angle;
}

double slice_total(const std::vector<PieChartSlice>& slices) {
    double total = 0.0;
    for (const PieChartSlice& slice : slices) {
        total += std::max(0.0, slice.value);
    }
    return total;
}

} // namespace

const PieChartStyleInfo* pie_chart_style_named(const char* id) {
    for (const PieChartStyleInfo& info : all_pie_chart_styles()) {
        if (info.id == id || (info.glyph[0] != '\0' && std::string(info.glyph) == id)) {
            return &info;
        }
    }
    return nullptr;
}

const std::vector<PieChartStyleInfo>& all_pie_chart_styles() {
    static const std::vector<PieChartStyleInfo> styles = {
        {"dots", "Thick dots", PieChartStyle::Dots, "●"},
        {"fine", "Fine dots", PieChartStyle::FineDots, "·"},
        {"small", "Small dots", PieChartStyle::SmallDots, "."},
        {"stars", "Stars", PieChartStyle::Stars, "*"},
        {"hash", "Hash", PieChartStyle::Hash, "#"},
        {"plus", "Plus", PieChartStyle::Plus, "+"},
        {"blocks", "Blocks", PieChartStyle::Blocks, "█"},
        {"braille", "Braille", PieChartStyle::Braille, "⠿"},
    };
    return styles;
}

std::string pie_chart_glyph_for(PieChartStyle style, const std::string& custom) {
    if (style == PieChartStyle::Custom && !custom.empty()) {
        return custom;
    }

    if (detect_glyph_set() == GlyphSet::Ascii) {
        switch (style) {
        case PieChartStyle::Dots:
            return "O";
        case PieChartStyle::FineDots:
        case PieChartStyle::SmallDots:
            return ".";
        case PieChartStyle::Blocks:
            return "#";
        case PieChartStyle::Braille:
            return ":";
        default:
            break;
        }
    }

    for (const PieChartStyleInfo& info : all_pie_chart_styles()) {
        if (info.style == style) {
            return info.glyph;
        }
    }

    return custom.empty() ? "●" : custom;
}

PieChart::PieChart(std::vector<PieChartSlice> slices, PieChartOptions options)
    : slices_(std::move(slices)),
      options_(std::move(options)) {}

void PieChart::set_slices(std::vector<PieChartSlice> slices) {
    slices_ = std::move(slices);
    mark_dirty();
}

void PieChart::set_options(PieChartOptions options) {
    options_ = std::move(options);
    mark_dirty();
}

void PieChart::set_style(PieChartStyle style) {
    options_.style = style;
    mark_dirty();
}

void PieChart::set_rotation(double radians) {
    options_.rotation = radians;
    mark_dirty();
}

void PieChart::set_interactive(bool interactive) {
    options_.interactive = interactive;
    mark_dirty();
}

void PieChart::set_on_change(std::function<void(const std::vector<PieChartSlice>&)> callback) {
    on_change_ = std::move(callback);
}

PieChart::Layout PieChart::compute_layout() const {
    Layout layout{};
    layout.title_rows = options_.title.empty() ? 0 : 1;
    layout.chart_radius = std::max(3, options_.diameter / 2);
    layout.chart_left = std::max(0, bounds_.width / 2 - layout.chart_radius);
    layout.chart_top = layout.title_rows;
    layout.legend_top = layout.chart_top + layout.chart_radius * 2 + 1;
    return layout;
}

Size PieChart::preferred_size() const {
    const int diameter = std::max(6, options_.diameter);
    int legend_width = 0;
    if (options_.show_legend) {
        for (const PieChartSlice& slice : slices_) {
            std::ostringstream line;
            line << slice.label;
            if (options_.show_percent) {
                line << " 100.0%";
            }
            legend_width = std::max(legend_width, text_display_width(line.str()));
        }
    }

    int title_width = options_.title.empty() ? 0 : text_display_width(options_.title);
    int height = (options_.title.empty() ? 0 : 1) + diameter + 1;
    if (options_.show_legend) {
        height += static_cast<int>(slices_.size());
    }

    return {std::max({diameter, legend_width, title_width}), height};
}

int PieChart::slice_at_angle(double angle) const {
    if (slices_.empty()) {
        return -1;
    }

    const double total = slice_total(slices_);
    if (total <= 0.0) {
        return 0;
    }

    double cursor = 0.0;
    for (std::size_t i = 0; i < slices_.size(); ++i) {
        cursor += std::max(0.0, slices_[i].value) / total * kTwoPi;
        if (angle < cursor) {
            return static_cast<int>(i);
        }
    }

    return static_cast<int>(slices_.size()) - 1;
}

double PieChart::angle_from_local(Point local, const Layout& layout) const {
    const int cx = layout.chart_left + layout.chart_radius;
    const int cy = layout.chart_top + layout.chart_radius;
    const double dx = static_cast<double>(local.x - cx);
    const double dy = static_cast<double>(local.y - cy) * kTerminalAspect;
    double angle = std::atan2(dy, dx) + (M_PI / 2.0) - options_.rotation;
    return normalize_angle(angle);
}

bool PieChart::point_in_chart(Point local, const Layout& layout) const {
    const int cx = layout.chart_left + layout.chart_radius;
    const int cy = layout.chart_top + layout.chart_radius;
    const double dx = static_cast<double>(local.x - cx);
    const double dy = static_cast<double>(local.y - cy) * kTerminalAspect;
    const double radius = static_cast<double>(layout.chart_radius);
    return (dx * dx + dy * dy) <= radius * radius;
}

void PieChart::adjust_slice_boundary(int slice_index, double delta_angle) {
    if (slice_index < 0 || slices_.size() < 2) {
        return;
    }

    const int next = (slice_index + 1) % static_cast<int>(slices_.size());
    const double total = slice_total(slices_);
    if (total <= 0.0) {
        return;
    }

    const double min_value = total * options_.min_slice_ratio;
    const double delta_value = delta_angle / kTwoPi * total;
    const double new_current = slices_[static_cast<std::size_t>(slice_index)].value + delta_value;
    const double new_next = slices_[static_cast<std::size_t>(next)].value - delta_value;

    if (new_current < min_value || new_next < min_value) {
        return;
    }

    slices_[static_cast<std::size_t>(slice_index)].value = new_current;
    slices_[static_cast<std::size_t>(next)].value = new_next;
    mark_dirty();

    if (on_change_) {
        on_change_(slices_);
    }
}

void PieChart::begin_drag(int slice_index, double angle) {
    dragging_ = true;
    drag_slice_ = slice_index;
    drag_start_angle_ = angle;
    drag_start_values_.clear();
    drag_start_values_.reserve(slices_.size());
    for (const PieChartSlice& slice : slices_) {
        drag_start_values_.push_back(slice.value);
    }
}

void PieChart::update_drag(double angle) {
    if (!dragging_ || drag_slice_ < 0) {
        return;
    }

    adjust_slice_boundary(drag_slice_, angle - drag_start_angle_);
    drag_start_angle_ = angle;
    for (std::size_t i = 0; i < slices_.size(); ++i) {
        drag_start_values_[i] = slices_[i].value;
    }
}

void PieChart::end_drag() {
    dragging_ = false;
    drag_slice_ = -1;
    drag_start_values_.clear();
}

void PieChart::paint_cell_glyph(
    Canvas& canvas,
    int x,
    int y,
    int slice_index,
    const Layout& /*layout*/) const {
    if (slice_index < 0 || slice_index >= static_cast<int>(slices_.size())) {
        return;
    }

    const std::string glyph =
        pie_chart_glyph_for(options_.style, options_.custom_glyph);
    canvas.draw_text({x, y}, glyph, slices_[static_cast<std::size_t>(slice_index)].style);
}

void PieChart::paint_braille_cell(
    Canvas& canvas,
    int x,
    int y,
    int slice_index,
    const bool dots[8]) const {
    if (slice_index < 0 || slice_index >= static_cast<int>(slices_.size())) {
        return;
    }

    const std::string glyph = utf8_from(braille_from_dots(dots));
    canvas.draw_text({x, y}, glyph, slices_[static_cast<std::size_t>(slice_index)].style);
}

void PieChart::paint_chart(Canvas& canvas, const Layout& layout) const {
    if (slices_.empty() || layout.chart_radius <= 0) {
        return;
    }

    const int cx = layout.chart_left + layout.chart_radius;
    const int cy = layout.chart_top + layout.chart_radius;
    const double radius = static_cast<double>(layout.chart_radius);

    if (options_.style == PieChartStyle::Braille) {
        for (int y = layout.chart_top; y < layout.chart_top + layout.chart_radius * 2; ++y) {
            for (int x = layout.chart_left; x < layout.chart_left + layout.chart_radius * 2; ++x) {
                bool dots[8] = {};
                int active = 0;
                int dominant_slice = -1;

                for (int sy = 0; sy < 4; ++sy) {
                    for (int sx = 0; sx < 2; ++sx) {
                        const double px = static_cast<double>(x - layout.chart_left) + (sx + 0.5) / 2.0;
                        const double py = static_cast<double>(y - layout.chart_top) + (sy + 0.5) / 4.0;
                        const double dx = px - radius;
                        const double dy = (py - radius) * kTerminalAspect;
                        if ((dx * dx + dy * dy) > radius * radius) {
                            continue;
                        }

                        double angle = std::atan2(dy, dx) + (M_PI / 2.0) - options_.rotation;
                        angle = normalize_angle(angle);
                        const int slice = slice_at_angle(angle);
                        dots[sy * 2 + sx] = true;
                        ++active;
                        dominant_slice = slice;
                    }
                }

                if (active > 0) {
                    paint_braille_cell(canvas, x, y, dominant_slice, dots);
                }
            }
        }
        return;
    }

    for (int y = layout.chart_top; y < layout.chart_top + layout.chart_radius * 2; ++y) {
        for (int x = layout.chart_left; x < layout.chart_left + layout.chart_radius * 2; ++x) {
            const double dx = static_cast<double>(x - cx);
            const double dy = static_cast<double>(y - cy) * kTerminalAspect;
            if ((dx * dx + dy * dy) > radius * radius) {
                continue;
            }

            double angle = std::atan2(dy, dx) + (M_PI / 2.0) - options_.rotation;
            angle = normalize_angle(angle);
            paint_cell_glyph(canvas, x, y, slice_at_angle(angle), layout);
        }
    }
}

void PieChart::paint_legend(Canvas& canvas, const Layout& layout) const {
    if (!options_.show_legend || slices_.empty()) {
        return;
    }

    const double total = slice_total(slices_);
    int row = layout.legend_top;
    for (const PieChartSlice& slice : slices_) {
        std::ostringstream line;
        line << slice.label;
        if (options_.show_percent && total > 0.0) {
            line << ' ' << std::fixed << std::setprecision(1)
                 << (std::max(0.0, slice.value) / total * 100.0) << '%';
        }

        Style style = options_.legend_style;
        if (style.foreground == Color::Default && !style.foreground_rgb) {
            style = slice.style;
        }

        canvas.draw_text({0, row}, line.str(), style);
        ++row;
    }
}

void PieChart::paint(Canvas& canvas) const {
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    const Layout layout = compute_layout();

    if (!options_.title.empty()) {
        canvas.draw_text({0, 0}, options_.title, options_.title_style);
    }

    paint_chart(canvas, layout);
    paint_legend(canvas, layout);
}

bool PieChart::handle_event(const Event& event) {
    if (!options_.interactive || slices_.size() < 2) {
        return false;
    }

    const Layout layout = compute_layout();

    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        const Point local{mouse->position.x - bounds_.x, mouse->position.y - bounds_.y};

        if (dragging_) {
            if (mouse->action == MouseAction::Release) {
                end_drag();
                return true;
            }

            if (mouse->action == MouseAction::Move && mouse->left_pressed) {
                update_drag(angle_from_local(local, layout));
                return true;
            }
        }

        if (!contains_point(mouse->position)) {
            return false;
        }

        const bool drag_motion =
            mouse->action == MouseAction::Move && mouse->left_pressed;

        if (mouse->action == MouseAction::Click
            || mouse->action == MouseAction::Press
            || drag_motion) {
            if (!point_in_chart(local, layout)) {
                if (mouse->action == MouseAction::Release) {
                    end_drag();
                }
                return false;
            }

            const double angle = angle_from_local(local, layout);
            const int slice = slice_at_angle(angle);

            if (mouse->action == MouseAction::Click || mouse->action == MouseAction::Press) {
                begin_drag(slice, angle);
                return true;
            }

            if (drag_motion) {
                if (!dragging_) {
                    begin_drag(slice, angle);
                }
                update_drag(angle);
                return true;
            }
        }

        if (mouse->action == MouseAction::Release && dragging_) {
            end_drag();
            return true;
        }

        if (mouse->action == MouseAction::WheelUp || mouse->action == MouseAction::WheelDown) {
            if (!point_in_chart(local, layout)) {
                return false;
            }

            const int slice = slice_at_angle(angle_from_local(local, layout));
            const double delta = mouse->action == MouseAction::WheelUp ? -0.05 : 0.05;
            adjust_slice_boundary(slice, delta * kTwoPi);
            return true;
        }

        return false;
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key || !is_focused()) {
        return false;
    }

    if (focused_slice_ >= static_cast<int>(slices_.size())) {
        focused_slice_ = 0;
    }

    switch (key->key) {
    case Key::Left:
        adjust_slice_boundary(focused_slice_, -0.08 * kTwoPi);
        return true;
    case Key::Right:
        adjust_slice_boundary(focused_slice_, 0.08 * kTwoPi);
        return true;
    case Key::Up:
        focused_slice_ = (focused_slice_ + static_cast<int>(slices_.size()) - 1)
            % static_cast<int>(slices_.size());
        mark_dirty();
        return true;
    case Key::Down:
        focused_slice_ = (focused_slice_ + 1) % static_cast<int>(slices_.size());
        mark_dirty();
        return true;
    default:
        break;
    }

    return false;
}

} // namespace tuinator
