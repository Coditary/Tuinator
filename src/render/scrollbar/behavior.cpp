#include <tuinator/render/scrollbar/behavior.hpp>

#include <algorithm>

namespace tuinator {

ScrollbarBehavior ScrollbarBehavior::classic() {
    return {};
}

ScrollbarBehavior ScrollbarBehavior::minimal() {
    ScrollbarBehavior behavior;
    behavior.show_arrows = false;
    return behavior;
}

ScrollbarBehavior ScrollbarBehavior::overlay() {
    ScrollbarBehavior behavior;
    behavior.show_arrows = false;
    behavior.allow_track_jump = false;
    behavior.wheel_step = 1;
    return behavior;
}

ScrollbarBehavior ScrollbarBehavior::from_kind(ScrollbarBehaviorKind kind) {
    switch (kind) {
    case ScrollbarBehaviorKind::Minimal:
        return minimal();
    case ScrollbarBehaviorKind::Overlay:
        return overlay();
    case ScrollbarBehaviorKind::Classic:
    default:
        return classic();
    }
}

ScrollbarBehavior ScrollbarBehavior::with_wheel_step(int step) const {
    ScrollbarBehavior next = *this;
    next.wheel_step = step;
    return next;
}

ScrollbarBehavior ScrollbarBehavior::with_arrow_step(int step) const {
    ScrollbarBehavior next = *this;
    next.arrow_step = step;
    return next;
}

ScrollbarBehavior ScrollbarBehavior::with_thumb_drag(bool enabled) const {
    ScrollbarBehavior next = *this;
    next.allow_thumb_drag = enabled;
    return next;
}

ScrollbarBehavior ScrollbarBehavior::with_track_jump(bool enabled) const {
    ScrollbarBehavior next = *this;
    next.allow_track_jump = enabled;
    return next;
}

namespace {

int vertical_track_offset(bool show_arrows) {
    return show_arrows ? 1 : 0;
}

int vertical_track_length(int bar_height, bool show_arrows) {
    return std::max(0, bar_height - (show_arrows ? 2 : 0));
}

int horizontal_track_length(int bar_width, bool show_arrows) {
    return std::max(0, bar_width - (show_arrows ? 2 : 0));
}

} // namespace

ScrollbarHit scrollbar_hit_test(const ScrollbarLayout& layout, Point local, const ScrollbarBehavior& behavior) {
    if (layout.metrics.show_vertical && local.x == layout.vertical_bar_x) {
        if (behavior.show_arrows) {
            if (local.y == layout.vertical_bar_y) {
                return ScrollbarHit::VerticalArrowUp;
            }
            if (local.y == layout.vertical_bar_y + layout.vertical_bar_height - 1) {
                return ScrollbarHit::VerticalArrowDown;
            }
        }

        const int track_y = local.y - layout.vertical_bar_y - vertical_track_offset(behavior.show_arrows);
        const int track_len = vertical_track_length(layout.vertical_bar_height, behavior.show_arrows);
        if (track_y >= 0 && track_y < track_len) {
            if (track_y >= layout.vertical_thumb.start
                && track_y < layout.vertical_thumb.start + layout.vertical_thumb.size) {
                return ScrollbarHit::VerticalThumb;
            }
            return ScrollbarHit::VerticalTrack;
        }
    }

    if (layout.metrics.show_horizontal && local.y == layout.horizontal_bar_y) {
        if (behavior.show_arrows) {
            if (local.x == layout.horizontal_bar_x) {
                return ScrollbarHit::HorizontalArrowLeft;
            }
            if (local.x == layout.horizontal_bar_x + layout.horizontal_bar_width - 1) {
                return ScrollbarHit::HorizontalArrowRight;
            }
        }

        const int track_x = local.x - layout.horizontal_bar_x - vertical_track_offset(behavior.show_arrows);
        const int track_len = horizontal_track_length(layout.horizontal_bar_width, behavior.show_arrows);
        if (track_x >= 0 && track_x < track_len) {
            if (track_x >= layout.horizontal_thumb.start
                && track_x < layout.horizontal_thumb.start + layout.horizontal_thumb.size) {
                return ScrollbarHit::HorizontalThumb;
            }
            return ScrollbarHit::HorizontalTrack;
        }
    }

    return ScrollbarHit::None;
}

bool handle_scrollbar_mouse(
    const MouseEvent& mouse,
    Point local,
    const ScrollbarLayout& layout,
    const ScrollbarBehavior& behavior,
    ScrollbarInteractionState& state,
    const ScrollbarScrollActions& actions,
    int scroll_x,
    int scroll_y,
    int content_width,
    int content_height) {
    if (!actions.scroll_by || !actions.scroll_to) {
        return false;
    }

    if (state.drag_mode != ScrollbarInteractionState::DragMode::None) {
        if (mouse.action == MouseAction::Release) {
            state.reset_drag();
            return true;
        }

        if (mouse.action == MouseAction::Move || mouse.action == MouseAction::Press) {
            if (state.drag_mode == ScrollbarInteractionState::DragMode::VerticalThumb) {
                const int track_y = local.y - layout.vertical_bar_y - vertical_track_offset(behavior.show_arrows)
                    - state.drag_thumb_offset;
                const int next_y = scroll_y_for_vertical_thumb(
                    layout,
                    track_y,
                    content_height,
                    layout.metrics.viewport_height,
                    behavior.show_arrows);
                actions.scroll_to(scroll_x, next_y);
                return true;
            }

            if (state.drag_mode == ScrollbarInteractionState::DragMode::HorizontalThumb) {
                const int track_x = local.x - layout.horizontal_bar_x - vertical_track_offset(behavior.show_arrows)
                    - state.drag_thumb_offset;
                const int next_x = scroll_x_for_horizontal_thumb(
                    layout,
                    track_x,
                    content_width,
                    layout.metrics.viewport_width,
                    behavior.show_arrows);
                actions.scroll_to(next_x, scroll_y);
                return true;
            }
        }
    }

    const ScrollbarHit hit = scrollbar_hit_test(layout, local, behavior);
    if (hit == ScrollbarHit::None) {
        return false;
    }

    if (mouse.action != MouseAction::Press && mouse.action != MouseAction::Click) {
        return false;
    }

    switch (hit) {
    case ScrollbarHit::VerticalArrowUp:
        actions.scroll_by(0, -behavior.arrow_step);
        return true;
    case ScrollbarHit::VerticalArrowDown:
        actions.scroll_by(0, behavior.arrow_step);
        return true;
    case ScrollbarHit::HorizontalArrowLeft:
        actions.scroll_by(-behavior.arrow_step, 0);
        return true;
    case ScrollbarHit::HorizontalArrowRight:
        actions.scroll_by(behavior.arrow_step, 0);
        return true;
    case ScrollbarHit::VerticalThumb:
        if (behavior.allow_thumb_drag) {
            state.drag_mode = ScrollbarInteractionState::DragMode::VerticalThumb;
            state.drag_thumb_offset =
                local.y - layout.vertical_bar_y - vertical_track_offset(behavior.show_arrows)
                - layout.vertical_thumb.start;
            return true;
        }
        return false;
    case ScrollbarHit::HorizontalThumb:
        if (behavior.allow_thumb_drag) {
            state.drag_mode = ScrollbarInteractionState::DragMode::HorizontalThumb;
            state.drag_thumb_offset =
                local.x - layout.horizontal_bar_x - vertical_track_offset(behavior.show_arrows)
                - layout.horizontal_thumb.start;
            return true;
        }
        return false;
    case ScrollbarHit::VerticalTrack:
        if (behavior.allow_track_jump) {
            const int track_y = local.y - layout.vertical_bar_y - vertical_track_offset(behavior.show_arrows);
            const int centered = track_y - layout.vertical_thumb.size / 2;
            const int next_y = scroll_y_for_vertical_thumb(
                layout,
                centered,
                content_height,
                layout.metrics.viewport_height,
                behavior.show_arrows);
            actions.scroll_to(scroll_x, next_y);
            return true;
        }
        return false;
    case ScrollbarHit::HorizontalTrack:
        if (behavior.allow_track_jump) {
            const int track_x = local.x - layout.horizontal_bar_x - vertical_track_offset(behavior.show_arrows);
            const int centered = track_x - layout.horizontal_thumb.size / 2;
            const int next_x = scroll_x_for_horizontal_thumb(
                layout,
                centered,
                content_width,
                layout.metrics.viewport_width,
                behavior.show_arrows);
            actions.scroll_to(next_x, scroll_y);
            return true;
        }
        return false;
    default:
        return false;
    }
}

bool handle_scrollbar_wheel(const MouseEvent& mouse, const ScrollbarBehavior& behavior, const ScrollbarScrollActions& actions) {
    if (!actions.scroll_by) {
        return false;
    }

    switch (mouse.action) {
    case MouseAction::WheelUp:
        actions.scroll_by(0, -behavior.wheel_step);
        return true;
    case MouseAction::WheelDown:
        actions.scroll_by(0, behavior.wheel_step);
        return true;
    case MouseAction::WheelLeft:
        actions.scroll_by(-behavior.wheel_step, 0);
        return true;
    case MouseAction::WheelRight:
        actions.scroll_by(behavior.wheel_step, 0);
        return true;
    default:
        return false;
    }
}

} // namespace tuinator
