#pragma once

#include <tuinator/core/event.hpp>
#include <tuinator/render/scrollbar/core.hpp>

#include <functional>

namespace tuinator {

enum class ScrollbarHit {
    None,
    VerticalArrowUp,
    VerticalArrowDown,
    VerticalThumb,
    VerticalTrack,
    HorizontalArrowLeft,
    HorizontalArrowRight,
    HorizontalThumb,
    HorizontalTrack,
};

enum class ScrollbarBehaviorKind {
    Classic,
    Minimal,
    Overlay,
};

struct ScrollbarBehavior {
    bool show_arrows = true;
    bool allow_thumb_drag = true;
    bool allow_track_jump = true;
    int wheel_step = 3;
    int arrow_step = 1;

    static ScrollbarBehavior classic();
    static ScrollbarBehavior minimal();
    static ScrollbarBehavior overlay();
    static ScrollbarBehavior from_kind(ScrollbarBehaviorKind kind);

    ScrollbarBehavior with_wheel_step(int step) const;
    ScrollbarBehavior with_arrow_step(int step) const;
    ScrollbarBehavior with_thumb_drag(bool enabled) const;
    ScrollbarBehavior with_track_jump(bool enabled) const;
};

struct ScrollbarInteractionState {
    enum class DragMode {
        None,
        VerticalThumb,
        HorizontalThumb,
    };

    DragMode drag_mode = DragMode::None;
    int drag_thumb_offset = 0;

    bool pointer_active() const { return drag_mode != DragMode::None; }
    void reset_drag() { drag_mode = DragMode::None; }
};

struct ScrollbarScrollActions {
    std::function<void(int delta_x, int delta_y)> scroll_by;
    std::function<void(int x, int y)> scroll_to;
};

ScrollbarHit scrollbar_hit_test(const ScrollbarLayout& layout, Point local, const ScrollbarBehavior& behavior);

bool handle_scrollbar_mouse(const MouseEvent& mouse, Point local, const ScrollbarLayout& layout,
                            const ScrollbarBehavior& behavior, ScrollbarInteractionState& state,
                            const ScrollbarScrollActions& actions, int scroll_x, int scroll_y, int content_width,
                            int content_height);

bool handle_scrollbar_wheel(const MouseEvent& mouse, const ScrollbarBehavior& behavior,
                            const ScrollbarScrollActions& actions);

} // namespace tuinator
