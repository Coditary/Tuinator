#include <tuinator/backend/platform.hpp>
#include <tuinator/render/tty_overlay.hpp>

#include <cstdio>
#include <string>

#include "test_harness.hpp"

#if TUINATOR_PLATFORM_POSIX

namespace {

struct CaptureOutput {
    char* buffer = nullptr;
    size_t length = 0;
    FILE* file = nullptr;

    CaptureOutput() { file = open_memstream(&buffer, &length); }

    ~CaptureOutput() {
        if (file != nullptr) {
            std::fclose(file);
        }
        std::free(buffer);
    }

    std::string text() const { return std::string(buffer != nullptr ? buffer : "", length); }
};

} // namespace

TUINATOR_TEST(tty_overlay_clears_region_with_reset_and_spaces) {
    CaptureOutput capture;
    tuinator::clear_tty_overlay_region(capture.file, {2, 1, 4, 2}, {10, 4});

    const std::string out = capture.text();
    TUINATOR_CHECK(out.find("\033[0m") != std::string::npos);
    TUINATOR_CHECK(out.find("\033[2;3H") != std::string::npos);
    TUINATOR_CHECK(out.find("\033[3;3H") != std::string::npos);
    TUINATOR_CHECK(out.find("    ") != std::string::npos);
}

TUINATOR_TEST(tty_overlay_clips_to_terminal_bounds) {
    CaptureOutput capture;
    tuinator::clear_tty_overlay_region(capture.file, {-5, 0, 20, 2}, {8, 3});

    const std::string out = capture.text();
    TUINATOR_CHECK(out.find("\033[1;1H") != std::string::npos);
    TUINATOR_CHECK(out.find("\033[2;1H") != std::string::npos);
    TUINATOR_CHECK(out.find("\033[3;1H") == std::string::npos);
}

#endif
