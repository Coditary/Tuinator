#pragma once

#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/widgets/widget.hpp>

#include <memory>
#include <optional>
#include <string>

namespace tuinator {

class Panel : public Widget {
public:
    Panel(std::string title,
          Style border_style = {},
          Style title_style = {},
          std::optional<BorderGlyphs> glyphs = std::nullopt);

    void set_title(std::string title);
    void set_content(std::unique_ptr<Widget> content);
    Widget* content() const { return content_.get(); }

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(Canvas& canvas) const override;
    bool handle_event(const Event& event) override;
    Widget* hit_test(Point point) override;
    Widget* hit_test_focusable(Point point) override;
    bool has_focused_descendant() const override;
    void collect_focusable(std::vector<Widget*>& out) override;

private:
    Rect content_bounds() const;

    std::string title_;
    Style border_style_;
    Style title_style_;
    std::optional<BorderGlyphs> glyphs_;
    std::unique_ptr<Widget> content_;
};

} // namespace tuinator
