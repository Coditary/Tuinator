#include <tuinator/widgets/containers/panel.hpp>

#include <algorithm>
#include <variant>

namespace tuinator {

Panel::Panel(std::string title, Style border_style, Style title_style, std::optional<BorderGlyphs> glyphs)
    : title_(std::move(title)), border_style_(border_style), title_style_(title_style), glyphs_(std::move(glyphs)) {}

void Panel::set_title(std::string title) {
    title_ = std::move(title);
    mark_dirty();
}

void Panel::set_content(std::unique_ptr<Widget> content) {
    content_ = std::move(content);
    if (content_) {
        content_->set_on_dirty(on_dirty_);
    }
    if (content_ && on_layout_) {
        content_->set_on_layout(on_layout_);
    }
    mark_dirty();
}

void Panel::set_on_dirty(std::function<void(Rect)> callback) {
    Widget::set_on_dirty(std::move(callback));
    if (content_) {
        content_->set_on_dirty(on_dirty_);
    }
}

Size Panel::preferred_size() const {
    Size content_size{};
    if (content_) {
        content_size = content_->preferred_size();
    }

    const int title_rows = title_.empty() ? 0 : 1;
    return {
        content_size.width + 2,
        content_size.height + 2 + title_rows,
    };
}

Rect Panel::content_bounds() const {
    const int title_rows = title_.empty() ? 0 : 1;
    return {
        bounds_.x + 1,
        bounds_.y + 1 + title_rows,
        std::max(0, bounds_.width - 2),
        std::max(0, bounds_.height - 2 - title_rows),
    };
}

void Panel::layout(Rect bounds) {
    bounds_ = bounds;
    if (content_) {
        content_->layout(content_bounds());
    }
}

void Panel::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    if (glyphs_.has_value()) {
        canvas.draw_box({{0, 0}, bounds_.size()}, border_style_, *glyphs_);
    } else {
        canvas.draw_box({{0, 0}, bounds_.size()}, border_style_);
    }

    if (!title_.empty()) {
        const std::string label = " " + title_ + " ";
        const int max_width = std::max(0, bounds_.width - 2);
        const std::string clipped = label.substr(0, static_cast<std::size_t>(max_width));
        canvas.draw_text({1, 0}, clipped, title_style_);
    }

    if (!content_) {
        return;
    }

    const Rect inner{
        content_->bounds().x - bounds_.x,
        content_->bounds().y - bounds_.y,
        content_->bounds().width,
        content_->bounds().height,
    };

    ctx.with_clip(inner, [&](PaintContext& child_ctx) { content_->paint(child_ctx); });
}

bool Panel::handle_event(const Event& event) {
    if (!content_) {
        return false;
    }

    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (content_->pointer_active()) {
            return content_->handle_event(event);
        }

        if (!content_bounds().contains(mouse->position)) {
            return false;
        }
        return content_->handle_event(event);
    }

    if (content_->has_focused_descendant() && content_->handle_event(event)) {
        return true;
    }

    return false;
}

Widget* Panel::hit_test(Point point) {
    if (!bounds_.contains(point)) {
        return nullptr;
    }

    if (content_) {
        if (Widget* hit = content_->hit_test(point)) {
            return hit;
        }
    }

    return this;
}

Widget* Panel::hit_test_focusable(Point point) {
    if (!bounds_.contains(point)) {
        return nullptr;
    }

    if (content_) {
        return content_->hit_test_focusable(point);
    }

    return nullptr;
}

bool Panel::has_focused_descendant() const { return content_ && content_->has_focused_descendant(); }

void Panel::collect_focusable(std::vector<Widget*>& out) {
    if (content_) {
        content_->collect_focusable(out);
    }
}

void Panel::for_each_child(const std::function<void(Widget*)>& visitor) {
    if (content_) {
        visitor(content_.get());
    }
}

} // namespace tuinator
