#include <tuinator/core/event.hpp>
#include <tuinator/platform/clipboard.hpp>
#include <tuinator/render/text.hpp>
#include <tuinator/widgets/display/label.hpp>

#include <algorithm>
#include <variant>

namespace tuinator {

Label::Label(std::string text, Style style, bool selectable)
    : text_(std::move(text)), style_(style), selectable_(selectable), selection_anchor_(text_.size()),
      cursor_(text_.size()) {}

void Label::set_text(std::string text) {
    if (text_ == text) {
        return;
    }
    text_ = std::move(text);
    cursor_ = text_.size();
    selection_anchor_ = cursor_;
    mark_dirty();
}

void Label::set_selectable(bool selectable) {
    if (selectable_ == selectable) {
        return;
    }
    selectable_ = selectable;
    if (!selectable_) {
        clear_selection();
    }
    mark_dirty();
}

void Label::set_style(Style style) {
    style_ = style;
    mark_dirty();
}

Size Label::preferred_size() const {
    int max_width = 0;
    int lines = 0;
    std::size_t start = 0;

    while (start <= text_.size()) {
        std::size_t end = text_.find('\n', start);
        if (end == std::string::npos) {
            end = text_.size();
        }

        max_width = std::max(max_width, text_display_width(std::string_view(text_.data() + start, end - start)));
        ++lines;

        if (end >= text_.size()) {
            break;
        }

        start = end + 1;
    }

    return {max_width, std::max(1, lines)};
}

void Label::layout(Rect bounds) { bounds_ = bounds; }

int Label::display_column_at_local(int local_x) const {
    if (text_.empty()) {
        return 0;
    }

    std::size_t end = text_.find('\n');
    if (end == std::string::npos) {
        end = text_.size();
    }

    const std::string_view line(text_.data(), end);
    const int line_width = text_display_width(line);
    const int origin_x = std::max(0, (bounds_.width - line_width) / 2);
    const int relative = local_x - origin_x;
    if (relative <= 0) {
        return 0;
    }
    if (relative >= line_width) {
        return line_width;
    }
    return relative;
}

std::size_t Label::byte_index_at_local_column(int local_x, CaretAffinity affinity) const {
    if (text_.empty()) {
        return 0;
    }

    std::size_t end = text_.find('\n');
    if (end == std::string::npos) {
        end = text_.size();
    }

    const std::string_view line(text_.data(), end);
    const int column = display_column_at_local(local_x);
    if (column >= text_display_width(line)) {
        return end;
    }
    return text_caret_index_at_column(line, column, affinity);
}

void Label::set_cursor_at(std::size_t pos, bool extend_selection) {
    const std::size_t next = std::min(pos, text_.size());
    if (!extend_selection) {
        selection_anchor_ = next;
    }
    cursor_ = next;
    mark_dirty();
}

bool Label::handle_mouse(const MouseEvent& mouse) {
    if (!selectable_) {
        return false;
    }

    if (mouse.action == MouseAction::Release) {
        if (selecting_with_mouse_) {
            const Point local{mouse.position.x - bounds_.x, mouse.position.y - bounds_.y};
            const int release_column = display_column_at_local(local.x);
            if (release_column != mouse_press_column_) {
                set_cursor_at(byte_index_at_local_column(local.x, CaretAffinity::After), true);
            } else {
                set_cursor_at(mouse_press_index_, false);
            }
            selecting_with_mouse_ = false;
            mark_dirty();
            return true;
        }
        return contains_point(mouse.position);
    }

    if (mouse.action == MouseAction::Move) {
        if (!selecting_with_mouse_ || !mouse.left_pressed) {
            return false;
        }

        const Point local{mouse.position.x - bounds_.x, mouse.position.y - bounds_.y};
        set_cursor_at(byte_index_at_local_column(local.x, CaretAffinity::After), true);
        selecting_with_mouse_ = true;
        return true;
    }

    if (mouse.action == MouseAction::Press || mouse.action == MouseAction::Click) {
        if (!contains_point(mouse.position)) {
            return false;
        }

        const Point local{mouse.position.x - bounds_.x, mouse.position.y - bounds_.y};
        mouse_press_column_ = display_column_at_local(local.x);
        mouse_press_index_ = byte_index_at_local_column(local.x, CaretAffinity::Before);
        set_cursor_at(mouse_press_index_, false);
        selecting_with_mouse_ = true;
        return true;
    }

    return false;
}

void Label::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    paint_bounds_background(ctx, style_);
    if (text_.empty()) {
        return;
    }

    const Size text_size = preferred_size();
    const int origin_y = (bounds_.height - text_size.height) / 2;

    int y = origin_y;
    std::size_t start = 0;
    while (start <= text_.size() && y < bounds_.height) {
        std::size_t end = text_.find('\n', start);
        if (end == std::string::npos) {
            end = text_.size();
        }

        const std::string line = text_.substr(start, end - start);
        const int line_width = text_display_width(line);
        int draw_x = std::max(0, (bounds_.width - line_width) / 2);
        const int max_columns = std::max(0, bounds_.width - draw_x);
        if (max_columns <= 0) {
            break;
        }
        const std::size_t byte_length = text_byte_length_for_width(line, max_columns);
        if (byte_length == 0) {
            break;
        }
        const Style style = ctx.styles().text(*this, style_);
        Style selection_style = style;
        selection_style.reverse = true;

        if (has_selection()) {
            const auto [sel_start, sel_end] = selection_range();
            const std::size_t line_start = start;
            const std::size_t line_end = start + byte_length;
            const std::size_t highlight_start = std::max(sel_start, line_start);
            const std::size_t highlight_end = std::min(sel_end, line_end);

            if (highlight_start > line_start) {
                const std::string_view before(line.data() + (line_start - start), highlight_start - line_start);
                canvas.draw_text({draw_x, y}, before, style);
                draw_x += text_display_width(before);
            }
            if (highlight_end > highlight_start) {
                const std::string_view selected(line.data() + (highlight_start - start),
                                                highlight_end - highlight_start);
                canvas.draw_text({draw_x, y}, selected, selection_style);
                draw_x += text_display_width(selected);
            }
            if (line_end > highlight_end) {
                const std::string_view after(line.data() + (highlight_end - start), line_end - highlight_end);
                canvas.draw_text({draw_x, y}, after, style);
            }
        } else {
            canvas.draw_text({draw_x, y}, line.substr(0, byte_length), style);
        }

        if (end >= text_.size()) {
            break;
        }

        ++y;
        start = end + 1;
    }
}

Widget* Label::hit_test_focusable(Point point) {
    if (!selectable_ || !bounds_.contains(point)) {
        return nullptr;
    }
    return this;
}

bool Label::handle_event(const Event& event) {
    if (!selectable_) {
        return false;
    }

    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        return handle_mouse(*mouse);
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key || !is_focused()) {
        return false;
    }

    if (handle_shortcut(*key)) {
        return true;
    }

    if (key->key == Key::Escape && has_selection()) {
        clear_selection();
        mark_dirty();
        return true;
    }

    return false;
}

bool Label::handle_shortcut(const KeyPress& key) {
    char shortcut = '\0';
    if (key.ctrl && key.character >= 'a' && key.character <= 'z') {
        shortcut = key.character;
    } else if (key.character >= 1 && key.character <= 26) {
        shortcut = static_cast<char>('a' + key.character - 1);
    }

    if (shortcut == '\0') {
        return false;
    }

    switch (shortcut) {
    case 'a': select_all(); return true;
    case 'c': copy_selection(); return true;
    default: break;
    }

    return false;
}

void Label::select_all() {
    selection_anchor_ = 0;
    cursor_ = text_.size();
    mark_dirty();
}

void Label::clear_selection() { selection_anchor_ = cursor_; }

bool Label::has_selection() const { return selectable_ && selection_anchor_ != cursor_; }

std::pair<std::size_t, std::size_t> Label::selection_range() const {
    const std::size_t start = std::min(selection_anchor_, cursor_);
    const std::size_t end = std::max(selection_anchor_, cursor_);
    return {start, end};
}

void Label::copy_selection() {
    if (!has_selection()) {
        return;
    }

    const auto [start, end] = selection_range();
    clipboard::set(text_.substr(start, end - start));
}

} // namespace tuinator
