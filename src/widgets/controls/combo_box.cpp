#include <tuinator/core/event.hpp>
#include <tuinator/render/text.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/controls/combo_box.hpp>

#include <algorithm>
#include <string>
#include <variant>

namespace tuinator {

namespace {

Style default_selected_style() {
    Style style{};
    style.foreground = Color::Black;
    style.background = Color::Cyan;
    style.bold = true;
    return style;
}

} // namespace

ComboBox::ComboBox(Style item_style, Style selected_style)
    : item_style_(item_style),
      selected_style_(selected_style.foreground == Color::Default && selected_style.background == Color::Default &&
                              !selected_style.bold && !selected_style.dim && !selected_style.reverse
                          ? default_selected_style()
                          : selected_style) {
    focused_style_ = item_style_;
    focused_style_.reverse = true;
}

void ComboBox::set_items(std::vector<std::string> items) {
    items_ = std::move(items);
    if (items_.empty()) {
        selected_index_ = 0;
    } else {
        selected_index_ = std::clamp(selected_index_, 0, static_cast<int>(items_.size()) - 1);
    }
    mark_dirty();
}

void ComboBox::set_selected_index(int index) { select_index(index, true); }

void ComboBox::set_on_select(std::function<void(int, const std::string&)> callback) {
    on_select_ = std::move(callback);
}

void ComboBox::set_min_width(int min_width) {
    min_width_ = std::max(1, min_width);
    mark_layout_dirty();
}

void ComboBox::apply_stylesheet(const StyleResolver& styles) {
    apply_value_control_stylesheet(*this, *this, styles);
}

void ComboBox::select_index(int index, bool notify) {
    if (items_.empty()) {
        selected_index_ = 0;
        return;
    }

    const int previous = selected_index_;
    selected_index_ = std::clamp(index, 0, static_cast<int>(items_.size()) - 1);
    mark_dirty();

    if (notify && previous != selected_index_ && on_select_) {
        on_select_(selected_index_, items_[static_cast<std::size_t>(selected_index_)]);
    }
}

Size ComboBox::preferred_size() const {
    int width = min_width_;
    for (const std::string& item : items_) {
        width = std::max(width, text_display_width(item) + 6);
    }

    const int list_rows = items_.empty() ? 0 : std::min(static_cast<int>(items_.size()), 6);
    return {width, 1 + list_rows};
}

void ComboBox::layout(Rect bounds) {
    bounds_ = bounds;

    if (open_ && selected_index_ < scroll_y_) {
        scroll_y_ = selected_index_;
    }
    if (open_ && bounds_.height > 1 && selected_index_ >= scroll_y_ + bounds_.height - 1) {
        scroll_y_ = selected_index_ - bounds_.height + 2;
    }
}

void ComboBox::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0 || items_.empty()) {
        return;
    }

    const StyleResolver& styles = ctx.styles();
    const Style item_style = styles.text(*this, item_style_);
    const Style focused_style = styles.focused(*this, focused_style_);
    const Style selected_style = styles.selected(*this, selected_style_);
    paint_bounds_background(ctx, item_style);

    const std::string& current = items_[static_cast<std::size_t>(selected_index_)];
    const std::string header = "[ " + current + (open_ ? " v]" : " >]");
    const Style& header_style = is_focused() ? focused_style : item_style;
    canvas.draw_text({0, 0}, header, header_style);

    if (!open_ || bounds_.height <= 1) {
        return;
    }

    const int list_height = bounds_.height - 1;
    for (int row = 0; row < list_height; ++row) {
        const int index = scroll_y_ + row;
        if (index < 0 || index >= static_cast<int>(items_.size())) {
            continue;
        }

        const bool selected = index == selected_index_;
        const Style& style = selected ? selected_style : item_style;
        const std::string prefix = selected ? "> " : "  ";
        const std::string& item = items_[static_cast<std::size_t>(index)];
        const int max_columns = std::max(0, bounds_.width - static_cast<int>(prefix.size()));
        const std::size_t bytes = text_byte_length_for_width(item, max_columns);
        canvas.draw_text({0, row + 1}, prefix + item.substr(0, bytes), style);
    }
}

bool ComboBox::handle_event(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (mouse->action != MouseAction::Click && mouse->action != MouseAction::Release) {
            return false;
        }

        if (!contains_point(mouse->position)) {
            if (open_) {
                close();
            }
            return false;
        }

        const Point local{mouse->position.x - bounds_.x, mouse->position.y - bounds_.y};
        if (local.y == 0) {
            open_ ? close() : open();
            return true;
        }

        if (open_ && local.y > 0) {
            const int index = scroll_y_ + local.y - 1;
            if (index >= 0 && index < static_cast<int>(items_.size())) {
                select_index(index, true);
                close();
            }
            return true;
        }

        return false;
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key || !is_focused() || items_.empty()) {
        return false;
    }

    if (key->key == Key::Escape && open_) {
        close();
        return true;
    }

    if (key->key == Key::Enter || key->character == ' ' || key->character == '\r') {
        if (open_) {
            close();
        } else {
            open();
        }
        return true;
    }

    if (!open_) {
        if (key->key == Key::Up) {
            select_index(selected_index_ - 1, true);
            return true;
        }
        if (key->key == Key::Down) {
            select_index(selected_index_ + 1, true);
            return true;
        }
        return false;
    }

    switch (key->key) {
    case Key::Up: select_index(selected_index_ - 1, true); return true;
    case Key::Down: select_index(selected_index_ + 1, true); return true;
    case Key::Home: select_index(0, true); return true;
    case Key::End: select_index(static_cast<int>(items_.size()) - 1, true); return true;
    default: break;
    }

    return false;
}

void ComboBox::close() {
    if (!open_) {
        return;
    }

    open_ = false;
    mark_dirty();
}

void ComboBox::open() {
    if (open_ || items_.empty()) {
        return;
    }

    open_ = true;
    scroll_y_ = std::max(0, selected_index_);
    mark_dirty();
}

} // namespace tuinator
