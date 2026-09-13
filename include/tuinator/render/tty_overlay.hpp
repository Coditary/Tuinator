#pragma once

#include <tuinator/core/geometry.hpp>

#include <cstdio>

namespace tuinator {

/// Erase direct-to-TTY RGB/ANSI overlay cells in `region` (used before partial repaints).
void clear_tty_overlay_region(FILE* output, Rect region, Size terminal_size);

} // namespace tuinator
