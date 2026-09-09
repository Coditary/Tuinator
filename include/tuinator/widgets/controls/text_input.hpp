#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/render/style_resolver.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/widget.hpp>

#include <string>

namespace tuinator {

struct TextInputOptions {
    int min_width = 20;
    std::string placeholder;
};

class TextInput : public Widget, public TextInputField {
  public:
    TextInput(TextInputOptions options = {}, Style style = {}, Style focused_style = {});

    const std::string& value() const { return value_; }
    const std::string& placeholder() const { return placeholder_; }
    int min_width() const { return min_width_; }
    std::string_view field_value() const override { return value_; }
    std::string_view field_placeholder() const override { return placeholder_; }

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

  private:
    int inner_width() const;
    void ensure_cursor_visible();
    void insert_char(char ch);
    void delete_before_cursor();
    void delete_at_cursor();
    void delete_selection();
    void move_cursor(int delta, bool extend_selection);
    void set_cursor(std::size_t pos, bool extend_selection = false);
    void select_all();
    void clear_selection();
    bool has_selection() const;
    std::pair<std::size_t, std::size_t> selection_range() const;
    void notify_change();

    std::string value_;
    std::string placeholder_;
    int min_width_;
    std::size_t cursor_ = 0;
    std::size_t selection_anchor_ = 0;
    int scroll_x_ = 0;
    Style style_;
    Style focused_style_;
    std::function<void(const std::string&)> on_change_;
    std::function<void(const std::string&)> on_submit_;
};

} // namespace tuinator
