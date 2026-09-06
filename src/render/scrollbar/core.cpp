#include <tuinator/render/scrollbar/core.hpp>

#include <algorithm>

namespace tuinator {

namespace {

int track_length(int bar_length, bool show_arrows) { return std::max(0, bar_length - (show_arrows ? 2 : 0)); }

ScrollbarThumb compute_vertical_thumb(int bar_height, int content_height, int viewport_height, int scroll_y,
                                      bool show_arrows) {
    ScrollbarThumb thumb;
    const int track_len = track_length(bar_height, show_arrows);
    const int max_scroll = std::max(0, content_height - viewport_height);

    if (track_len <= 0) {
        return thumb;
    }

    if (content_height > 0 && viewport_height > 0) {
        thumb.size = std::max(1, track_len * viewport_height / content_height);
        thumb.size = std::min(thumb.size, track_len);
    } else {
        thumb.size = 1;
    }

    if (max_scroll > 0 && track_len > thumb.size) {
        thumb.start = (track_len - thumb.size) * scroll_y / max_scroll;
    }

    return thumb;
}

ScrollbarThumb compute_horizontal_thumb(int bar_width, int content_width, int viewport_width, int scroll_x,
                                        bool show_arrows) {
    ScrollbarThumb thumb;
    const int track_len = track_length(bar_width, show_arrows);
    const int max_scroll = std::max(0, content_width - viewport_width);

    if (track_len <= 0) {
        return thumb;
    }

    if (content_width > 0 && viewport_width > 0) {
        thumb.size = std::max(1, track_len * viewport_width / content_width);
        thumb.size = std::min(thumb.size, track_len);
    } else {
        thumb.size = 1;
    }

    if (max_scroll > 0 && track_len > thumb.size) {
        thumb.start = (track_len - thumb.size) * scroll_x / max_scroll;
    }

    return thumb;
}

} // namespace

ScrollbarMetrics compute_scrollbar_metrics(int width, int height, int content_width, int content_height,
                                           const ScrollbarConfig& config) {
    ScrollbarMetrics metrics;
    metrics.viewport_width = width;
    metrics.viewport_height = height;

    if (!config.enabled || width <= 0 || height <= 0) {
        return metrics;
    }

    if (config.vertical && content_height > height) {
        metrics.show_vertical = true;
        metrics.viewport_width = width - 1;
    }

    if (config.horizontal && content_width > metrics.viewport_width) {
        metrics.show_horizontal = true;
        metrics.viewport_height = height - 1;
    }

    if (config.vertical && !metrics.show_vertical && content_height > metrics.viewport_height) {
        metrics.show_vertical = true;
        metrics.viewport_width = width - 1;
    }

    metrics.viewport_width = std::max(0, metrics.viewport_width);
    metrics.viewport_height = std::max(0, metrics.viewport_height);
    return metrics;
}

ScrollbarLayout compute_scrollbar_layout(int width, int height, int content_width, int content_height, int scroll_x,
                                         int scroll_y, const ScrollbarConfig& config, bool show_arrows) {
    ScrollbarLayout layout;
    layout.metrics = compute_scrollbar_metrics(width, height, content_width, content_height, config);

    if (layout.metrics.show_vertical) {
        layout.vertical_bar_x = width - 1;
        layout.vertical_bar_y = 0;
        layout.vertical_bar_height = height - (layout.metrics.show_horizontal ? 1 : 0);
        layout.vertical_thumb = compute_vertical_thumb(layout.vertical_bar_height, content_height,
                                                       layout.metrics.viewport_height, scroll_y, show_arrows);
    }

    if (layout.metrics.show_horizontal) {
        layout.horizontal_bar_x = 0;
        layout.horizontal_bar_y = height - 1;
        layout.horizontal_bar_width = width - (layout.metrics.show_vertical ? 1 : 0);
        layout.horizontal_thumb = compute_horizontal_thumb(layout.horizontal_bar_width, content_width,
                                                           layout.metrics.viewport_width, scroll_x, show_arrows);
    }

    return layout;
}

int scroll_y_for_vertical_thumb(const ScrollbarLayout& layout, int thumb_start, int content_height, int viewport_height,
                                bool show_arrows) {
    const int track_len = track_length(layout.vertical_bar_height, show_arrows);
    const int max_scroll = std::max(0, content_height - viewport_height);
    if (max_scroll <= 0 || track_len <= layout.vertical_thumb.size) {
        return 0;
    }

    const int clamped = std::clamp(thumb_start, 0, track_len - layout.vertical_thumb.size);
    return clamped * max_scroll / (track_len - layout.vertical_thumb.size);
}

int scroll_x_for_horizontal_thumb(const ScrollbarLayout& layout, int thumb_start, int content_width, int viewport_width,
                                  bool show_arrows) {
    const int track_len = track_length(layout.horizontal_bar_width, show_arrows);
    const int max_scroll = std::max(0, content_width - viewport_width);
    if (max_scroll <= 0 || track_len <= layout.horizontal_thumb.size) {
        return 0;
    }

    const int clamped = std::clamp(thumb_start, 0, track_len - layout.horizontal_thumb.size);
    return clamped * max_scroll / (track_len - layout.horizontal_thumb.size);
}

} // namespace tuinator
