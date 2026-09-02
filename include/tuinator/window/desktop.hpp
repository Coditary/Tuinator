#pragma once

#include <tuinator/window/window.hpp>

#include <memory>
#include <vector>

namespace tuinator {

struct MenuItem;
struct CommandPaletteEntry;
class ActionRegistry;

class ContextMenu;
class CommandPalette;

class Desktop : public Widget {
public:
    Desktop();
    ~Desktop() override;

    Window* create_window(std::string title, Rect bounds, std::unique_ptr<Widget> content,
                          WindowOptions options = {});
    Window* show_modal(std::string title, Rect bounds, std::unique_ptr<Widget> content);
    void close_window(Window* window);

    void set_background(std::unique_ptr<Widget> background);

    void set_context_menu_items(std::vector<MenuItem> items);
    void show_context_menu(Point position);
    void set_command_palette_entries(std::vector<CommandPaletteEntry> entries);
    void show_command_palette();
    void set_action_registry(std::shared_ptr<ActionRegistry> registry);

    Size preferred_size() const override;
    bool wants_full_screen() const override { return true; }
    void layout(Rect bounds) override;
    void paint(Canvas& canvas) const override;
    bool handle_event(const Event& event) override;
    void collect_focusable(std::vector<Widget*>& out) override;
    Widget* hit_test_focusable(Point point) override;

    Window* active_window() const { return active_window_; }

private:
    enum class DragMode {
        None,
        Move,
        Resize,
    };

    Window* top_window_at(Point point) const;
    Window* window_capturing_pointer() const;
    Window* top_modal() const;
    bool has_modal() const;
    void focus_window(Window* window);
    void bring_to_front(Window* window);
    void sort_windows();
    bool handle_mouse(const MouseEvent& event);
    bool dispatch_window_click(const MouseEvent& event);
    bool begin_drag_if_needed(const MouseEvent& event, Window* window);
    bool update_drag(const MouseEvent& event);
    void end_drag(const MouseEvent& event);
    bool route_keyboard(const Event& event);
    void close_top_window();

    std::unique_ptr<Widget> background_;
    std::unique_ptr<ContextMenu> context_menu_;
    std::unique_ptr<CommandPalette> command_palette_;
    std::shared_ptr<ActionRegistry> action_registry_;
    std::vector<std::unique_ptr<Window>> windows_;
    Window* active_window_ = nullptr;
    DragMode drag_mode_ = DragMode::None;
    Window* drag_window_ = nullptr;
    Point drag_anchor_{};
    Rect drag_start_bounds_{};
    int next_z_index_ = 1;
};

} // namespace tuinator
