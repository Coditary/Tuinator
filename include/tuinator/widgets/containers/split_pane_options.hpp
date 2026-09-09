#pragma once

#include <tuinator/render/style.hpp>

namespace tuinator {

enum class SplitOrientation {
    Horizontal,
    Vertical,
};

struct SplitPaneOptions {
    SplitOrientation orientation = SplitOrientation::Horizontal;
    int first_size = 24;
    int min_pane_size = 4;
    int divider_hit_slop = 2;
    bool outer_border = false;
    Style divider_style{};
};

} // namespace tuinator
