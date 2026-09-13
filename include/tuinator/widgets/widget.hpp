#pragma once

#include <tuinator/core/event.hpp>
#include <tuinator/core/geometry.hpp>
#include <tuinator/render/paint_context.hpp>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

class StyleResolver;

class Widget {
  public:
    virtual ~Widget() = default;

    virtual Size preferred_size() const = 0;
    virtual void layout(Rect bounds);
    virtual void paint(PaintContext& ctx) const;

    /// Fill widget bounds with the resolved background style before drawing content.
    void paint_bounds_background(PaintContext& ctx, Style fallback = {}) const;

    virtual bool handle_event(const Event& event);
    virtual bool is_focusable() const { return false; }
    virtual bool wants_full_screen() const { return false; }
    virtual bool captures_pointer() const { return false; }
    virtual bool captures_keyboard() const { return captures_pointer() || is_dropdown_open(); }
    virtual bool pointer_active() const { return false; }
    virtual bool wants_hover_redraw() const { return false; }
    virtual void on_idle() {}
    /// When true, the application polls input periodically so on_idle() can react.
    virtual bool needs_periodic_idle() const { return false; }
    virtual bool wants_initial_focus() const { return false; }
    virtual bool is_shell_terminal() const { return false; }

    void add_child(std::unique_ptr<Widget> child);
    const std::vector<std::unique_ptr<Widget>>& children() const { return children_; }

    Rect bounds() const { return bounds_; }

    bool contains_point(Point point) const;
    virtual Widget* hit_test(Point point);
    virtual Widget* hit_test_focusable(Point point);

    bool is_focused() const { return focused_; }
    void set_focused(bool focused);

    bool is_enabled() const { return enabled_; }
    void set_enabled(bool enabled);

    bool is_hovered() const { return hovered_; }
    void set_hovered(bool hovered);
    virtual bool wants_hover() const { return false; }

    virtual bool is_dropdown_open() const { return false; }

    virtual bool has_focused_descendant() const;

    virtual void set_on_dirty(std::function<void(Rect)> callback);
    virtual void set_on_layout(std::function<void()> callback);
    void mark_dirty();
    void mark_layout_dirty();

    void set_flex(int flex);
    int flex() const { return flex_; }

    virtual void collect_focusable(std::vector<Widget*>& out);

    virtual void for_each_child(const std::function<void(Widget*)>& visitor);
    virtual void for_each_descendant(const std::function<void(Widget*)>& visitor);

    virtual std::string_view widget_type_name() const { return "Widget"; }

    void set_widget_id(std::string id);
    const std::string& widget_id() const { return widget_id_; }

    void add_widget_class(std::string class_name);
    void clear_widget_classes();
    bool has_widget_class(std::string_view class_name) const;

    Widget* parent_widget() const { return parent_; }

    virtual void apply_stylesheet(const StyleResolver& styles);

  protected:
    void attach_child_widget(Widget* child);
    Rect bounds_{};
    bool focused_ = false;
    bool enabled_ = true;
    bool hovered_ = false;
    int flex_ = 0;
    std::function<void(Rect)> on_dirty_;
    std::function<void()> on_layout_;
    std::vector<std::unique_ptr<Widget>> children_;
    Widget* parent_ = nullptr;
    std::string widget_id_;
    std::vector<std::string> widget_classes_;
};

} // namespace tuinator
