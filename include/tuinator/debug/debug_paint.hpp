#pragma once

#include <tuinator/core/geometry.hpp>

namespace tuinator {

/// True when `TUINATOR_DEBUG_PAINT` is set to a non-empty, non-zero value.
bool debug_paint_enabled();

/// Log one rendered frame (mode, dirty region, background-fill count).
void debug_paint_log_frame(bool partial, const Rect& dirty_region);

/// Record that a widget called `paint_bounds_background()` this frame.
void debug_paint_note_background_fill();

/// Reset per-frame counters at the start of a render pass.
void debug_paint_begin_frame();

} // namespace tuinator
