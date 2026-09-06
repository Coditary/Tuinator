#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/widgets/widget.hpp>

#include <functional>
#include <string>
#include <vector>

namespace tuinator {

struct TableColumn {
    std::string title;
    int width = 10;
};

class Table : public Widget {
public:
    Table(Style header_style = {}, Style cell_style = {}, Style selected_style = {});

    const std::vector<TableColumn>& columns() const { return columns_; }
    const std::vector<std::vector<std::string>>& rows() const { return rows_; }
    int selected_row() const { return selected_row_; }

    void set_columns(std::vector<TableColumn> columns);
    void set_rows(std::vector<std::vector<std::string>> rows);
    void add_row(std::vector<std::string> cells);
    void set_selected_row(int row);
    void set_on_activate(std::function<void(int row, const std::vector<std::string>& cells)> callback);

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return true; }

private:
    void clamp_selection();
    void ensure_selected_visible();
    int visible_row_capacity() const;
    int row_at(Point local) const;
    int total_width() const;

    std::vector<TableColumn> columns_;
    std::vector<std::vector<std::string>> rows_;
    int selected_row_ = 0;
    int scroll_y_ = 0;
    Style header_style_;
    Style cell_style_;
    Style selected_style_;
    std::function<void(int, const std::vector<std::string>&)> on_activate_;
};

} // namespace tuinator
