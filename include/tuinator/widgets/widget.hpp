#pragma once

#include <tuinator/core/event.hpp>
#include <tuinator/core/geometry.hpp>
#include <tuinator/render/paint_context.hpp>

#include <functional>
#include <memory>
#include <vector>

namespace tuinator {

class Widget {
public:
    virtual ~Widget() = default;

    virtual Size preferred_size() const = 0;
    virtual void layout(Rect bounds);
    virtual void paint(PaintContext& ctx) const;

    virtual bool handle_event(const Event& event);
    virtual bool is_focusable() const { return false; }
    virtual bool wants_full_screen() const { return false; }
    virtual bool captures_pointer() const { return false; }
    virtual bool pointer_active() const { return false; }
    virtual bool wants_hover_redraw() const { return false; }

    void add_child(std::unique_ptr<Widget> child);
    const std::vector<std::unique_ptr<Widget>>& children() const { return children_; }

    Rect bounds() const { return bounds_; }

    bool contains_point(Point point) const;
    virtual Widget* hit_test(Point point);
    virtual Widget* hit_test_focusable(Point point);

    bool is_focused() const { return focused_; }
    void set_focused(bool focused);

    virtual bool has_focused_descendant() const;

    virtual void set_on_dirty(std::function<void(Rect)> callback);
    void set_on_layout(std::function<void()> callback);
    void mark_dirty();
    void mark_layout_dirty();

    void set_flex(int flex);
    int flex() const { return flex_; }

    virtual void collect_focusable(std::vector<Widget*>& out);

    virtual void for_each_child(const std::function<void(Widget*)>& visitor);

protected:
    Rect bounds_{};
    bool focused_ = false;
    int flex_ = 0;
    std::function<void(Rect)> on_dirty_;
    std::function<void()> on_layout_;
    std::vector<std::unique_ptr<Widget>> children_;
};

} // namespace tuinator
