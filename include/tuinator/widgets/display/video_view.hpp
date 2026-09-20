#pragma once

#include <tuinator/core/geometry.hpp>
#include <tuinator/render/terminal_image.hpp>
#include <tuinator/widgets/widget.hpp>

#include <chrono>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

struct VideoViewOptions {
    /// Cell area the video is scaled into; {0,0} derives it from the layout
    /// bounds (capped at max_pixel_size for throughput).
    Size display_cells = {};
    /// Pixel cap for the ffmpeg scale filter (keeps escape traffic sane).
    Size max_pixel_size = {320, 180};
    int fps = 15;
    bool loop = true;
};

/// Plays a video file by piping decoded RGBA frames from an ffmpeg child
/// process (`ffmpeg -i <path> -f rawvideo -pix_fmt rgba -`). No linked codec
/// dependency: ffmpeg is spawned at runtime and must be on PATH. Frames are
/// paced by wall clock and rendered through the terminal graphics protocol
/// (same path as images/GIFs); audio is not played.
class VideoView : public Widget {
  public:
    explicit VideoView(VideoViewOptions options = {});
    explicit VideoView(std::string path, VideoViewOptions options = {});
    ~VideoView() override;

    VideoView(const VideoView&) = delete;
    VideoView& operator=(const VideoView&) = delete;

    /// Start playback; false when ffmpeg cannot be spawned.
    bool open(const std::string& path);
    void close();
    bool playing() const { return pid_ > 0 || draining_; }

    Size preferred_size() const override { return {16, 8}; }
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;
    bool needs_periodic_idle() const override { return pid_ > 0 || draining_; }
    void on_idle() override;

    std::string_view widget_type_name() const override { return "VideoView"; }

  private:
    bool spawn();
    void pump_pipe();      ///< read available bytes, complete frames
    void advance_frame();  ///< wall-clock pacing

    VideoViewOptions options_;
    std::string path_;
    int pid_ = -1;
    int pipe_fd_ = -1;
    int pixel_width_ = 0;
    int pixel_height_ = 0;
    std::size_t frame_bytes_ = 0;
    std::vector<std::uint8_t> read_buf_;
    TerminalImage current_;
    std::chrono::steady_clock::time_point started_{};
    std::size_t frames_shown_ = 0;
    bool draining_ = false; ///< EOF reached, buffered frames still being shown
};

} // namespace tuinator
