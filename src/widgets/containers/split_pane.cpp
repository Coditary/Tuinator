#include <tuinator/widgets/containers/split_pane.hpp>

#include <algorithm>
#include <string>
#include <variant>

namespace tuinator {

SplitPane::SplitPane(std::unique_ptr<Widget> first,
                     std::unique_ptr<Widget> second,
                     SplitPaneOptions options)
    : first_(std::move(first)),
      second_(std::move(second)),
      options_(options) {}

Size SplitPane::preferred_size() const {
    const Size first_size = first_ ? first_->preferred_size() : Size{};
    const Size second_size = second_ ? second_->preferred_size() : Size{};

    if (options_.orientation == SplitOrientation::Horizontal) {
        return {
            first_size.width + 1 + second_size.width,
            std::max(first_size.height, second_size.height),
        };
    }

    return {
        std::max(first_size.width, second_size.width),
        first_size.height + 1 + second_size.height,
    };
}

void SplitPane::layout(Rect bounds) {
    bounds_ = bounds;

    if (options_.orientation == SplitOrientation::Horizontal) {
        const int divider = 1;
        const int first_width =
            std::clamp(options_.first_size, 0, std::max(0, bounds.width - divider));
        const int second_width = std::max(0, bounds.width - first_width - divider);

        if (first_) {
            first_->layout({bounds.x, bounds.y, first_width, bounds.height});
        }
        if (second_) {
            second_->layout({bounds.x + first_width + divider, bounds.y, second_width, bounds.height});
        }
        return;
    }

    const int divider = 1;
    const int first_height =
        std::clamp(options_.first_size, 0, std::max(0, bounds.height - divider));
    const int second_height = std::max(0, bounds.height - first_height - divider);

    if (first_) {
        first_->layout({bounds.x, bounds.y, bounds.width, first_height});
    }
    if (second_) {
        second_->layout({bounds.x, bounds.y + first_height + divider, bounds.width, second_height});
    }
}

void SplitPane::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    auto paint_child = [&](const Widget* child) {
        if (!child) {
            return;
        }

        const Rect local{
            child->bounds().x - bounds_.x,
            child->bounds().y - bounds_.y,
            child->bounds().width,
            child->bounds().height,
        };

        ctx.with_clip(local, [&](PaintContext& child_ctx) { child->paint(child_ctx); });
    };

    paint_child(first_.get());
    paint_child(second_.get());

    Style divider = options_.divider_style;
    if (divider.foreground == Color::Default) {
        divider.foreground = Color::Cyan;
    }

    if (options_.orientation == SplitOrientation::Horizontal) {
        const int x = first_ ? first_->bounds().width : options_.first_size;
        canvas.draw_vline(x, 0, bounds_.height, divider);
        return;
    }

    const int y = first_ ? first_->bounds().height : options_.first_size;
    canvas.draw_hline(0, y, bounds_.width, divider);
}

bool SplitPane::handle_event(const Event& event) {
    for (Widget* pane : {first_.get(), second_.get()}) {
        if (pane != nullptr && pane->has_focused_descendant() && pane->handle_event(event)) {
            return true;
        }
    }

    if (std::holds_alternative<MouseEvent>(event)) {
        const auto* mouse = std::get_if<MouseEvent>(&event);
        if (!mouse) {
            return false;
        }

        for (Widget* pane : {first_.get(), second_.get()}) {
            if (pane != nullptr && pane->bounds().contains(mouse->position)) {
                return pane->handle_event(event);
            }
        }
    }

    return false;
}

bool SplitPane::has_focused_descendant() const {
    return (first_ && first_->has_focused_descendant())
        || (second_ && second_->has_focused_descendant());
}

void SplitPane::collect_focusable(std::vector<Widget*>& out) {
    if (first_) {
        first_->collect_focusable(out);
    }
    if (second_) {
        second_->collect_focusable(out);
    }
}

void SplitPane::for_each_child(const std::function<void(Widget*)>& visitor) {
    if (first_) {
        visitor(first_.get());
    }
    if (second_) {
        visitor(second_.get());
    }
}

} // namespace tuinator
