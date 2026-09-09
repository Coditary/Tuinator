#include <tuinator/core/event.hpp>
#include <tuinator/render/text.hpp>
#include <tuinator/widgets/views/tree_view.hpp>

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

TreeView::TreeView(Style item_style, Style selected_style)
    : item_style_(item_style),
      selected_style_(selected_style.foreground == Color::Default && selected_style.background == Color::Default &&
                              !selected_style.bold && !selected_style.dim && !selected_style.reverse
                          ? default_selected_style()
                          : selected_style) {}

void TreeView::set_root(TreeNode root) {
    root_ = std::move(root);
    rebuild_visible();
    selected_index_ = std::clamp(selected_index_, 0, std::max(0, static_cast<int>(visible_.size()) - 1));
    mark_dirty();
}

void TreeView::set_on_select(std::function<void(const std::string&)> callback) { on_select_ = std::move(callback); }

void TreeView::set_selected_index(int index) {
    if (visible_.empty()) {
        selected_index_ = 0;
        return;
    }

    selected_index_ = std::clamp(index, 0, static_cast<int>(visible_.size()) - 1);
    mark_dirty();
}

void TreeView::apply_stylesheet(const StyleResolver& styles) {
    Widget::apply_stylesheet(styles);
}

void TreeView::append_visible(TreeNode& node, const std::string& path, int depth) {
    visible_.push_back({&node, path, depth});

    if (!node.expanded) {
        return;
    }

    for (TreeNode& child : node.children) {
        const std::string child_path = path.empty() ? child.label : path + "/" + child.label;
        append_visible(child, child_path, depth + 1);
    }
}

void TreeView::rebuild_visible() {
    visible_.clear();
    if (!root_.label.empty() || !root_.children.empty()) {
        append_visible(root_, root_.label, 0);
    }
}

Size TreeView::preferred_size() const {
    int width = 12;
    for (const VisibleNode& entry : visible_) {
        const std::string indent(static_cast<std::size_t>(entry.depth * 2), ' ');
        width = std::max(width, text_display_width(indent + entry.node->label) + 6);
    }

    return {width, std::max(1, static_cast<int>(visible_.size()))};
}

void TreeView::layout(Rect bounds) {
    bounds_ = bounds;

    if (selected_index_ < scroll_y_) {
        scroll_y_ = selected_index_;
    }
    if (bounds_.height > 0 && selected_index_ >= scroll_y_ + bounds_.height) {
        scroll_y_ = selected_index_ - bounds_.height + 1;
    }

    const int max_scroll = std::max(0, static_cast<int>(visible_.size()) - bounds_.height);
    scroll_y_ = std::clamp(scroll_y_, 0, max_scroll);
}

void TreeView::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    const StyleResolver& styles = ctx.styles();
    const Style item_style = styles.text(*this, item_style_);
    const Style selected_style = styles.selected(*this, selected_style_);
    paint_bounds_background(ctx, item_style);

    for (int row = 0; row < bounds_.height; ++row) {
        const int index = scroll_y_ + row;
        if (index < 0 || index >= static_cast<int>(visible_.size())) {
            continue;
        }

        const VisibleNode& entry = visible_[static_cast<std::size_t>(index)];
        const bool selected = index == selected_index_;
        const bool has_children = !entry.node->children.empty();
        const std::string marker = has_children ? (entry.node->expanded ? "[-] " : "[+] ") : "    ";
        const std::string indent(static_cast<std::size_t>(entry.depth * 2), ' ');
        const std::string line = indent + marker + entry.node->label;

        const Style& style = selected ? selected_style : item_style;
        const int max_columns = std::max(0, bounds_.width);
        const std::size_t bytes = text_byte_length_for_width(line, max_columns);
        canvas.draw_text({0, row}, std::string_view(line.data(), bytes), style);
    }
}

bool TreeView::handle_event(const Event& event) {
    if (visible_.empty()) {
        return false;
    }

    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (mouse->action != MouseAction::Click && mouse->action != MouseAction::Release) {
            return false;
        }

        if (!contains_point(mouse->position)) {
            return false;
        }

        const Point local{mouse->position.x - bounds_.x, mouse->position.y - bounds_.y};
        const int index = scroll_y_ + local.y;
        if (index < 0 || index >= static_cast<int>(visible_.size())) {
            return false;
        }

        selected_index_ = index;
        toggle_selected();
        return true;
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key || !is_focused()) {
        return false;
    }

    switch (key->key) {
    case Key::Up:
        selected_index_ = std::max(0, selected_index_ - 1);
        layout(bounds_);
        if (on_select_) {
            on_select_(visible_[static_cast<std::size_t>(selected_index_)].path);
        }
        mark_dirty();
        return true;
    case Key::Down:
        selected_index_ = std::min(selected_index_ + 1, static_cast<int>(visible_.size()) - 1);
        layout(bounds_);
        if (on_select_) {
            on_select_(visible_[static_cast<std::size_t>(selected_index_)].path);
        }
        mark_dirty();
        return true;
    case Key::Left:
    case Key::Right: toggle_selected(); return true;
    case Key::Enter:
        if (on_select_) {
            on_select_(visible_[static_cast<std::size_t>(selected_index_)].path);
        }
        return true;
    default: break;
    }

    return false;
}

void TreeView::toggle_selected() {
    if (selected_index_ < 0 || selected_index_ >= static_cast<int>(visible_.size())) {
        return;
    }

    VisibleNode& entry = visible_[static_cast<std::size_t>(selected_index_)];
    if (!entry.node->children.empty()) {
        entry.node->expanded = !entry.node->expanded;
        rebuild_visible();
        layout(bounds_);
    }

    if (on_select_) {
        on_select_(visible_[static_cast<std::size_t>(selected_index_)].path);
    }

    mark_dirty();
}

} // namespace tuinator
