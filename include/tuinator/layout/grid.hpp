#pragma once

#include <tuinator/widgets/widget.hpp>

namespace tuinator {

struct GridOptions {
    int columns = 2;
    int gap = 1;
    int padding = 0;
};

class Grid : public Widget {
public:
    explicit Grid(GridOptions options = {});

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(Canvas& canvas) const override;
    bool handle_event(const Event& event) override;

private:
    int columns_;
    int gap_;
    int padding_;
};

} // namespace tuinator
