#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/render/style_resolver.hpp>
#include <tuinator/widgets/charts/chart_widget.hpp>
#include <tuinator/widgets/charts/chart_widget_paint.hpp>
#include <tuinator/widgets/widget.hpp>

#include <functional>
#include <string>
#include <vector>

namespace tuinator {

enum class PieChartStyle {
    Dots,
    FineDots,
    SmallDots,
    Stars,
    Hash,
    Plus,
    Blocks,
    Braille,
    Custom,
};

struct PieChartSlice {
    std::string label;
    double value = 0.0;
    Style style{};
};

struct PieChartStyleInfo {
    const char* id = "";
    const char* title = "";
    PieChartStyle style = PieChartStyle::Dots;
    const char* glyph = "";
};

struct PieChartOptions {
    PieChartStyle style = PieChartStyle::Dots;
    std::string custom_glyph;
    std::string title;
    Style title_style{};
    Style legend_style{};
    bool show_legend = true;
    bool show_percent = true;
    bool interactive = false;
    double rotation = 0.0;
    int diameter = 14;
    double min_slice_ratio = 0.02;
};

const PieChartStyleInfo* pie_chart_style_named(const char* id);
const std::vector<PieChartStyleInfo>& all_pie_chart_styles();
std::string pie_chart_glyph_for(PieChartStyle style, const std::string& custom = {});

class PieChart : public Widget, public ChartWidget {
  public:
    PieChart(std::vector<PieChartSlice> slices = {}, PieChartOptions options = {});

    const std::vector<PieChartSlice>& slices() const { return slices_; }
    const PieChartOptions& options() const { return options_; }

    void set_slices(std::vector<PieChartSlice> slices);
    void set_options(PieChartOptions options);
    void set_style(PieChartStyle style);
    void set_rotation(double radians);
    void set_interactive(bool interactive);
    void set_on_change(std::function<void(const std::vector<PieChartSlice>&)> callback);

    std::string_view widget_type_name() const override { return "PieChart"; }
    void apply_stylesheet(const StyleResolver& styles) override;

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return options_.interactive; }
    bool captures_pointer() const override { return options_.interactive; }
    bool pointer_active() const override { return dragging_; }

  private:
    struct Layout {
        int title_rows = 0;
        int chart_top = 0;
        int chart_left = 0;
        int chart_radius = 0;
        int legend_top = 0;
    };

    Layout compute_layout() const;
    int slice_at_angle(double angle) const;
    double angle_from_local(Point local, const Layout& layout) const;
    bool point_in_chart(Point local, const Layout& layout) const;
    void adjust_slice_boundary(int slice_index, double delta_angle);
    void begin_drag(int slice_index, double angle);
    void update_drag(double angle);
    void end_drag();

    void paint_chart(Canvas& canvas, const Layout& layout) const;
    void paint_legend(Canvas& canvas, const Layout& layout) const;
    void paint_cell_glyph(Canvas& canvas, int x, int y, int slice_index, const Layout& layout) const;
    void paint_braille_cell(Canvas& canvas, int x, int y, int slice_index, const bool dots[8]) const;

    std::vector<PieChartSlice> slices_;
    PieChartOptions options_;
    mutable ChartPaintSupport paint_{};
    std::function<void(const std::vector<PieChartSlice>&)> on_change_;

    bool dragging_ = false;
    int drag_slice_ = -1;
    int focused_slice_ = 0;
    double drag_start_angle_ = 0.0;
    std::vector<double> drag_start_values_;
};

} // namespace tuinator
