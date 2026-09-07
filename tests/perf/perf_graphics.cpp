#include <tuinator/render/graphics_encode.hpp>
#include <tuinator/render/terminal_image.hpp>

#include "bench_harness.hpp"

TUINATOR_PERF_TEST(graphics_rgba_to_png_medium) {
    const tuinator::TerminalImage image = tuinator::TerminalImage::gradient(64, 48);

    return tuinator::perf::bench("graphics_rgba_to_png_medium", 400.0, 2, 15, [&]() { tuinator::rgba_to_png(image); });
}

TUINATOR_PERF_TEST(graphics_kitty_transmit_medium) {
    const tuinator::TerminalImage image = tuinator::TerminalImage::gradient(64, 48);

    return tuinator::perf::bench("graphics_kitty_transmit_medium", 500.0, 2, 15,
                                 [&]() { tuinator::encode_kitty_transmit(image); });
}

TUINATOR_PERF_TEST(graphics_terminal_image_hash) {
    const tuinator::TerminalImage image = tuinator::TerminalImage::gradient(128, 96);

    return tuinator::perf::bench("graphics_terminal_image_hash", 80.0, 2, 20,
                                 [&]() { tuinator::terminal_image_content_hash(image); });
}
