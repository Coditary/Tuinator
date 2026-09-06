#pragma once

#include <tuinator/widgets/widget.hpp>

#include <functional>
#include <string>
#include <vector>

namespace tuinator {

struct CommandPaletteEntry {
    std::string id;
    std::string label;
    std::string category;
    std::string shortcut;
    std::function<void()> action;
};

class CommandPalette : public Widget {
  public:
    CommandPalette();

    void set_entries(std::vector<CommandPaletteEntry> entries);
    void set_on_close(std::function<void()> callback);

    bool is_open() const { return open_; }
    void open();
    void close();

    bool wants_full_screen() const override { return true; }
    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    bool captures_pointer() const override { return open_; }

  private:
    void rebuild_matches();
    void move_selection(int delta);
    void activate_selection();
    int panel_width() const;
    int panel_height() const;

    std::vector<CommandPaletteEntry> entries_;
    std::vector<int> matches_;
    std::string query_;
    int selected_ = 0;
    bool open_ = false;
    std::function<void()> on_close_;
};

} // namespace tuinator
