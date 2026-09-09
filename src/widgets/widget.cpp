#include <tuinator/debug/debug_paint.hpp>
#include <tuinator/render/style_resolver.hpp>
#include <tuinator/widgets/widget.hpp>

#include <algorithm>
#include <string_view>
#include <variant>

namespace tuinator {

namespace {

bool is_mouse_activation(const MouseEvent& mouse) {
    return mouse.action == MouseAction::Click || mouse.action == MouseAction::Release;
}

} // namespace

void Widget::layout(Rect bounds) {
    bounds_ = bounds;

    for (auto& child : children_) {
        child->layout(bounds_);
    }
}

void Widget::paint_bounds_background(PaintContext& ctx, Style fallback) const {
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    debug_paint_note_background_fill();

    const Style background = ctx.styles().text(*this, fallback);
    ctx.canvas.fill_rect({{0, 0}, bounds_.size()}, ' ', background);
}

void Widget::paint(PaintContext& ctx) const {
    for (const auto& child : children_) {
        const Rect local{
            child->bounds().x - bounds_.x,
            child->bounds().y - bounds_.y,
            child->bounds().width,
            child->bounds().height,
        };

        ctx.with_clip(local, [&](PaintContext& child_ctx) { child->paint(child_ctx); });
    }
}

bool Widget::handle_event(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        Widget* target = hit_test(mouse->position);
        if (target != nullptr && target != this) {
            return target->handle_event(event);
        }

        if (!is_mouse_activation(*mouse)) {
            return false;
        }

        return false;
    }

    for (auto& child : children_) {
        if (child->handle_event(event)) {
            return true;
        }
    }
    return false;
}

bool Widget::contains_point(Point point) const { return bounds_.contains(point); }

Widget* Widget::hit_test(Point point) {
    if (!bounds_.contains(point)) {
        return nullptr;
    }

    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if (Widget* hit = (*it)->hit_test(point)) {
            return hit;
        }
    }

    return this;
}

Widget* Widget::hit_test_focusable(Point point) {
    if (!bounds_.contains(point)) {
        return nullptr;
    }

    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if (Widget* hit = (*it)->hit_test_focusable(point)) {
            return hit;
        }
    }

    return is_focusable() ? this : nullptr;
}

bool Widget::has_focused_descendant() const {
    if (focused_) {
        return true;
    }

    for (const auto& child : children_) {
        if (child->has_focused_descendant()) {
            return true;
        }
    }

    return false;
}

void Widget::add_child(std::unique_ptr<Widget> child) {
    if (on_dirty_) {
        child->set_on_dirty(on_dirty_);
    }
    if (on_layout_) {
        child->set_on_layout(on_layout_);
    }
    attach_child_widget(child.get());
    children_.push_back(std::move(child));
}

void Widget::set_widget_id(std::string id) {
    widget_id_ = std::move(id);
    mark_dirty();
}

void Widget::add_widget_class(std::string class_name) {
    if (has_widget_class(class_name)) {
        return;
    }
    widget_classes_.push_back(std::move(class_name));
    mark_dirty();
}

void Widget::clear_widget_classes() {
    if (widget_classes_.empty()) {
        return;
    }
    widget_classes_.clear();
    mark_dirty();
}

bool Widget::has_widget_class(std::string_view class_name) const {
    return std::find(widget_classes_.begin(), widget_classes_.end(), class_name) != widget_classes_.end();
}

void Widget::apply_stylesheet(const StyleResolver& styles) {
    const WidgetOptions opts = styles.options(*this);
    if (opts.has("flex")) {
        set_flex(opts.int_or("flex", flex()));
    }
}

void Widget::attach_child_widget(Widget* child) {
    if (child == nullptr) {
        return;
    }
    child->parent_ = this;
}

void Widget::set_focused(bool focused) {
    if (focused_ == focused) {
        return;
    }

    focused_ = focused;
    mark_dirty();
}

void Widget::set_enabled(bool enabled) {
    if (enabled_ == enabled) {
        return;
    }

    enabled_ = enabled;
    mark_dirty();
}

void Widget::set_hovered(bool hovered) {
    if (hovered_ == hovered) {
        return;
    }

    hovered_ = hovered;
    mark_dirty();
}

void Widget::set_on_dirty(std::function<void(Rect)> callback) {
    on_dirty_ = std::move(callback);

    for (auto& child : children_) {
        child->set_on_dirty(on_dirty_);
    }
}

void Widget::set_on_layout(std::function<void()> callback) {
    on_layout_ = std::move(callback);

    for (auto& child : children_) {
        child->set_on_layout(on_layout_);
    }
}

void Widget::set_flex(int flex) { flex_ = std::max(0, flex); }

void Widget::mark_dirty() {
    if (!on_dirty_) {
        return;
    }

    if (bounds_.width <= 0 || bounds_.height <= 0) {
        on_dirty_({});
        return;
    }

    on_dirty_(bounds_);
}

void Widget::mark_layout_dirty() {
    mark_dirty();
    if (on_layout_) {
        on_layout_();
    }
}

void Widget::collect_focusable(std::vector<Widget*>& out) {
    if (is_focusable()) {
        out.push_back(this);
    }

    for (auto& child : children_) {
        child->collect_focusable(out);
    }
}

void Widget::for_each_child(const std::function<void(Widget*)>& visitor) {
    for (auto& child : children_) {
        visitor(child.get());
    }
}

void Widget::for_each_descendant(const std::function<void(Widget*)>& visitor) {
    visitor(this);
    for (auto& child : children_) {
        child->for_each_descendant(visitor);
    }
}

} // namespace tuinator
