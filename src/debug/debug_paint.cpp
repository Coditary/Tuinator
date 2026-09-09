#include <tuinator/debug/debug_paint.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace tuinator {

namespace {

bool env_flag_enabled(const char* name) {
    const char* value = std::getenv(name);
    return value != nullptr && value[0] != '\0' && std::strcmp(value, "0") != 0;
}

int background_fills_ = 0;

} // namespace

bool debug_paint_enabled() { return env_flag_enabled("TUINATOR_DEBUG_PAINT"); }

void debug_paint_begin_frame() { background_fills_ = 0; }

void debug_paint_note_background_fill() {
    if (debug_paint_enabled()) {
        ++background_fills_;
    }
}

void debug_paint_log_frame(bool partial, const Rect& dirty_region) {
    if (!debug_paint_enabled()) {
        return;
    }

    std::fprintf(stderr,
                 "tuinator-paint: mode=%s dirty=%d,%d %dx%d fills=%d\n",
                 partial ? "partial" : "full",
                 dirty_region.x,
                 dirty_region.y,
                 dirty_region.width,
                 dirty_region.height,
                 background_fills_);
}

} // namespace tuinator
