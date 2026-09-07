#include <tuinator/layout/grid.hpp>

#include <algorithm>
#include <variant>

namespace tuinator {

Grid::Grid(GridOptions options)
    : columns_(std::max(1, options.columns)), gap_(options.gap), padding_(options.padding) {}

Size Grid::preferred_size() const {
    if (children_.empty()) {
        return {padding_ * 2, padding_ * 2};
    }

    const int rows = static_cast<int>((children_.size() + static_cast<std::size_t>(columns_) - 1) /
                                      static_cast<std::size_t>(columns_));

    std::vector<int> column_widths(static_cast<std::size_t>(columns_), 0);
    std::vector<int> row_heights(static_cast<std::size_t>(rows), 0);

    for (std::size_t i = 0; i < children_.size(); ++i) {
        const Size child_size = children_[i]->preferred_size();
        const int column = static_cast<int>(i) % columns_;
        const int row = static_cast<int>(i) / columns_;
        column_widths[static_cast<std::size_t>(column)] =
            std::max(column_widths[static_cast<std::size_t>(column)], child_size.width);
        row_heights[static_cast<std::size_t>(row)] =
            std::max(row_heights[static_cast<std::size_t>(row)], child_size.height);
    }

    const int padding_total = padding_ * 2;
    int width = padding_total;
    int height = padding_total;
    for (int column = 0; column < columns_; ++column) {
        width += column_widths[static_cast<std::size_t>(column)];
        if (column + 1 < columns_) {
            width += gap_;
        }
    }
    for (int row = 0; row < rows; ++row) {
        height += row_heights[static_cast<std::size_t>(row)];
        if (row + 1 < rows) {
            height += gap_;
        }
    }

    return {width, height};
}

void Grid::layout(Rect bounds) {
    bounds_ = bounds;

    if (children_.empty()) {
        return;
    }

    const int rows = static_cast<int>((children_.size() + static_cast<std::size_t>(columns_) - 1) /
                                      static_cast<std::size_t>(columns_));
    const int content_width = std::max(0, bounds.width - padding_ * 2);
    const int content_height = std::max(0, bounds.height - padding_ * 2);

    const int column_width = columns_ > 0 ? std::max(0, (content_width - gap_ * (columns_ - 1)) / columns_) : 0;
    const int row_height = rows > 0 ? std::max(0, (content_height - gap_ * (rows - 1)) / rows) : 0;

    for (std::size_t i = 0; i < children_.size(); ++i) {
        const int column = static_cast<int>(i) % columns_;
        const int row = static_cast<int>(i) / columns_;
        const int x = bounds.x + padding_ + column * (column_width + gap_);
        const int y = bounds.y + padding_ + row * (row_height + gap_);
        children_[i]->layout({x, y, column_width, row_height});
    }
}

void Grid::paint(PaintContext& ctx) const {
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

bool Grid::handle_event(const Event& event) {
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
