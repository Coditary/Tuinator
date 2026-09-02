#pragma once

#include <tuinator/widgets/menu/menu_common.hpp>
#include <tuinator/widgets/widget.hpp>

#include <functional>
#include <string>
#include <vector>

namespace tuinator {

class ContextMenu : public Widget {
public:
    ContextMenu();

    void set_items(std::vector<MenuItem> items);
    void set_look(const MenuBarLook& look);
    void set_on_action(std::function<void(const std::string& item)> callback);
    void set_on_hint(std::function<void(const std::string& hint)> callback);

    bool is_open() const { return open_; }
    void show(Point position);
    void close();

    bool wants_full_screen() const override { return true; }
    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    bool captures_pointer() const override { return open_; }

private:
    const std::vector<MenuItem>* current_items() const;
    void open_submenu();
    void close_submenu();
    void activate_item(int index);
    void move_selection(int delta);
    bool item_selectable(int index) const;
    void notify_hint();
    std::vector<MenuPanelLayout> open_panels() const;
    bool hit_panel(Point local, int& panel_index, int& row) const;

    std::vector<MenuItem> items_;
    MenuBarLook look_;
    Point anchor_{};
    int active_item_ = 0;
    std::vector<int> submenu_path_;
    bool open_ = false;
    std::function<void(const std::string&)> on_action_;
    std::function<void(const std::string&)> on_hint_;
};

} // namespace tuinator
