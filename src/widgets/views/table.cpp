#include <tuinator/widgets/views/table.hpp>

#include <tuinator/core/event.hpp>
#include <tuinator/render/text.hpp>

#include <algorithm>
#include <variant>

namespace tuinator {

namespace {

Style default_header_style() {
    Style style{};
    style.foreground = Color::White;
    style.background = Color::Blue;
    style.bold = true;
    return style;
}

Style default_selected_style() {
    Style style{};
    style.foreground = Color::Black;
    style.background = Color::Cyan;
    return style;
}

std::string clip_cell(std::string_view text, int width) {
    if (width <= 0) {
        return {};
    }

    const std::size_t bytes = text_byte_length_for_width(text, width);
    std::string out(text.substr(0, bytes));
    while (text_display_width(out) < width && out.size() < text.size()) {
        out.push_back(' ');
    }
    return out;
}

} // namespace

Table::Table(Style header_style, Style cell_style, Style selected_style)
    : header_style_(header_style.foreground == Color::Default
                        && header_style.background == Color::Default
                        && !header_style.bold
                    ? default_header_style()
                    : header_style),
      cell_style_(cell_style),
      selected_style_(selected_style.foreground == Color::Default
                          && selected_style.background == Color::Default
                      ? default_selected_style()
                      : selected_style) {}

void Table::set_columns(std::vector<TableColumn> columns) {
    columns_ = std::move(columns);
    mark_dirty();
}

void Table::set_rows(std::vector<std::vector<std::string>> rows) {
    rows_ = std::move(rows);
    clamp_selection();
    mark_dirty();
}

void Table::add_row(std::vector<std::string> cells) {
    rows_.push_back(std::move(cells));
    clamp_selection();
    mark_dirty();
}

void Table::set_selected_row(int row) {
    selected_row_ = row;
    clamp_selection();
    ensure_selected_visible();
    mark_dirty();
}

void Table::set_on_activate(std::function<void(int, const std::vector<std::string>&)> callback) {
    on_activate_ = std::move(callback);
}

int Table::total_width() const {
    int width = 0;
    for (const TableColumn& column : columns_) {
        width += std::max(1, column.width);
    }
    return width;
}

Size Table::preferred_size() const {
    const int width = std::max(8, total_width());
    const int height = std::max(2, static_cast<int>(rows_.size()) + 1);
    return {width, height};
}

void Table::layout(Rect bounds) {
    bounds_ = bounds;
    ensure_selected_visible();
}

int Table::visible_row_capacity() const {
    return std::max(0, bounds_.height - 1);
}

void Table::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0 || columns_.empty()) {
        return;
    }

    int x = 0;
    for (const TableColumn& column : columns_) {
        const int width = std::min(column.width, bounds_.width - x);
        if (width <= 0) {
            break;
        }

        canvas.draw_text({x, 0}, clip_cell(column.title, width), header_style_);
        x += column.width;
    }

    const int capacity = visible_row_capacity();
    for (int row = 0; row < capacity; ++row) {
        const int index = scroll_y_ + row;
        if (index < 0 || index >= static_cast<int>(rows_.size())) {
            continue;
        }

        const bool selected = index == selected_row_;
        const Style& style = selected ? selected_style_ : cell_style_;
        const std::vector<std::string>& cells = rows_[static_cast<std::size_t>(index)];

        int cell_x = 0;
        for (std::size_t col = 0; col < columns_.size(); ++col) {
            const int width = std::min(columns_[col].width, bounds_.width - cell_x);
            if (width <= 0) {
                break;
            }

            const std::string value =
                col < cells.size() ? clip_cell(cells[col], width) : std::string(static_cast<std::size_t>(width), ' ');
            canvas.draw_text({cell_x, row + 1}, value, style);
            cell_x += columns_[col].width;
        }
    }
}

bool Table::handle_event(const Event& event) {
    if (rows_.empty()) {
        return false;
    }

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
            set_selected_row(row);
            return true;
        }

        return false;
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key || !is_focused()) {
        return false;
    }

    switch (key->key) {
    case Key::Up:
        set_selected_row(selected_row_ - 1);
        return true;
    case Key::Down:
        set_selected_row(selected_row_ + 1);
        return true;
    case Key::Home:
        set_selected_row(0);
        return true;
    case Key::End:
        set_selected_row(static_cast<int>(rows_.size()) - 1);
        return true;
    case Key::Enter:
        if (on_activate_ && selected_row_ >= 0
            && selected_row_ < static_cast<int>(rows_.size())) {
            on_activate_(selected_row_, rows_[static_cast<std::size_t>(selected_row_)]);
        }
        return true;
    default:
        break;
    }

    return false;
}

void Table::clamp_selection() {
    if (rows_.empty()) {
        selected_row_ = 0;
        scroll_y_ = 0;
        return;
    }

    selected_row_ = std::clamp(selected_row_, 0, static_cast<int>(rows_.size()) - 1);
}

void Table::ensure_selected_visible() {
    const int capacity = visible_row_capacity();
    if (capacity <= 0) {
        return;
    }

    if (selected_row_ < scroll_y_) {
        scroll_y_ = selected_row_;
    }

    if (selected_row_ >= scroll_y_ + capacity) {
        scroll_y_ = selected_row_ - capacity + 1;
    }

    const int max_scroll = std::max(0, static_cast<int>(rows_.size()) - capacity);
    scroll_y_ = std::clamp(scroll_y_, 0, max_scroll);
}

int Table::row_at(Point local) const {
    if (local.y <= 0 || local.y >= bounds_.height) {
        return -1;
    }

    return scroll_y_ + (local.y - 1);
}

} // namespace tuinator
