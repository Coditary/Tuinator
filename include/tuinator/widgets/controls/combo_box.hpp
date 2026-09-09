#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/render/style_resolver.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/widget.hpp>

#include <functional>
#include <string>
#include <vector>

namespace tuinator {

class ComboBox : public Widget, public ValueControl, public SelectableList {
  public:
    ComboBox(Style item_style = {}, Style selected_style = {});

    const std::vector<std::string>& items() const { return items_; }
    int selected_index() const { return selected_index_; }
    bool is_open() const { return open_; }
    bool is_dropdown_open() const override { return open_; }
    int value_minimum() const override { return items_.empty() ? 0 : 0; }
    int value_maximum() const override {
        return items_.empty() ? 0 : static_cast<int>(items_.size()) - 1;
    }
    int value_current() const override { return selected_index_; }
    void set_value_current(int value) override { set_selected_index(value); }
    void set_min_width(int min_width);

    void set_items(std::vector<std::string> items);
    void set_selected_index(int index);
    void set_on_select(std::function<void(int, const std::string&)> callback);

    std::string_view widget_type_name() const override { return "ComboBox"; }
    void apply_stylesheet(const StyleResolver& styles) override;

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return true; }

  private:
    void close();
    void open();
    void select_index(int index, bool notify);

    std::vector<std::string> items_;
    int selected_index_ = 0;
    bool open_ = false;
    int scroll_y_ = 0;
    Style item_style_;
    Style selected_style_;
    Style focused_style_;
    int min_width_ = 20;
    std::function<void(int, const std::string&)> on_select_;
};

} // namespace tuinator
