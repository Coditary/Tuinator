#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/render/scrollbar.hpp>
#include <tuinator/widgets/widget.hpp>

#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

struct GutterLine {
    int index = 0;
    int number = 1;
    int cursor_index = 0;
    int line_count = 1;
    bool current = false;
    std::string_view text;
};

struct GutterCell {
    std::string text;
    Style style{};
};

using GutterRenderer = std::function<GutterCell(const GutterLine&)>;

GutterCell absolute_gutter(const GutterLine& line);
GutterCell relative_gutter(const GutterLine& line);
GutterCell hybrid_gutter(const GutterLine& line);

struct TextAreaOptions {
    int min_width = 40;
    int min_height = 8;
    bool line_numbers = true;
    bool status_bar = true;
    int gutter_width = 0;
    std::string title;
    std::string placeholder;
    GutterRenderer gutter;
    ScrollbarOptions scrollbars{};
};

class TextArea : public Widget {
public:
    TextArea(TextAreaOptions options = {}, Style style = {}, Style focused_style = {});

    std::string value() const;
    const std::string& title() const { return title_; }
    int cursor_row() const { return cursor_row_; }
    int cursor_column() const { return cursor_col_; }
    int line_count() const { return static_cast<int>(lines_.size()); }
    bool line_numbers() const { return line_numbers_; }
    bool status_bar() const { return status_bar_; }

    void set_value(std::string value);
    void set_title(std::string title);
    void set_placeholder(std::string placeholder);
    void set_line_numbers(bool enabled);
    void set_status_bar(bool enabled);
    void set_gutter_renderer(GutterRenderer renderer);
    void set_gutter_width(int width);
    void set_on_change(std::function<void(const std::string&)> callback);

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return true; }

private:
    int gutter_width() const;
    int content_width() const;
    int content_height() const;
    int max_line_width() const;
    ScrollbarMetrics scrollbar_metrics() const;
    GutterLine make_gutter_line(int index) const;
    GutterCell render_gutter_line(int index) const;
    void clamp_cursor();
    void ensure_cursor_visible();
    void notify_change();
    void insert_char(char ch);
    void insert_newline();
    void delete_before_cursor();
    void delete_at_cursor();
    void set_cursor(int row, int col);
    bool handle_mouse(const MouseEvent& mouse);
    Point to_local(Point terminal) const;

    std::vector<std::string> lines_{""};
    std::string title_;
    std::string placeholder_;
    int min_width_ = 40;
    int min_height_ = 8;
    bool line_numbers_ = true;
    bool status_bar_ = true;
    int gutter_width_ = 0;
    int cursor_row_ = 0;
    int cursor_col_ = 0;
    int scroll_x_ = 0;
    int scroll_y_ = 0;
    ScrollbarOptions scrollbars_;
    Style style_;
    Style focused_style_;
    GutterRenderer gutter_renderer_;
    std::function<void(const std::string&)> on_change_;
};

} // namespace tuinator
