#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/widget.hpp>

#include <string>

namespace tuinator {

class Label : public Widget, public TextDisplay {
  public:
    explicit Label(std::string text, Style style = {});

    const std::string& text() const { return text_; }
    void set_text(std::string text);

    const Style& style() const { return style_; }
    std::string_view display_text() const override { return text_; }
    Style display_style() const override { return style_; }
    void set_style(Style style);

    std::string_view widget_type_name() const override { return "Label"; }

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;

  private:
    std::string text_;
    Style style_;
};

} // namespace tuinator
