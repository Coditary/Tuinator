#include <tuinator/widgets/widget.hpp>

#include <algorithm>
#include <variant>

namespace tuinator {

namespace {

bool is_mouse_activation(const MouseEvent& mouse) {
    return mouse.action == MouseAction::Click
        || mouse.action == MouseAction::Release;
}

} // namespace

void Widget::layout(Rect bounds) {
    bounds_ = bounds;

    for (auto& child : children_) {
        child->layout(bounds_);
    }
}

void Widget::paint(Canvas& canvas) const {
    for (const auto& child : children_) {
        const Rect local{
            child->bounds().x - bounds_.x,
            child->bounds().y - bounds_.y,
            child->bounds().width,
            child->bounds().height,
        };

        canvas.with_clip(local, [&](Canvas& clipped) {
            child->paint(clipped);
        });
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

bool Widget::contains_point(Point point) const {
    return bounds_.contains(point);
}

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
    children_.push_back(std::move(child));
}

void Widget::set_focused(bool focused) {
    if (focused_ == focused) {
        return;
    }

    focused_ = focused;
    mark_dirty();
}

void Widget::set_on_dirty(std::function<void()> callback) {
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

void Widget::set_flex(int flex) {
    flex_ = std::max(0, flex);
}

void Widget::mark_dirty() {
    if (on_dirty_) {
        on_dirty_();
    }
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

} // namespace tuinator
