#pragma once

#include <tuinator/render/border_style.hpp>
#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/render/style_resolver.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/widget.hpp>

#include <memory>
#include <optional>
#include <string>

namespace tuinator {

class Panel : public Widget, public Pane {
  public:
    Panel(std::string title, Style border_style = {}, Style title_style = {},
          std::optional<BorderGlyphs> glyphs = std::nullopt);

    void set_title(std::string title);
    void set_content(std::unique_ptr<Widget> content) override;
    Widget* content() const override { return content_.get(); }

    void set_border_edges(BorderEdges edges) override;
    BorderEdges border_edges() const override { return border_edges_; }
    void reset_border_edges();
    std::string_view pane_title() const override { return title_; }

    std::string_view widget_type_name() const override { return "Panel"; }
    void apply_stylesheet(const StyleResolver& styles) override;

    void set_on_dirty(std::function<void(Rect)> callback) override;

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    Widget* hit_test(Point point) override;
    Widget* hit_test_focusable(Point point) override;
    bool has_focused_descendant() const override;
    void collect_focusable(std::vector<Widget*>& out) override;
    void for_each_child(const std::function<void(Widget*)>& visitor) override;

  private:
    Rect content_bounds() const;

    std::string title_;
    Style border_style_;
    Style title_style_;
    std::optional<BorderGlyphs> glyphs_;
    BorderEdges configured_border_edges_{true, true, true, true};
    BorderEdges border_edges_{true, true, true, true};
    std::unique_ptr<Widget> content_;
};

} // namespace tuinator
