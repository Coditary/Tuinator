#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/render/style_resolver.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/widget.hpp>

#include <functional>
#include <string>
#include <vector>

namespace tuinator {

struct TreeNode {
    std::string label;
    std::vector<TreeNode> children;
    bool expanded = true;
};

class TreeView : public Widget, public SelectableList {
  public:
    TreeView(Style item_style = {}, Style selected_style = {});

    void set_root(TreeNode root);
    const TreeNode& root() const { return root_; }

    int selected_index() const override { return selected_index_; }
    void set_selected_index(int index) override;
    void set_on_select(std::function<void(const std::string& path)> callback);

    std::string_view widget_type_name() const override { return "TreeView"; }
    bool wants_hover() const override { return true; }
    void apply_stylesheet(const StyleResolver& styles) override;

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return true; }

  private:
    struct VisibleNode {
        TreeNode* node = nullptr;
        std::string path;
        int depth = 0;
    };

    void rebuild_visible();
    void append_visible(TreeNode& node, const std::string& path, int depth);
    void toggle_selected();

    TreeNode root_;
    std::vector<VisibleNode> visible_;
    int selected_index_ = 0;
    int scroll_y_ = 0;
    Style item_style_;
    Style selected_style_;
    std::function<void(const std::string&)> on_select_;
};

} // namespace tuinator
