#pragma once

#include <tuinator/render/paint_context.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/render/style_resolver.hpp>
#include <tuinator/widgets/widget.hpp>

#include <functional>

namespace tuinator {

/// Free-form drawing surface. Paint a callback, compose child widgets, or both.
///
/// Example — custom drawing:
/// ```cpp
/// auto canvas = std::make_unique<tuinator::CustomPaint>(
///     [](tuinator::PaintContext& ctx, tuinator::Rect area) {
///         ctx.canvas.fill_rect(area, '.', {});
///         ctx.canvas.draw_text({1, 0}, "Hello", {});
///     },
///     tuinator::Size{20, 5});
/// ```
///
/// Example — composition (children painted under the callback):
/// ```cpp
/// auto host = std::make_unique<tuinator::CustomPaint>();
/// host->add_child(std::make_unique<tuinator::Label>("Overlay"));
/// host->set_on_paint([](tuinator::PaintContext& ctx, tuinator::Rect area) {
///     ctx.canvas.fill_rect(area, ' ', tuinator::style_bg(tuinator::Color::Blue));
/// });
/// ```
class CustomPaint : public Widget {
  public:
    using PaintCallback = std::function<void(PaintContext& ctx, Rect bounds)>;
    using SizeCallback = std::function<Size()>;

    CustomPaint();
    explicit CustomPaint(PaintCallback on_paint, Size preferred = {1, 1});
    CustomPaint(PaintCallback on_paint, SizeCallback preferred_size);

    void set_on_paint(PaintCallback callback);
    void set_preferred_size(Size size);
    void set_preferred_size(SizeCallback callback);
    void set_background(Style background);
    void set_paint_children_first(bool children_first);

    void set_needs_periodic_idle(bool enabled);
    bool needs_periodic_idle() const override { return needs_periodic_idle_; }

    std::string_view widget_type_name() const override { return "CustomPaint"; }
    void apply_stylesheet(const StyleResolver& styles) override;

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;

  private:
    void paint_children(PaintContext& ctx) const;

    PaintCallback on_paint_;
    SizeCallback preferred_size_;
    Size fixed_size_{1, 1};
    Style background_{};
    bool paint_children_first_ = true;
    bool needs_periodic_idle_ = false;
};

} // namespace tuinator
