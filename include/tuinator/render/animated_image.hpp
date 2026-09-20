#pragma once

#include <tuinator/render/terminal_image.hpp>

#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace tuinator {

/// Multi-frame RGBA image (GIF). Frames are fully composited by the decoder,
/// so consumers just pick frame_at(elapsed) and draw it like a still image.
class AnimatedImage {
  public:
    struct Frame {
        TerminalImage image;
        int delay_cs = 10; ///< centiseconds
    };

    AnimatedImage() = default;
    AnimatedImage(int width, int height, std::vector<Frame> frames);

    int width() const { return width_; }
    int height() const { return height_; }
    bool empty() const { return frames_.empty() || frames_.front().image.empty(); }
    const std::vector<Frame>& frames() const { return frames_; }
    bool animated() const { return frames_.size() > 1; }

    /// Looping frame for a point in time since animation start.
    const Frame& frame_at(std::chrono::milliseconds elapsed) const;
    /// Index of frame_at(elapsed) — for change detection without comparing pixels.
    int frame_index_at(std::chrono::milliseconds elapsed) const;

    static AnimatedImage from_image(TerminalImage image);
    static std::optional<AnimatedImage> load_gif(const std::string& path);

  private:
    int width_ = 0;
    int height_ = 0;
    std::vector<Frame> frames_;
};

} // namespace tuinator
