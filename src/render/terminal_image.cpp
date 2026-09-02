#include <tuinator/render/terminal_image.hpp>

#include <zlib.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>

namespace tuinator {

namespace {

std::uint8_t clamp_u8(int value) {
    return static_cast<std::uint8_t>(std::clamp(value, 0, 255));
}

std::uint32_t read_be32(const std::uint8_t* bytes) {
    return (static_cast<std::uint32_t>(bytes[0]) << 24U)
        | (static_cast<std::uint32_t>(bytes[1]) << 16U)
        | (static_cast<std::uint32_t>(bytes[2]) << 8U)
        | static_cast<std::uint32_t>(bytes[3]);
}

int paeth_predictor(int left, int up, int up_left) {
    const int estimate = left + up - up_left;
    const int dist_left = std::abs(estimate - left);
    const int dist_up = std::abs(estimate - up);
    const int dist_up_left = std::abs(estimate - up_left);
    if (dist_left <= dist_up && dist_left <= dist_up_left) {
        return left;
    }
    if (dist_up <= dist_up_left) {
        return up;
    }
    return up_left;
}

void unfilter_scanlines(std::vector<std::uint8_t>& pixels, int width, int height, int bytes_per_pixel) {
    const int row_stride = 1 + width * bytes_per_pixel;
    for (int y = 0; y < height; ++y) {
        const std::size_t row_start = static_cast<std::size_t>(y * row_stride);
        const int filter = pixels[row_start];
        std::uint8_t* row = pixels.data() + row_start + 1;

        for (int x = 0; x < width * bytes_per_pixel; ++x) {
            const std::uint8_t raw = row[x];
            const std::uint8_t left = x >= bytes_per_pixel ? row[x - bytes_per_pixel] : 0;
            const std::uint8_t up =
                y > 0 ? pixels[static_cast<std::size_t>((y - 1) * row_stride + 1 + x)] : 0;
            const std::uint8_t up_left =
                y > 0 && x >= bytes_per_pixel
                ? pixels[static_cast<std::size_t>((y - 1) * row_stride + 1 + x - bytes_per_pixel)]
                : 0;

            switch (filter) {
            case 0:
                row[x] = raw;
                break;
            case 1:
                row[x] = static_cast<std::uint8_t>(raw + left);
                break;
            case 2:
                row[x] = static_cast<std::uint8_t>(raw + up);
                break;
            case 3:
                row[x] = static_cast<std::uint8_t>(raw + ((left + up) / 2));
                break;
            case 4:
                row[x] = static_cast<std::uint8_t>(raw + paeth_predictor(left, up, up_left));
                break;
            default:
                return;
            }
        }
    }
}

} // namespace

TerminalImage::TerminalImage(int width, int height, std::vector<std::uint8_t> rgba)
    : width_(width), height_(height), rgba_(std::move(rgba)) {}

TerminalImage TerminalImage::gradient(int width, int height) {
    width = std::max(1, width);
    height = std::max(1, height);

    std::vector<std::uint8_t> rgba(static_cast<std::size_t>(width * height * 4));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const float tx = static_cast<float>(x) / static_cast<float>(width - 1 > 0 ? width - 1 : 1);
            const float ty = static_cast<float>(y) / static_cast<float>(height - 1 > 0 ? height - 1 : 1);
            const std::size_t index = static_cast<std::size_t>((y * width + x) * 4);
            rgba[index + 0] = clamp_u8(static_cast<int>(95 + tx * 137));
            rgba[index + 1] = clamp_u8(static_cast<int>(184 - ty * 32 + tx * 20));
            rgba[index + 2] = clamp_u8(static_cast<int>(158 + ty * 60 + (1.0f - tx) * 40));
            rgba[index + 3] = 255;
        }
    }

    return TerminalImage(width, height, std::move(rgba));
}

std::optional<TerminalImage> TerminalImage::load_ppm(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return std::nullopt;
    }

    std::string magic;
    input >> magic;
    if (magic != "P6") {
        return std::nullopt;
    }

    int width = 0;
    int height = 0;
    int max_value = 0;
    while (input.peek() == '#' || std::isspace(static_cast<unsigned char>(input.peek())) != 0) {
        if (input.peek() == '#') {
            std::string line;
            std::getline(input, line);
            continue;
        }
        break;
    }

    input >> width >> height >> max_value;
    input.get();
    if (!input || width <= 0 || height <= 0 || max_value != 255) {
        return std::nullopt;
    }

    const std::size_t byte_count = static_cast<std::size_t>(width * height * 3);
    std::vector<std::uint8_t> rgb(byte_count);
    input.read(reinterpret_cast<char*>(rgb.data()), static_cast<std::streamsize>(byte_count));
    if (static_cast<std::size_t>(input.gcount()) != byte_count) {
        return std::nullopt;
    }

    std::vector<std::uint8_t> rgba(byte_count / 3 * 4);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const std::size_t rgb_index = static_cast<std::size_t>((y * width + x) * 3);
            const std::size_t rgba_index = rgb_index / 3 * 4;
            rgba[rgba_index + 0] = rgb[rgb_index + 0];
            rgba[rgba_index + 1] = rgb[rgb_index + 1];
            rgba[rgba_index + 2] = rgb[rgb_index + 2];
            rgba[rgba_index + 3] = 255;
        }
    }

    return TerminalImage(width, height, std::move(rgba));
}

std::optional<TerminalImage> TerminalImage::load_png(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return std::nullopt;
    }

    std::vector<std::uint8_t> file(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());
    if (file.size() < 8) {
        return std::nullopt;
    }

    static constexpr std::uint8_t kSignature[] = {
        0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A,
    };
    if (!std::equal(std::begin(kSignature), std::end(kSignature), file.begin())) {
        return std::nullopt;
    }

    int width = 0;
    int height = 0;
    int bit_depth = 0;
    int color_type = 0;
    std::vector<std::uint8_t> compressed;

    for (std::size_t offset = 8; offset + 12 <= file.size();) {
        const std::size_t chunk_length = read_be32(file.data() + offset);
        const char* chunk_type = reinterpret_cast<const char*>(file.data() + offset + 4);
        const std::uint8_t* chunk_data = file.data() + offset + 8;
        offset += 12 + chunk_length;

        if (offset > file.size()) {
            return std::nullopt;
        }

        if (std::strncmp(chunk_type, "IHDR", 4) == 0 && chunk_length >= 13) {
            width = static_cast<int>(read_be32(chunk_data));
            height = static_cast<int>(read_be32(chunk_data + 4));
            bit_depth = static_cast<int>(chunk_data[8]);
            color_type = static_cast<int>(chunk_data[9]);
        } else if (std::strncmp(chunk_type, "IDAT", 4) == 0) {
            compressed.insert(compressed.end(), chunk_data, chunk_data + chunk_length);
        } else if (std::strncmp(chunk_type, "IEND", 4) == 0) {
            break;
        }
    }

    if (width <= 0 || height <= 0 || bit_depth != 8) {
        return std::nullopt;
    }

    const int bytes_per_pixel = color_type == 6 ? 4 : color_type == 2 ? 3 : 0;
    if (bytes_per_pixel == 0) {
        return std::nullopt;
    }

    const std::size_t expected =
        static_cast<std::size_t>((1 + width * bytes_per_pixel) * height);

    uLongf decoded_size = static_cast<uLongf>(expected);
    std::vector<std::uint8_t> decoded(decoded_size);
    const int inflate_result = uncompress(
        decoded.data(),
        &decoded_size,
        compressed.data(),
        static_cast<uLong>(compressed.size()));
    if (inflate_result != Z_OK) {
        return std::nullopt;
    }
    decoded.resize(decoded_size);

    if (decoded.size() < expected) {
        return std::nullopt;
    }

    unfilter_scanlines(decoded, width, height, bytes_per_pixel);

    std::vector<std::uint8_t> rgba(static_cast<std::size_t>(width * height * 4));
    for (int y = 0; y < height; ++y) {
        const std::size_t row_start = static_cast<std::size_t>((y * (1 + width * bytes_per_pixel)) + 1);
        for (int x = 0; x < width; ++x) {
            const std::size_t src = row_start + static_cast<std::size_t>(x * bytes_per_pixel);
            const std::size_t dst = static_cast<std::size_t>((y * width + x) * 4);
            rgba[dst + 0] = decoded[src + 0];
            rgba[dst + 1] = decoded[src + 1];
            rgba[dst + 2] = decoded[src + 2];
            rgba[dst + 3] = bytes_per_pixel == 4 ? decoded[src + 3] : 255;
        }
    }

    return TerminalImage(width, height, std::move(rgba));
}

TerminalImage TerminalImage::resized(int width, int height) const {
    if (empty() || width <= 0 || height <= 0) {
        return {};
    }

    std::vector<std::uint8_t> out(static_cast<std::size_t>(width * height * 4));
    for (int y = 0; y < height; ++y) {
        const int src_y = std::min(height_ - 1, y * height_ / height);
        for (int x = 0; x < width; ++x) {
            const int src_x = std::min(width_ - 1, x * width_ / width);
            const std::size_t src_index = static_cast<std::size_t>((src_y * width_ + src_x) * 4);
            const std::size_t dst_index = static_cast<std::size_t>((y * width + x) * 4);
            out[dst_index + 0] = rgba_[src_index + 0];
            out[dst_index + 1] = rgba_[src_index + 1];
            out[dst_index + 2] = rgba_[src_index + 2];
            out[dst_index + 3] = rgba_[src_index + 3];
        }
    }

    return TerminalImage(width, height, std::move(out));
}

} // namespace tuinator
