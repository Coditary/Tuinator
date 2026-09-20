#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/widgets/display/video_view.hpp>

#include <chrono>
#include <cstdlib>
#include <string>
#include <thread>

#include "render_helper.hpp"
#include "test_harness.hpp"

namespace {

bool ffmpeg_available() { return std::system("command -v ffmpeg > /dev/null 2>&1") == 0; }

std::string make_test_video() {
    const std::string path = "/tmp/tuinator_test_video.mp4";
    const std::string cmd = "ffmpeg -loglevel error -y -f lavfi -i "
                            "testsrc=size=64x32:duration=2:rate=5 -pix_fmt yuv420p " +
                            path + " > /dev/null 2>&1";
    if (std::system(cmd.c_str()) != 0) {
        return {};
    }
    return path;
}

} // namespace

TUINATOR_TEST(video_view_plays_frames_from_ffmpeg) {
    if (!ffmpeg_available()) {
        TUINATOR_CHECK(true); // skip: no ffmpeg on this machine
        return;
    }
    const std::string video = make_test_video();
    TUINATOR_CHECK(!video.empty());

    tuinator::VideoViewOptions options;
    options.display_cells = {8, 4};
    options.fps = 5;
    options.loop = false;
    tuinator::VideoView view(options);
    view.layout({0, 0, 8, 4});
    TUINATOR_CHECK(view.open(video));
    TUINATOR_CHECK(view.playing());

    // Pump until the first frame lands (generous timeout for slow machines).
    bool drew = false;
    for (int i = 0; i < 200 && !drew; ++i) {
        view.on_idle();
        tuinator::MemoryTerminalBackend backend({8, 4});
        backend.init();
        backend.begin_frame({.full_redraw = true, .clear_buffer = true});
        tuinator::Canvas canvas(backend);
        tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
        view.paint(ctx);
        backend.end_frame();
        drew = !backend.image_draws().empty();
        if (!drew) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    TUINATOR_CHECK(drew);
    view.close();
    TUINATOR_CHECK(!view.playing());
}

TUINATOR_TEST(video_view_loops_after_eof) {
    if (!ffmpeg_available()) {
        TUINATOR_CHECK(true);
        return;
    }
    const std::string path = "/tmp/tuinator_test_video_loop.mp4";
    const std::string cmd = "ffmpeg -loglevel error -y -f lavfi -i "
                            "testsrc=size=64x32:duration=1:rate=5 -pix_fmt yuv420p " +
                            path + " > /dev/null 2>&1";
    if (std::system(cmd.c_str()) != 0) {
        TUINATOR_CHECK(true);
        return;
    }

    tuinator::VideoViewOptions options;
    options.display_cells = {8, 4};
    options.fps = 5;
    options.loop = true;
    tuinator::VideoView view(options);
    view.layout({0, 0, 8, 4});
    TUINATOR_CHECK(view.open(path));

    // Hash of the most recently drawn frame; 0 when nothing was drawn.
    auto draw_and_hash = [&]() -> std::size_t {
        view.on_idle();
        tuinator::MemoryTerminalBackend backend({8, 4});
        backend.init();
        backend.begin_frame({.full_redraw = true, .clear_buffer = true});
        tuinator::Canvas canvas(backend);
        tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
        view.paint(ctx);
        backend.end_frame();
        if (backend.image_draws().empty()) {
            return 0;
        }
        const auto& pixels = backend.image_draws().back().image.rgba();
        std::size_t hash = 1469598103934665603ULL;
        for (std::uint8_t byte : pixels) {
            hash = (hash ^ byte) * 1099511628211ULL;
        }
        return hash;
    };

    // Pump until `ms` milliseconds (wall clock) have elapsed.
    auto pump_for = [&](int ms) {
        const auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(ms);
        while (std::chrono::steady_clock::now() < until) {
            view.on_idle();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    };

    pump_for(1500); // first playthrough (1s) ends, loop respawns
    const std::size_t first = draw_and_hash();
    pump_for(600); // well inside the second playthrough
    const std::size_t second = draw_and_hash();

    TUINATOR_CHECK(first != 0);
    TUINATOR_CHECK(second != 0);
    TUINATOR_CHECK(first != second); // frozen frame would hash identically
    view.close();
}

TUINATOR_TEST(video_view_open_missing_file_still_spawns) {
    if (!ffmpeg_available()) {
        TUINATOR_CHECK(true);
        return;
    }
    tuinator::VideoView view;
    view.layout({0, 0, 8, 4});
    // ffmpeg exits with an error on stderr; the pipe just EOFs. open() itself
    // succeeds (spawn worked) and close() must not hang or crash.
    TUINATOR_CHECK(view.open("/nonexistent/video.mp4"));
    view.on_idle();
    view.close();
    TUINATOR_CHECK(true);
}
