#pragma once

#include <tuinator/window/window.hpp>
#include <tuinator/window/window_manager.hpp>

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

    Window* create_window(std::string title, Rect bounds, std::unique_ptr<Widget> content, WindowOptions options = {});
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
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    void collect_focusable(std::vector<Widget*>& out) override;
    Widget* hit_test_focusable(Point point) override;

    Window* active_window() const { return windows_.active_window(); }

  private:
    std::unique_ptr<Widget> background_;
    std::unique_ptr<ContextMenu> context_menu_;
    std::unique_ptr<CommandPalette> command_palette_;
    std::shared_ptr<ActionRegistry> action_registry_;
    WindowManager windows_;
};

} // namespace tuinator
