#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/widgets/widget.hpp>

#include <string>

namespace tuinator {

class StatusBar : public Widget {
  public:
    explicit StatusBar(std::string text = {}, Style style = {});

    const std::string& text() const { return text_; }
    void set_text(std::string text);

    std::string_view widget_type_name() const override { return "StatusBar"; }

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;

  private:
    std::string text_;
    Style style_;
};

} // namespace tuinator
