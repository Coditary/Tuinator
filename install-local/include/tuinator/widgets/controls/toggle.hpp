#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/widgets/widget.hpp>

#include <functional>
#include <string>

namespace tuinator {

class Toggle : public Widget {
public:
    Toggle(std::string label,
           bool checked = false,
           std::function<void(bool)> on_change = {},
           Style style = {},
           Style checked_style = {});

    const std::string& label() const { return label_; }
    bool checked() const { return checked_; }

    void set_label(std::string label);
    void set_checked(bool checked);

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return true; }

private:
    void toggle();

    std::string label_;
    bool checked_;
    std::function<void(bool)> on_change_;
    Style style_;
    Style focused_style_;
    Style checked_style_;
};

} // namespace tuinator
