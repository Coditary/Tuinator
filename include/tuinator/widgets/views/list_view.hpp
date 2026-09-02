#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/widgets/widget.hpp>

#include <functional>
#include <string>
#include <vector>

namespace tuinator {

class ListView : public Widget {
public:
    ListView(Style item_style = {}, Style selected_style = {});

    const std::vector<std::string>& items() const { return items_; }
    int selected_index() const { return selected_index_; }

    void set_items(std::vector<std::string> items);
    void add_item(std::string item);
    void set_selected_index(int index);
    void set_on_select(std::function<void(int index, const std::string& item)> callback);
    void set_on_activate(std::function<void(int index, const std::string& item)> callback);

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(Canvas& canvas) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return true; }

private:
    void clamp_selection();
    void ensure_selected_visible();
    int row_at(Point local) const;

    std::vector<std::string> items_;
    int selected_index_ = 0;
    int scroll_y_ = 0;
    Style item_style_;
    Style selected_style_;
    std::function<void(int, const std::string&)> on_select_;
    std::function<void(int, const std::string&)> on_activate_;
};

} // namespace tuinator
