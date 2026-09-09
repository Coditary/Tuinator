#include <tuinator/debug/debug_paint.hpp>

#include <cstdlib>

#include "test_harness.hpp"

TUINATOR_TEST(debug_paint_disabled_by_default) {
#if TUINATOR_PLATFORM_POSIX
    const char* previous = std::getenv("TUINATOR_DEBUG_PAINT");
    unsetenv("TUINATOR_DEBUG_PAINT");
    TUINATOR_CHECK(!tuinator::debug_paint_enabled());
    if (previous != nullptr) {
        setenv("TUINATOR_DEBUG_PAINT", previous, 1);
    }
#else
    TUINATOR_CHECK(!tuinator::debug_paint_enabled());
#endif
}

#if TUINATOR_PLATFORM_POSIX

TUINATOR_TEST(debug_paint_enabled_from_env) {
    const int previous = setenv("TUINATOR_DEBUG_PAINT", "1", 1);
    TUINATOR_CHECK(previous == 0 || previous == 1);
    TUINATOR_CHECK(tuinator::debug_paint_enabled());
    unsetenv("TUINATOR_DEBUG_PAINT");
}

#endif
