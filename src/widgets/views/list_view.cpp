#include <tuinator/widgets/views/list_view.hpp>

#include <tuinator/core/event.hpp>
#include <tuinator/render/text.hpp>

#include <algorithm>
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

ListView::ListView(Style item_style, Style selected_style)
    : item_style_(item_style),
      selected_style_(selected_style.foreground == Color::Default
                          && selected_style.background == Color::Default
                          && !selected_style.bold
                          && !selected_style.dim
                          && !selected_style.reverse
                      ? default_selected_style()
                      : selected_style) {}

void ListView::set_items(std::vector<std::string> items) {
    items_ = std::move(items);
    clamp_selection();
    mark_dirty();
}

void ListView::add_item(std::string item) {
    items_.push_back(std::move(item));
    clamp_selection();
    mark_dirty();
}

void ListView::set_selected_index(int index) {
    const int previous = selected_index_;
    selected_index_ = index;
    clamp_selection();
    ensure_selected_visible();

    if (previous != selected_index_ && on_select_ && !items_.empty()) {
        on_select_(selected_index_, items_[static_cast<std::size_t>(selected_index_)]);
    }

    mark_dirty();
}

void ListView::set_on_select(std::function<void(int, const std::string&)> callback) {
    on_select_ = std::move(callback);
}

void ListView::set_on_activate(std::function<void(int, const std::string&)> callback) {
    on_activate_ = std::move(callback);
}

Size ListView::preferred_size() const {
    int width = 8;
    for (const std::string& item : items_) {
        width = std::max(width, text_display_width(item) + 2);
    }

    const int height = std::max(1, static_cast<int>(items_.size()));
    return {width, height};
}

void ListView::layout(Rect bounds) {
    bounds_ = bounds;
    ensure_selected_visible();
}

void ListView::paint(Canvas& canvas) const {
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    const int capacity = bounds_.height;
    for (int row = 0; row < capacity; ++row) {
        const int index = scroll_y_ + row;
        if (index < 0 || index >= static_cast<int>(items_.size())) {
            continue;
        }

        const bool selected = index == selected_index_;
        const Style& style = selected ? selected_style_ : item_style_;
        const std::string prefix = selected ? "> " : "  ";
        const std::string& item = items_[static_cast<std::size_t>(index)];

        const int max_columns = std::max(0, bounds_.width - static_cast<int>(prefix.size()));
        const std::size_t bytes = text_byte_length_for_width(item, max_columns);
        canvas.draw_text({0, row}, prefix + item.substr(0, bytes), style);
    }
}

bool ListView::handle_event(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (mouse->action != MouseAction::Click && mouse->action != MouseAction::Release) {
            return false;
        }

        if (!contains_point(mouse->position)) {
            return false;
        }

        const Point local{
            mouse->position.x - bounds_.x,
            mouse->position.y - bounds_.y,
        };
        const int row = row_at(local);
        if (row >= 0) {
            set_selected_index(row);
            return true;
        }

        return false;
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key || !is_focused() || items_.empty()) {
        return false;
    }

    switch (key->key) {
    case Key::Up:
        set_selected_index(selected_index_ - 1);
        return true;
    case Key::Down:
        set_selected_index(selected_index_ + 1);
        return true;
    case Key::Home:
        set_selected_index(0);
        return true;
    case Key::End:
        set_selected_index(static_cast<int>(items_.size()) - 1);
        return true;
    case Key::Enter:
        if (on_activate_ && selected_index_ >= 0
            && selected_index_ < static_cast<int>(items_.size())) {
            on_activate_(selected_index_, items_[static_cast<std::size_t>(selected_index_)]);
        }
        return true;
    default:
        break;
    }

    return false;
}

void ListView::clamp_selection() {
    if (items_.empty()) {
        selected_index_ = 0;
        scroll_y_ = 0;
        return;
    }

    selected_index_ = std::clamp(selected_index_, 0, static_cast<int>(items_.size()) - 1);
}

void ListView::ensure_selected_visible() {
    if (bounds_.height <= 0) {
        return;
    }

    if (selected_index_ < scroll_y_) {
        scroll_y_ = selected_index_;
    }

    if (selected_index_ >= scroll_y_ + bounds_.height) {
        scroll_y_ = selected_index_ - bounds_.height + 1;
    }

    const int max_scroll = std::max(0, static_cast<int>(items_.size()) - bounds_.height);
    scroll_y_ = std::clamp(scroll_y_, 0, max_scroll);
}

int ListView::row_at(Point local) const {
    if (local.y < 0 || local.y >= bounds_.height) {
        return -1;
    }

    return scroll_y_ + local.y;
}

} // namespace tuinator
