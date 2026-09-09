#include <tuinator/render/text.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/containers/panel.hpp>

#include <algorithm>
#include <variant>

namespace tuinator {

Panel::Panel(std::string title, Style border_style, Style title_style, std::optional<BorderGlyphs> glyphs)
    : title_(std::move(title)), border_style_(std::move(border_style)), title_style_(std::move(title_style)),
      glyphs_(std::move(glyphs)) {}

void Panel::set_title(std::string title) {
    title_ = std::move(title);
    mark_dirty();
}

void Panel::apply_stylesheet(const StyleResolver& styles) {
    apply_bordered_pane_stylesheet(*this, *this, styles);
    mark_dirty();
}

void Panel::set_border_edges(BorderEdges edges) {
    if (configured_border_edges_.top == edges.top && configured_border_edges_.right == edges.right &&
        configured_border_edges_.bottom == edges.bottom && configured_border_edges_.left == edges.left &&
        border_edges_.top == edges.top && border_edges_.right == edges.right &&
        border_edges_.bottom == edges.bottom && border_edges_.left == edges.left) {
        return;
    }

    configured_border_edges_ = edges;
    border_edges_ = edges;
    mark_dirty();
}

void Panel::reset_border_edges() {
    if (border_edges_.top == configured_border_edges_.top && border_edges_.right == configured_border_edges_.right &&
        border_edges_.bottom == configured_border_edges_.bottom &&
        border_edges_.left == configured_border_edges_.left) {
        return;
    }

    border_edges_ = configured_border_edges_;
    mark_dirty();
}

void Panel::set_content(std::unique_ptr<Widget> content) {
    content_ = std::move(content);
    if (content_) {
        attach_child_widget(content_.get());
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
    const int left_inset = border_edges_.left ? 1 : 0;
    const int right_inset = border_edges_.right ? 1 : 0;
    const int top_inset = border_edges_.top ? 1 : 0;
    const int bottom_inset = border_edges_.bottom ? 1 : 0;

    return {
        bounds_.x + left_inset,
        bounds_.y + top_inset + title_rows,
        std::max(0, bounds_.width - left_inset - right_inset),
        std::max(0, bounds_.height - top_inset - bottom_inset - title_rows),
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

    const Rect rect{{0, 0}, bounds_.size()};
    const StyleResolver& styles = ctx.styles();
    const BorderGlyphs glyphs = glyphs_.has_value() ? *glyphs_ : styles.border_glyphs(*this);
    const Style border_style = styles.border(*this, border_style_);
    const Style title_style = styles.title(*this, title_style_);

    const Rect content = content_bounds();
    const Rect local_content{
        content.x - bounds_.x,
        content.y - bounds_.y,
        content.width,
        content.height,
    };
    if (local_content.width > 0 && local_content.height > 0) {
        canvas.fill_rect(local_content, ' ', styles.text(*this, border_style_));
    }
    const int left = rect.x;
    const int right = rect.right() - 1;
    const int top = rect.y;
    const int bottom = rect.bottom() - 1;

    if (border_edges_.top && border_edges_.left) {
        canvas.draw_text({left, top}, glyphs.top_left, border_style);
    }
    if (border_edges_.top && border_edges_.right) {
        canvas.draw_text({right, top}, glyphs.top_right, border_style);
    }
    if (border_edges_.bottom && border_edges_.left) {
        canvas.draw_text({left, bottom}, glyphs.bottom_left, border_style);
    }
    if (border_edges_.bottom && border_edges_.right) {
        canvas.draw_text({right, bottom}, glyphs.bottom_right, border_style);
    }

    if (border_edges_.top) {
        const int start = border_edges_.left ? left + 1 : left;
        const int end = border_edges_.right ? right - 1 : right;
        for (int x = start; x <= end; ++x) {
            canvas.draw_text({x, top}, glyphs.horizontal, border_style);
        }
    }

    if (border_edges_.bottom) {
        const int start = border_edges_.left ? left + 1 : left;
        const int end = border_edges_.right ? right - 1 : right;
        for (int x = start; x <= end; ++x) {
            canvas.draw_text({x, bottom}, glyphs.horizontal, border_style);
        }
    }

    if (border_edges_.left) {
        const int start = border_edges_.top ? top + 1 : top;
        const int end = border_edges_.bottom ? bottom - 1 : bottom;
        for (int y = start; y <= end; ++y) {
            canvas.draw_text({left, y}, glyphs.vertical, border_style);
        }
    }

    if (border_edges_.right) {
        const int start = border_edges_.top ? top + 1 : top;
        const int end = border_edges_.bottom ? bottom - 1 : bottom;
        for (int y = start; y <= end; ++y) {
            canvas.draw_text({right, y}, glyphs.vertical, border_style);
        }
    }

    if (!title_.empty()) {
        const std::string label = " " + title_ + " ";
        const int title_x = border_edges_.left ? 1 : 0;
        const int title_y = top;
        const int max_width = std::max(0, bounds_.width - title_x);
        const std::size_t bytes = text_byte_length_for_width(label, max_width);
        canvas.draw_text({title_x, title_y}, label.substr(0, bytes), title_style);
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
