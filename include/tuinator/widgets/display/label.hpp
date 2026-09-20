#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/render/text.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/widget.hpp>

#include <string>

namespace tuinator {

class Label : public Widget, public TextDisplay {
  public:
    explicit Label(std::string text, Style style = {}, bool selectable = true);

    const std::string& text() const { return text_; }
    void set_text(std::string text);
    bool selectable() const { return selectable_; }
    void set_selectable(bool selectable);

    const Style& style() const { return style_; }
    std::string_view display_text() const override { return text_; }
    Style display_style() const override { return style_; }
    void set_style(Style style);

    std::string_view widget_type_name() const override { return "Label"; }

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return false; }
    bool pointer_active() const override { return selectable_ && selecting_with_mouse_; }
    Widget* hit_test_focusable(Point point) override;

  private:
    int display_column_at_local(int local_x) const;
    std::size_t byte_index_at_local_column(int local_x, CaretAffinity affinity) const;
    bool handle_mouse(const MouseEvent& mouse);
    void set_cursor_at(std::size_t pos, bool extend_selection);
    void select_all();
    void clear_selection();
    bool has_selection() const;
    std::pair<std::size_t, std::size_t> selection_range() const;
    void copy_selection();
    bool handle_shortcut(const KeyPress& key);

    std::string text_;
    Style style_;
    bool selectable_ = false;
    std::size_t selection_anchor_ = 0;
    std::size_t cursor_ = 0;
    bool selecting_with_mouse_ = false;
    std::size_t mouse_press_index_ = 0;
    int mouse_press_column_ = 0;
};

} // namespace tuinator
