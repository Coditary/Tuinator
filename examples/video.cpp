/// Video playback demo: pipes frames from an ffmpeg child process into a
/// VideoView, rendered via the terminal graphics protocol (Kitty/Sixel/iTerm2).
/// Run from the repository root: ./build/examples/video
#include <tuinator/tuinator.hpp>

using namespace tuinator;

int main() {
    Application app;

    VideoViewOptions options;
    options.display_cells = {56, 16};
    options.fps = 15;
    options.loop = true;

    auto video = std::make_unique<VideoView>("examples/assets/test.mp4", options);
    if (!video->playing()) {
        app.set_root(std::make_unique<Label>("Konnte ffmpeg nicht starten (ffmpeg auf PATH?)"));
    } else {
        app.set_root(std::move(video));
    }
    return app.run();
}
