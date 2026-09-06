#pragma once

#include <tuinator/core/geometry.hpp>

namespace tuinator {

struct ScrollbarConfig {
    bool enabled = true;
    bool vertical = true;
    bool horizontal = true;
};

struct ScrollbarMetrics {
    bool show_vertical = false;
    bool show_horizontal = false;
    int viewport_width = 0;
    int viewport_height = 0;
};

struct ScrollbarThumb {
    int start = 0;
    int size = 0;
};

struct ScrollbarLayout {
    ScrollbarMetrics metrics;
    int vertical_bar_x = 0;
    int vertical_bar_y = 0;
    int vertical_bar_height = 0;
    int horizontal_bar_x = 0;
    int horizontal_bar_y = 0;
    int horizontal_bar_width = 0;
    ScrollbarThumb vertical_thumb;
    ScrollbarThumb horizontal_thumb;
};

ScrollbarMetrics compute_scrollbar_metrics(int width, int height, int content_width, int content_height,
                                           const ScrollbarConfig& config);

ScrollbarLayout compute_scrollbar_layout(int width, int height, int content_width, int content_height, int scroll_x,
                                         int scroll_y, const ScrollbarConfig& config, bool show_arrows);

int scroll_y_for_vertical_thumb(const ScrollbarLayout& layout, int thumb_start, int content_height, int viewport_height,
                                bool show_arrows);

int scroll_x_for_horizontal_thumb(const ScrollbarLayout& layout, int thumb_start, int content_width, int viewport_width,
                                  bool show_arrows);

} // namespace tuinator
