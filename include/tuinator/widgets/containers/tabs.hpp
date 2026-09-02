#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/widgets/widget.hpp>

#include <memory>
#include <string>
#include <vector>

namespace tuinator {

struct TabsOptions {
    Style tab_style{};
    Style selected_tab_style{};
};

class Tabs : public Widget {
public:
    explicit Tabs(TabsOptions options = {});

    void add_tab(std::string title, std::unique_ptr<Widget> content);
    int selected_index() const { return selected_index_; }
    int tab_count() const { return static_cast<int>(tabs_.size()); }
    void set_selected_index(int index);

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(Canvas& canvas) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return true; }
    bool has_focused_descendant() const override;
    void collect_focusable(std::vector<Widget*>& out) override;

private:
    struct TabEntry {
        std::string title;
        std::unique_ptr<Widget> content;
    };

    bool handle_tab_switch(const Event& event);
    int tab_at_position(Point position) const;
    void layout_active_content(const Rect& area);

    TabsOptions options_;
    std::vector<TabEntry> tabs_;
    int selected_index_ = 0;
};

} // namespace tuinator
