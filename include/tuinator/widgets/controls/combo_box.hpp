#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/widgets/widget.hpp>

#include <functional>
#include <string>
#include <vector>

namespace tuinator {

class ComboBox : public Widget {
  public:
    ComboBox(Style item_style = {}, Style selected_style = {});

    const std::vector<std::string>& items() const { return items_; }
    int selected_index() const { return selected_index_; }
    bool is_open() const { return open_; }

    void set_items(std::vector<std::string> items);
    void set_selected_index(int index);
    void set_on_select(std::function<void(int, const std::string&)> callback);

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
    std::function<void(int, const std::string&)> on_select_;
};

} // namespace tuinator
