#include <tuinator/widgets/scroll_view.hpp>

#include <tuinator/core/event.hpp>
#include <tuinator/core/geometry.hpp>

#include <algorithm>
#include <variant>

namespace tuinator {

namespace {

bool widget_tree_contains(const Widget* root, const Widget* target) {
    if (root == nullptr || target == nullptr) {
        return false;
    }

    if (root == target) {
        return true;
    }

    for (const auto& child : root->children()) {
        if (widget_tree_contains(child.get(), target)) {
            return true;
        }
    }

    return false;
}

} // namespace

ScrollView::ScrollView(std::unique_ptr<Widget> content, ScrollViewOptions options)
    : content_(std::move(content)), options_(options) {}

int ScrollView::max_scroll_y() const {
    return std::max(0, content_height_ - bounds_.height);
}

void ScrollView::scroll_to(int y) {
    scroll_y_ = y;
    clamp_scroll();
    layout_content();
    mark_dirty();
}

void ScrollView::scroll_by(int delta) {
    scroll_to(scroll_y_ + delta);
}

void ScrollView::refresh_content() {
    layout_content();
    mark_dirty();
}

bool ScrollView::contains_widget(const Widget* widget) const {
    return widget_tree_contains(content_.get(), widget);
}

void ScrollView::ensure_visible(const Widget* widget) {
    if (!content_ || widget == nullptr || !contains_widget(widget)) {
        return;
    }

    const int content_top = content_->bounds().y;
    const int widget_top = widget->bounds().y - content_top;
    const int widget_bottom = widget_top + widget->bounds().height;

    if (widget_top < scroll_y_) {
        scroll_to(widget_top);
        return;
    }

    if (widget_bottom > scroll_y_ + bounds_.height) {
        scroll_to(widget_bottom - bounds_.height);
    }
}

Size ScrollView::preferred_size() const {
    return {options_.width, options_.height};
}

void ScrollView::layout(Rect bounds) {
    bounds_ = bounds;
    layout_content();
}

void ScrollView::layout_content() {
    if (!content_) {
        content_height_ = 0;
        return;
    }

    clamp_scroll();
    const Size pref = content_->preferred_size();
    content_height_ = pref.height;
    content_->layout({
        bounds_.x,
        bounds_.y,
        bounds_.width,
        std::max(pref.height, bounds_.height),
    });
}

void ScrollView::paint(Canvas& canvas) const {
    if (!content_) {
        return;
    }

    canvas.with_clip({{0, 0}, bounds_.size()}, [&](Canvas& viewport) {
        Canvas scrolled = viewport.scrolled(0, -scroll_y_);
        content_->paint(scrolled);
    });
}

Widget* ScrollView::hit_test(Point point) {
    if (bounds_.contains(point)) {
        return this;
    }
    return nullptr;
}

Widget* ScrollView::hit_test_focusable(Point point) {
    if (!bounds_.contains(point) || !content_) {
        return nullptr;
    }

    const Point adjusted{point.x, point.y + scroll_y_};
    return content_->hit_test_focusable(adjusted);
}

void ScrollView::collect_focusable(std::vector<Widget*>& out) {
    if (content_) {
        content_->collect_focusable(out);
    }
}

bool ScrollView::has_focused_descendant() const {
    return content_ && content_->has_focused_descendant();
}

bool ScrollView::handle_event(const Event& event) {
    if (try_scroll(event)) {
        return true;
    }

    if (!content_) {
        return false;
    }

    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (!bounds_.contains(mouse->position)) {
            return false;
        }

        MouseEvent adjusted = *mouse;
        adjusted.position.y += scroll_y_;
        return content_->handle_event(adjusted);
    }

    if (content_->has_focused_descendant() && content_->handle_event(event)) {
        return true;
    }

    return false;
}

bool ScrollView::try_scroll(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (!bounds_.contains(mouse->position)) {
            return false;
        }

        if (mouse->action == MouseAction::WheelUp) {
            scroll_by(-3);
            return true;
        }

        if (mouse->action == MouseAction::WheelDown) {
            scroll_by(3);
            return true;
        }

        return false;
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key) {
        return false;
    }

    switch (key->key) {
    case Key::PageUp:
        scroll_by(-bounds_.height);
        return true;
    case Key::PageDown:
        scroll_by(bounds_.height);
        return true;
    case Key::Home:
        scroll_to(0);
        return true;
    case Key::End:
        scroll_to(max_scroll_y());
        return true;
    default:
        return false;
    }
}

void ScrollView::clamp_scroll() {
    scroll_y_ = std::clamp(scroll_y_, 0, max_scroll_y());
}

} // namespace tuinator
