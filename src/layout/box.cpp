#include <tuinator/layout/box.hpp>
#include <tuinator/widgets/capabilities.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>

#include <algorithm>
#include <numeric>
#include <variant>

namespace tuinator {

namespace {

int total_gap(int gap, std::size_t child_count) {
    if (child_count <= 1) {
        return 0;
    }
    return gap * static_cast<int>(child_count - 1);
}

void layout_vertical_children(const Rect& bounds, int gap, int padding,
                              const std::vector<std::unique_ptr<Widget>>& children) {
    const int content_width = std::max(0, bounds.width - padding * 2);
    const int content_height = std::max(0, bounds.height - padding * 2);

    struct Row {
        Widget* widget = nullptr;
        Size preferred{};
        int flex = 0;
    };

    std::vector<Row> rows;
    rows.reserve(children.size());
    for (const auto& child : children) {
        rows.push_back({child.get(), child->preferred_size(), child->flex()});
    }

    int fixed_height = 0;
    int flex_total = 0;
    for (const Row& row : rows) {
        if (row.flex > 0) {
            flex_total += row.flex;
        } else {
            fixed_height += row.preferred.height;
        }
    }

    const int remaining = content_height - fixed_height - total_gap(gap, rows.size());
    const int extra_per_flex = flex_total > 0 ? std::max(0, remaining) / flex_total : 0;
    int leftover = flex_total > 0 ? std::max(0, remaining) % flex_total : 0;

    int y = bounds.y + padding;
    for (const Row& row : rows) {
        int height = row.preferred.height;
        if (row.flex > 0) {
            height = std::max(row.preferred.height, extra_per_flex * row.flex);
            if (leftover > 0) {
                height += 1;
                leftover -= 1;
            }
        }

        row.widget->layout({bounds.x + padding, y, content_width, height});
        y += height + gap;
    }
}

void layout_horizontal_children(const Rect& bounds, int gap, int padding,
                                const std::vector<std::unique_ptr<Widget>>& children) {
    const int content_width = std::max(0, bounds.width - padding * 2);
    const int content_height = std::max(0, bounds.height - padding * 2);

    struct Column {
        Widget* widget = nullptr;
        Size preferred{};
        int flex = 0;
    };

    std::vector<Column> columns;
    columns.reserve(children.size());
    for (const auto& child : children) {
        columns.push_back({child.get(), child->preferred_size(), child->flex()});
    }

    int fixed_width = 0;
    int flex_total = 0;
    for (const Column& column : columns) {
        if (column.flex > 0) {
            flex_total += column.flex;
        } else {
            fixed_width += column.preferred.width;
        }
    }

    const int remaining = content_width - fixed_width - total_gap(gap, columns.size());
    const int extra_per_flex = flex_total > 0 ? std::max(0, remaining) / flex_total : 0;
    int leftover = flex_total > 0 ? std::max(0, remaining) % flex_total : 0;

    int x = bounds.x + padding;
    const int content_right = bounds.x + padding + content_width;
    for (const Column& column : columns) {
        int width = column.preferred.width;
        if (column.flex > 0) {
            width = std::max(column.preferred.width, extra_per_flex * column.flex);
            if (leftover > 0) {
                width += 1;
                leftover -= 1;
            }
        }

        width = std::max(0, std::min(width, content_right - x));
        column.widget->layout({x, bounds.y + padding, width, content_height});
        x += width;
        if (x < content_right) {
            x += std::min(gap, content_right - x);
        }
    }
}

} // namespace

VBox::VBox(BoxOptions options) : gap_(options.gap), padding_(options.padding) {}

void VBox::set_gap(int gap) {
    gap_ = std::max(0, gap);
    mark_layout_dirty();
}

void VBox::set_padding(int padding) {
    padding_ = std::max(0, padding);
    mark_layout_dirty();
}

void VBox::apply_stylesheet(const StyleResolver& styles) { apply_layout_box_stylesheet(*this, *this, styles); }

Size VBox::preferred_size() const {
    int width = 0;
    int height = padding_ * 2;

    for (std::size_t i = 0; i < children_.size(); ++i) {
        const Size child_size = children_[i]->preferred_size();
        width = std::max(width, child_size.width);
        height += child_size.height;

        if (i + 1 < children_.size()) {
            height += gap_;
        }
    }

    return {width + padding_ * 2, height};
}

void VBox::layout(Rect bounds) {
    bounds_ = bounds;
    layout_vertical_children(bounds, gap_, padding_, children_);
}

void VBox::paint(PaintContext& ctx) const {
    paint_bounds_background(ctx);

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

bool VBox::handle_event(const Event& event) {
    if (std::holds_alternative<KeyPress>(event)) {
        if (dispatch_keyboard_capture(this, event)) {
            return true;
        }

        for (auto& child : children_) {
            if (child->has_focused_descendant() && child->handle_event(event)) {
                return true;
            }
        }
        for (auto& child : children_) {
            if (child->is_focused() && child->handle_event(event)) {
                return true;
            }
        }
        return false;
    }

    if (std::holds_alternative<MouseEvent>(event)) {
        return Widget::handle_event(event);
    }

    for (auto& child : children_) {
        if (child->has_focused_descendant() && child->handle_event(event)) {
            return true;
        }
    }

    return false;
}

HBox::HBox(BoxOptions options) : gap_(options.gap), padding_(options.padding) {}

void HBox::set_gap(int gap) {
    gap_ = std::max(0, gap);
    mark_layout_dirty();
}

void HBox::set_padding(int padding) {
    padding_ = std::max(0, padding);
    mark_layout_dirty();
}

void HBox::apply_stylesheet(const StyleResolver& styles) { apply_layout_box_stylesheet(*this, *this, styles); }

Size HBox::preferred_size() const {
    int width = padding_ * 2;
    int height = 0;

    for (std::size_t i = 0; i < children_.size(); ++i) {
        const Size child_size = children_[i]->preferred_size();
        width += child_size.width;
        height = std::max(height, child_size.height);

        if (i + 1 < children_.size()) {
            width += gap_;
        }
    }

    return {width, height + padding_ * 2};
}

void HBox::layout(Rect bounds) {
    bounds_ = bounds;
    layout_horizontal_children(bounds, gap_, padding_, children_);
}

void HBox::paint(PaintContext& ctx) const {
    paint_bounds_background(ctx);

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

Widget* HBox::focusable_child_at(Point point) const {
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if (Widget* hit = (*it)->hit_test_focusable(point)) {
            return hit;
        }
    }

    if (!bounds_.contains(point)) {
        return nullptr;
    }

    for (const auto& child : children_) {
        if (child->is_focusable()) {
            return child.get();
        }
    }

    return nullptr;
}

Widget* HBox::hit_test(Point point) {
    if (!bounds_.contains(point)) {
        return nullptr;
    }

    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if (Widget* hit = (*it)->hit_test(point)) {
            if (hit->is_focusable()) {
                return hit;
            }
            if (focusable_child_at(point) != nullptr) {
                return this;
            }
            return hit;
        }
    }

    return this;
}

Widget* HBox::hit_test_focusable(Point point) { return focusable_child_at(point); }

bool HBox::handle_event(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (!bounds_.contains(mouse->position)) {
            return false;
        }

        switch (mouse->action) {
        case MouseAction::WheelUp:
        case MouseAction::WheelDown:
        case MouseAction::WheelLeft:
        case MouseAction::WheelRight:
            for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
                if ((*it)->bounds().contains(mouse->position)) {
                    return (*it)->handle_event(event);
                }
            }
            return false;
        default: break;
        }

        if (Widget* target = focusable_child_at(mouse->position)) {
            MouseEvent adjusted = *mouse;
            if (!target->contains_point(mouse->position)) {
                adjusted.position = {target->bounds().x, target->bounds().y};
            }
            return target->handle_event(adjusted);
        }

        return false;
    }

    if (dispatch_keyboard_capture(this, event)) {
        return true;
    }

    for (auto& child : children_) {
        if (child->has_focused_descendant() && child->handle_event(event)) {
            return true;
        }
    }

    return false;
}

} // namespace tuinator
