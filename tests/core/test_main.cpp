#include <cstdlib>

#include "test_harness.hpp"

int main() {
#if !defined(_WIN32)
    // Ensure image/graphics tests exercise the draw path in headless CI.
    setenv("TUINATOR_GRAPHICS", "kitty", 0);
#endif
    return tuinator::test::run_all();
}
