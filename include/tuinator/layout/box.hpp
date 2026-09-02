#pragma once

#include <tuinator/widgets/widget.hpp>

namespace tuinator {

struct BoxOptions {
    int gap = 1;
    int padding = 0;
};

class VBox : public Widget {
public:
    explicit VBox(BoxOptions options = {});

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(Canvas& canvas) const override;
    bool handle_event(const Event& event) override;

private:
    int gap_;
    int padding_;
};

class HBox : public Widget {
public:
    explicit HBox(BoxOptions options = {});

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(Canvas& canvas) const override;
    bool handle_event(const Event& event) override;

private:
    int gap_;
    int padding_;
};

} // namespace tuinator
