#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/widgets/widget.hpp>

#include <functional>

namespace tuinator {

class Spinner : public Widget {
public:
    Spinner(int min_value,
            int max_value,
            int value,
            int step = 1,
            std::function<void(int)> on_change = {},
            Style style = {});

    int value() const { return value_; }
    void set_value(int value);

    Size preferred_size() const override;
    void paint(Canvas& canvas) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return true; }

private:
    void adjust(int delta);

    int min_value_;
    int max_value_;
    int value_;
    int step_;
    std::function<void(int)> on_change_;
    Style style_;
    Style focused_style_;
};

} // namespace tuinator
