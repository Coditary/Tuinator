#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/render/style_resolver.hpp>
#include <tuinator/render/text.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/widget.hpp>

#include <string>
#include <string_view>

namespace tuinator {

enum class TextInputChrome {
    Brackets,
    Framed,
};

struct TextInputOptions {
    int min_width = 20;
    std::string placeholder;
    /// When true, typed characters are shown as mask glyphs (default `*`).
    bool password = false;
    TextInputChrome chrome = TextInputChrome::Brackets;
    /// Framed mode: minimum inner text rows (default 1 → 3-row box with borders).
    int min_content_lines = 1;
    /// Framed mode: grow inner text rows with content, up to this limit.
    int max_content_lines = 1;
    std::string prompt;
};

class TextInput : public Widget, public TextInputField {
  public:
    TextInput(TextInputOptions options = {}, Style style = {}, Style focused_style = {});

    const std::string& value() const { return value_; }
    const std::string& placeholder() const { return placeholder_; }
    int min_width() const { return min_width_; }
    bool password() const { return password_; }
    std::string_view field_value() const override { return value_; }
    std::string_view field_placeholder() const override { return placeholder_; }

    [[nodiscard]] std::size_t cursor_position() const { return cursor_; }
    [[nodiscard]] int horizontal_scroll() const { return scroll_x_; }
    void set_value(std::string value);
    void set_placeholder(std::string placeholder);
    void set_on_change(std::function<void(const std::string&)> callback);
    void set_on_submit(std::function<void(const std::string&)> callback);
    void set_options(TextInputOptions options);

    std::string_view widget_type_name() const override { return "TextInput"; }
    void apply_stylesheet(const StyleResolver& styles) override;

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return true; }
    bool pointer_active() const override { return selecting_with_mouse_; }

  private:
    int text_row() const;
    int text_col_start() const;
    int inner_width() const;
    int inner_height() const;
    int content_line_count() const;
    int framed_height_for_lines(int lines) const;
    int display_column_at_local(int local_x) const;
    std::size_t byte_index_at_local(int local_x, int local_y, CaretAffinity affinity) const;
    void move_cursor_vertical(int delta, bool extend_selection);
    void update_layout_for_lines();
    bool handle_mouse(const MouseEvent& mouse);
    void ensure_cursor_visible();
    void insert_char(char ch);
    void delete_before_cursor();
    void delete_at_cursor();
    void delete_selection();
    void delete_word_before_cursor();
    void delete_word_after_cursor();
    void delete_line_before_cursor();
    void delete_line_after_cursor();
    void delete_entire_field();
    bool insert_text(std::string_view text);
    void copy_selection();
    void cut_selection();
    void paste_from_clipboard();
    void move_cursor(int delta, bool extend_selection);
    void set_cursor(std::size_t pos, bool extend_selection = false);
    void select_all();
    void clear_selection();
    bool has_selection() const;
    std::string selected_text() const;
    std::pair<std::size_t, std::size_t> selection_range() const;
    bool handle_shortcut(const KeyPress& key);
    void notify_change();

    std::string value_;
    std::string placeholder_;
    int min_width_;
    bool password_ = false;
    TextInputChrome chrome_ = TextInputChrome::Brackets;
    int min_content_lines_ = 1;
    int max_content_lines_ = 1;
    std::string prompt_;
    std::size_t cursor_ = 0;
    std::size_t selection_anchor_ = 0;
    bool selecting_with_mouse_ = false;
    std::size_t mouse_press_index_ = 0;
    int mouse_press_display_col_ = 0;
    int scroll_x_ = 0;
    int scroll_y_ = 0;
    Style style_;
    Style focused_style_;
    std::function<void(const std::string&)> on_change_;
    std::function<void(const std::string&)> on_submit_;
};

} // namespace tuinator
