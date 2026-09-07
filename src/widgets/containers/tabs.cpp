#include <tuinator/core/event.hpp>
#include <tuinator/render/text.hpp>
#include <tuinator/widgets/containers/tabs.hpp>

#include <algorithm>
#include <string>
#include <variant>

namespace tuinator {

Tabs::Tabs(TabsOptions options) : options_(options) {}

void Tabs::set_on_dirty(std::function<void(Rect)> callback) {
    Widget::set_on_dirty(std::move(callback));
    for (TabEntry& tab : tabs_) {
        if (tab.content) {
            tab.content->set_on_dirty(on_dirty_);
        }
    }
}

void Tabs::add_tab(std::string title, std::unique_ptr<Widget> content) {
    if (content && on_dirty_) {
        content->set_on_dirty(on_dirty_);
    }

    tabs_.push_back(TabEntry{std::move(title), std::move(content)});
    if (selected_index_ >= static_cast<int>(tabs_.size())) {
        selected_index_ = static_cast<int>(tabs_.size()) - 1;
    }
    mark_dirty();
}

void Tabs::set_selected_index(int index) {
    if (tabs_.empty()) {
        selected_index_ = 0;
        return;
    }

    const int next = std::clamp(index, 0, static_cast<int>(tabs_.size()) - 1);
    if (next == selected_index_) {
        return;
    }

    selected_index_ = next;
    mark_dirty();
}

Widget* Tabs::active_content() const {
    if (tabs_.empty() || selected_index_ < 0 || selected_index_ >= static_cast<int>(tabs_.size())) {
        return nullptr;
    }
    return tabs_[static_cast<std::size_t>(selected_index_)].content.get();
}

Size Tabs::preferred_size() const {
    int tab_bar_width = 0;
    for (const TabEntry& tab : tabs_) {
        const std::string label = " " + tab.title + " ";
        tab_bar_width += text_display_width(label) + 1;
    }

    Size content_size{};
    if (!tabs_.empty() && tabs_[static_cast<std::size_t>(selected_index_)].content) {
        content_size = tabs_[static_cast<std::size_t>(selected_index_)].content->preferred_size();
    }

    return {
        std::max(tab_bar_width, content_size.width),
        1 + content_size.height,
    };
}

void Tabs::layout_active_content(const Rect& area) {
    if (tabs_.empty()) {
        return;
    }

    Widget* content = tabs_[static_cast<std::size_t>(selected_index_)].content.get();
    if (content != nullptr) {
        content->layout(area);
    }
}

void Tabs::layout(Rect bounds) {
    bounds_ = bounds;

    const Rect content_area{
        bounds.x,
        bounds.y + 1,
        bounds.width,
        std::max(0, bounds.height - 1),
    };
    layout_active_content(content_area);
}

void Tabs::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    Style normal = options_.tab_style;
    Style selected = options_.selected_tab_style;
    if (selected.foreground == Color::Default) {
        selected = normal;
        selected.background = Color::Cyan;
        selected.foreground = Color::Black;
        selected.bold = true;
    }

    int x = 0;
    for (int i = 0; i < static_cast<int>(tabs_.size()); ++i) {
        const std::string label = " " + tabs_[static_cast<std::size_t>(i)].title + " ";
        Style style = i == selected_index_ ? selected : normal;
        if (is_focused() && i == selected_index_) {
            style.bold = true;
        }
        if (x >= bounds_.width) {
            break;
        }
        canvas.draw_text({x, 0}, label, style);
        x += text_display_width(label) + 1;
    }

    if (is_focused()) {
        Style focused_marker;
        focused_marker.foreground = Color::Yellow;
        focused_marker.bold = true;
        canvas.draw_text({x, 0}, "*", focused_marker);
    }

    if (tabs_.empty()) {
        return;
    }

    const Widget* content = tabs_[static_cast<std::size_t>(selected_index_)].content.get();
    if (content == nullptr) {
        return;
    }

    const Rect local{
        content->bounds().x - bounds_.x,
        content->bounds().y - bounds_.y,
        content->bounds().width,
        content->bounds().height,
    };

    ctx.with_clip(local, [&](PaintContext& child_ctx) { content->paint(child_ctx); });
}

int Tabs::tab_at_position(Point position) const {
    if (position.y != bounds_.y || tabs_.empty()) {
        return -1;
    }

    int x = bounds_.x;
    for (int i = 0; i < static_cast<int>(tabs_.size()); ++i) {
        const std::string label = " " + tabs_[static_cast<std::size_t>(i)].title + " ";
        const int width = text_display_width(label) + 1;
        if (position.x >= x && position.x < x + width) {
            return i;
        }
        x += width;
    }

    return -1;
}

bool Tabs::handle_tab_switch(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (mouse->action != MouseAction::Click && mouse->action != MouseAction::Release) {
            return false;
        }

        const int tab = tab_at_position(mouse->position);
        if (tab < 0) {
            return false;
        }

        set_selected_index(tab);
        return true;
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key) {
        return false;
    }

    if (key->key == Key::Left) {
        if (!is_focused()) {
            return false;
        }
        set_selected_index(selected_index_ - 1);
        return true;
    }

    if (key->key == Key::Right) {
        if (!is_focused()) {
            return false;
        }
        set_selected_index(selected_index_ + 1);
        return true;
    }

    if (key->character >= '1' && key->character <= '9') {
        const int index = key->character - '1';
        if (index < static_cast<int>(tabs_.size())) {
            set_selected_index(index);
            return true;
        }
    }

    return false;
}

bool Tabs::handle_event(const Event& event) {
    if (handle_tab_switch(event)) {
        return true;
    }

    if (tabs_.empty()) {
        return false;
    }

    Widget* content = tabs_[static_cast<std::size_t>(selected_index_)].content.get();
    if (content == nullptr) {
        return false;
    }

    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (!bounds_.contains(mouse->position)) {
            return false;
        }

        if (tab_at_position(mouse->position) >= 0) {
            return false;
        }

        return content->handle_event(event);
    }

    if (content->has_focused_descendant() && content->handle_event(event)) {
        return true;
    }

    if (is_focused() && content->handle_event(event)) {
        return true;
    }

    return false;
}

bool Tabs::has_focused_descendant() const {
    if (focused_) {
        return true;
    }

    if (tabs_.empty()) {
        return false;
    }

    const Widget* content = tabs_[static_cast<std::size_t>(selected_index_)].content.get();
    return content != nullptr && content->has_focused_descendant();
}

void Tabs::collect_focusable(std::vector<Widget*>& out) {
    out.push_back(this);

    if (tabs_.empty()) {
        return;
    }

    Widget* content = tabs_[static_cast<std::size_t>(selected_index_)].content.get();
    if (content != nullptr) {
        content->collect_focusable(out);
    }
}

void Tabs::for_each_child(const std::function<void(Widget*)>& visitor) {
    if (Widget* content = active_content()) {
        visitor(content);
    }
}

} // namespace tuinator
