#include <cstdlib>

#include "test_harness.hpp"

int main(int argc, char** argv) {
#if !defined(_WIN32)
    // Ensure image/graphics tests exercise the draw path in headless CI.
    setenv("TUINATOR_GRAPHICS", "kitty", 0);
#endif
    if (const char* mucpp_output = std::getenv("TUINATOR_MUCPP_OUTPUT")) {
        return tuinator::test::run_all_mucpp(mucpp_output);
    }
    return tuinator::test::run_all(argc, argv);
}
