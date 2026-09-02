#include <tuinator/layout/box.hpp>

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

void layout_vertical_children(
    const Rect& bounds,
    int gap,
    int padding,
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

    const int remaining =
        content_height - fixed_height - total_gap(gap, rows.size());
    const int extra_per_flex =
        flex_total > 0 ? std::max(0, remaining) / flex_total : 0;
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

void layout_horizontal_children(
    const Rect& bounds,
    int gap,
    int padding,
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

    const int remaining =
        content_width - fixed_width - total_gap(gap, columns.size());
    const int extra_per_flex =
        flex_total > 0 ? std::max(0, remaining) / flex_total : 0;
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

VBox::VBox(BoxOptions options)
    : gap_(options.gap), padding_(options.padding) {}

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

void VBox::paint(Canvas& canvas) const {
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

bool VBox::handle_event(const Event& event) {
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

HBox::HBox(BoxOptions options)
    : gap_(options.gap), padding_(options.padding) {}

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

void HBox::paint(Canvas& canvas) const {
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

bool HBox::handle_event(const Event& event) {
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

} // namespace tuinator
