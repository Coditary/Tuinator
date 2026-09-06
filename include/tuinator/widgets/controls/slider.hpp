#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/widgets/widget.hpp>

#include <functional>

namespace tuinator {

class Slider : public Widget {
  public:
    Slider(int min_value, int max_value, int value, std::function<void(int)> on_change = {}, Style style = {},
           int min_width = 20);

    int value() const { return value_; }
    void set_value(int value);

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return true; }

  private:
    void set_value_internal(int value, bool notify);

    int min_value_;
    int max_value_;
    int value_;
    int min_width_;
    std::function<void(int)> on_change_;
    Style style_;
    Style focused_style_;
};

} // namespace tuinator
