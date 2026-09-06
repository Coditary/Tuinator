#include <tuinator/core/event.hpp>
#include <tuinator/core/geometry.hpp>
#include <tuinator/widgets/containers/scroll_view.hpp>

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
    : content_(std::move(content)), options_(std::move(options)) {}

int ScrollView::max_scroll_x() const {
    const auto metrics = scrollbar_metrics();
    return std::max(0, content_width_ - metrics.viewport_width);
}

int ScrollView::max_scroll_y() const {
    const auto metrics = scrollbar_metrics();
    return std::max(0, content_height_ - metrics.viewport_height);
}

ScrollbarMetrics ScrollView::scrollbar_metrics() const {
    return compute_scrollbar_metrics(bounds_.width, bounds_.height, content_width_, content_height_,
                                     options_.scrollbars.config);
}

ScrollbarLayout ScrollView::scrollbar_layout() const {
    return compute_scrollbar_layout(bounds_.width, bounds_.height, content_width_, content_height_, scroll_x_,
                                    scroll_y_, options_.scrollbars.config, options_.scrollbars.behavior.show_arrows);
}

ScrollbarScrollActions ScrollView::scrollbar_actions() {
    ScrollbarScrollActions actions;
    actions.scroll_by = [this](int delta_x, int delta_y) { scroll_by(delta_x, delta_y); };
    actions.scroll_to = [this](int x, int y) { scroll_to(x, y); };
    return actions;
}

Point ScrollView::to_local(Point terminal) const { return {terminal.x - bounds_.x, terminal.y - bounds_.y}; }

Point ScrollView::to_content_local(Point terminal) const {
    const Point local = to_local(terminal);
    return {local.x + scroll_x_, local.y + scroll_y_};
}

void ScrollView::scroll_to(int x, int y) {
    scroll_x_ = x;
    scroll_y_ = y;
    clamp_scroll();
    layout_content();
    mark_dirty();
}

void ScrollView::scroll_by(int delta_x, int delta_y) { scroll_to(scroll_x_ + delta_x, scroll_y_ + delta_y); }

void ScrollView::set_on_dirty(std::function<void(Rect)> callback) {
    Widget::set_on_dirty(std::move(callback));
    bind_content_dirty_callback();
}

void ScrollView::bind_content_dirty_callback() {
    if (!content_ || !on_dirty_) {
        return;
    }

    content_->set_on_dirty([this](Rect region) {
        if (!on_dirty_) {
            return;
        }

        if (region.width <= 0 || region.height <= 0) {
            on_dirty_(bounds_);
            return;
        }

        const ScrollbarLayout layout = scrollbar_layout();
        const Rect viewport{
            bounds_.x,
            bounds_.y,
            layout.metrics.viewport_width,
            layout.metrics.viewport_height,
        };

        const Rect translated{
            bounds_.x + region.x - scroll_x_,
            bounds_.y + region.y - scroll_y_,
            region.width,
            region.height,
        };

        const Rect visible = intersect(translated, viewport);
        if (visible.width <= 0 || visible.height <= 0) {
            return;
        }

        on_dirty_(visible);
    });
}

void ScrollView::refresh_content() {
    layout_content();
    mark_dirty();
}

bool ScrollView::contains_widget(const Widget* widget) const { return widget_tree_contains(content_.get(), widget); }

void ScrollView::ensure_visible(const Widget* widget) {
    if (!content_ || widget == nullptr || !contains_widget(widget)) {
        return;
    }

    const int widget_top = widget->bounds().y - content_->bounds().y;
    const int widget_bottom = widget_top + widget->bounds().height;
    const int widget_left = widget->bounds().x - content_->bounds().x;
    const int widget_right = widget_left + widget->bounds().width;

    const auto metrics = scrollbar_metrics();
    int next_x = scroll_x_;
    int next_y = scroll_y_;

    if (widget_top < next_y) {
        next_y = widget_top;
    } else if (widget_bottom > next_y + metrics.viewport_height) {
        next_y = widget_bottom - metrics.viewport_height;
    }

    if (widget_left < next_x) {
        next_x = widget_left;
    } else if (widget_right > next_x + metrics.viewport_width) {
        next_x = widget_right - metrics.viewport_width;
    }

    scroll_to(next_x, next_y);
}

Size ScrollView::preferred_size() const { return {options_.width, options_.height}; }

void ScrollView::layout(Rect bounds) {
    bounds_ = bounds;
    bind_content_dirty_callback();
    layout_content();
}

void ScrollView::layout_content() {
    if (!content_) {
        content_width_ = 0;
        content_height_ = 0;
        return;
    }

    clamp_scroll();
    const Size pref = content_->preferred_size();
    content_width_ = pref.width;
    content_height_ = pref.height;

    const auto metrics = scrollbar_metrics();
    content_->layout({
        0,
        0,
        std::max(content_width_, metrics.viewport_width),
        std::max(content_height_, metrics.viewport_height),
    });
}

void ScrollView::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (!content_) {
        return;
    }

    const auto layout = scrollbar_layout();
    const Rect viewport{{0, 0}, {layout.metrics.viewport_width, layout.metrics.viewport_height}};

    canvas.fill_rect(viewport, ' ', options_.background);

    ctx.with_clip(viewport, [&](PaintContext& clipped_ctx) {
        clipped_ctx.with_clip({{-scroll_x_, -scroll_y_},
                               {std::max(content_width_, layout.metrics.viewport_width),
                                std::max(content_height_, layout.metrics.viewport_height)}},
                              [&](PaintContext& content_ctx) { content_->paint(content_ctx); });
    });

    paint_scrollbars(canvas, options_.scrollbars, layout);
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

    const Point local = to_local(point);
    const auto layout = scrollbar_layout();
    if (local.x >= layout.metrics.viewport_width || local.y >= layout.metrics.viewport_height) {
        return nullptr;
    }

    const Point content_point = to_content_local(point);
    if (Widget* target = content_->hit_test_focusable(content_point)) {
        return target;
    }

    if (Widget* target = content_->hit_test(content_point)) {
        if (target != content_.get() && target->is_focusable()) {
            return target;
        }
    }

    return nullptr;
}

void ScrollView::collect_focusable(std::vector<Widget*>& out) {
    if (content_) {
        content_->collect_focusable(out);
    }
}

void ScrollView::for_each_child(const std::function<void(Widget*)>& visitor) {
    if (content_) {
        visitor(content_.get());
    }
}

bool ScrollView::has_focused_descendant() const { return content_ && content_->has_focused_descendant(); }

bool ScrollView::pointer_active() const { return scrollbar_state_.pointer_active(); }

bool ScrollView::handle_scrollbar_mouse(const MouseEvent& mouse) {
    const Point local = to_local(mouse.position);
    const auto layout = scrollbar_layout();
    return tuinator::handle_scrollbar_mouse(mouse, local, layout, options_.scrollbars.behavior, scrollbar_state_,
                                            scrollbar_actions(), scroll_x_, scroll_y_, content_width_, content_height_);
}

bool ScrollView::handle_event(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (scrollbar_state_.pointer_active() && mouse->action == MouseAction::Release) {
            scrollbar_state_.reset_drag();
            return true;
        }

        if (!bounds_.contains(mouse->position)) {
            return false;
        }

        if (handle_scrollbar_mouse(*mouse)) {
            return true;
        }

        switch (mouse->action) {
        case MouseAction::WheelUp:
        case MouseAction::WheelDown:
        case MouseAction::WheelLeft:
        case MouseAction::WheelRight: return try_scroll(event);
        default: break;
        }

        const Point local = to_local(mouse->position);
        const auto layout = scrollbar_layout();
        if (local.x >= layout.metrics.viewport_width || local.y >= layout.metrics.viewport_height) {
            return false;
        }

        if (!content_) {
            return false;
        }

        MouseEvent adjusted = *mouse;
        adjusted.position = to_content_local(mouse->position);
        if (Widget* target = content_->hit_test(adjusted.position)) {
            if (target != content_.get() && target->handle_event(adjusted)) {
                return true;
            }
        }

        if (Widget* focusable = content_->hit_test_focusable(adjusted.position)) {
            MouseEvent focus_event = adjusted;
            if (!focusable->contains_point(adjusted.position)) {
                focus_event.position = {focusable->bounds().x, focusable->bounds().y};
            }
            return focusable->handle_event(focus_event);
        }

        return content_->handle_event(adjusted);
    }

    if (try_scroll(event)) {
        return true;
    }

    if (!content_) {
        return false;
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

        return handle_scrollbar_wheel(*mouse, options_.scrollbars.behavior, scrollbar_actions());
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key) {
        return false;
    }

    const auto metrics = scrollbar_metrics();
    switch (key->key) {
    case Key::PageUp: scroll_by(0, -metrics.viewport_height); return true;
    case Key::PageDown: scroll_by(0, metrics.viewport_height); return true;
    case Key::Home: scroll_to(0, 0); return true;
    case Key::End: scroll_to(max_scroll_x(), max_scroll_y()); return true;
    case Key::Left: scroll_by(-1, 0); return true;
    case Key::Right: scroll_by(1, 0); return true;
    default: return false;
    }
}

void ScrollView::clamp_scroll() {
    scroll_x_ = std::clamp(scroll_x_, 0, max_scroll_x());
    scroll_y_ = std::clamp(scroll_y_, 0, max_scroll_y());
}

} // namespace tuinator
