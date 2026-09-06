#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace tuinator {

/// RGBA8 pixel buffer for terminal graphics protocols.
class TerminalImage {
  public:
    TerminalImage() = default;
    TerminalImage(int width, int height, std::vector<std::uint8_t> rgba);

    int width() const { return width_; }
    int height() const { return height_; }
    bool empty() const { return width_ <= 0 || height_ <= 0 || rgba_.empty(); }
    const std::vector<std::uint8_t>& rgba() const { return rgba_; }

    static TerminalImage gradient(int width, int height);
    static std::optional<TerminalImage> load_ppm(const std::string& path);
    static std::optional<TerminalImage> load_png(const std::string& path);

    TerminalImage resized(int width, int height) const;

  private:
    int width_ = 0;
    int height_ = 0;
    std::vector<std::uint8_t> rgba_;
};

struct ImageDrawRequest {
    int x = 0;
    int y = 0;
    int cell_width = 0;
    int cell_height = 0;
    TerminalImage image;
};

} // namespace tuinator
