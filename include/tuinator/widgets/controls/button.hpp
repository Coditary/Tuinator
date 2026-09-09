#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/widget.hpp>

#include <functional>
#include <string>

namespace tuinator {

class Button : public Widget, public Activatable {
  public:
    Button(std::string label, std::function<void()> on_click = {}, Style style = {});

    const std::string& label() const { return label_; }
    void set_label(std::string label);
    void activate() override;

    std::string_view widget_type_name() const override { return "Button"; }

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return true; }
    bool wants_hover() const override { return true; }

  private:
    std::string label_;
    std::function<void()> on_click_;
    Style style_;
    Style focused_style_;
};

} // namespace tuinator
