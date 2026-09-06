#pragma once

#include <tuinator/core/geometry.hpp>

namespace tuinator {

// Tracks the screen area that needs repainting between frames.
class DirtyRegion {
public:
    bool needs_render() const { return full_ || has_bounds_; }
    bool is_full() const { return full_; }
    Rect bounds() const { return bounds_; }

    void clear() {
        full_ = false;
        has_bounds_ = false;
        bounds_ = {};
    }

    void mark_full() {
        full_ = true;
        has_bounds_ = false;
    }

    void mark_rect(Rect rect) {
        if (rect.width <= 0 || rect.height <= 0) {
            return;
        }

        if (full_) {
            return;
        }

        if (!has_bounds_) {
            bounds_ = rect;
            has_bounds_ = true;
            return;
        }

        bounds_ = unite(bounds_, rect);
    }

private:
    bool full_ = true;
    bool has_bounds_ = false;
    Rect bounds_{};
};

} // namespace tuinator
