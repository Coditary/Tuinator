#include <tuinator/render/animated_image.hpp>

#include <cstdint>
#include <fstream>
#include <iterator>

namespace tuinator {

namespace {

struct GifColor {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
};

class Reader {
  public:
    explicit Reader(const std::vector<std::uint8_t>& data) : data_(data) {}

    bool done() const { return pos_ >= data_.size(); }
    std::uint8_t u8() { return pos_ < data_.size() ? data_[pos_++] : 0; }
    std::uint16_t u16() {
        const std::uint16_t lo = u8();
        return static_cast<std::uint16_t>(lo | (static_cast<std::uint16_t>(u8()) << 8));
    }
    bool expect(std::uint8_t value) { return u8() == value; }

    std::vector<std::uint8_t> sub_blocks() {
        std::vector<std::uint8_t> out;
        for (;;) {
            const std::uint8_t size = u8();
            if (size == 0 || pos_ + size > data_.size()) {
                break;
            }
            out.insert(out.end(), data_.begin() + static_cast<std::ptrdiff_t>(pos_),
                       data_.begin() + static_cast<std::ptrdiff_t>(pos_ + size));
            pos_ += size;
        }
        return out;
    }

  private:
    const std::vector<std::uint8_t>& data_;
    std::size_t pos_ = 0;
};

/// GIF LZW decompression over a concatenated sub-block stream.
std::vector<std::uint8_t> lzw_decode(const std::vector<std::uint8_t>& data, int min_code_size,
                                     std::size_t expected_pixels) {
    std::vector<std::uint8_t> out;
    out.reserve(expected_pixels);
    if (min_code_size < 2 || min_code_size > 8) {
        return out;
    }

    const int clear = 1 << min_code_size;
    const int eoi = clear + 1;

    std::vector<std::vector<std::uint8_t>> table;
    std::vector<std::uint8_t> prefix;
    int code_size = 0;
    std::uint32_t bits = 0;
    int bit_count = 0;
    std::size_t byte_pos = 0;

    const auto reset = [&] {
        table.assign(static_cast<std::size_t>(clear) + 2, {});
        for (int i = 0; i < clear; ++i) {
            table[static_cast<std::size_t>(i)] = {static_cast<std::uint8_t>(i)};
        }
        code_size = min_code_size + 1;
        prefix.clear();
    };
    reset();

    while (byte_pos < data.size() || bit_count >= code_size) {
        while (bit_count < code_size && byte_pos < data.size()) {
            bits |= static_cast<std::uint32_t>(data[byte_pos++]) << bit_count;
            bit_count += 8;
        }
        if (bit_count < code_size) {
            break;
        }
        const int code = static_cast<int>(bits & ((1u << code_size) - 1));
        bits >>= code_size;
        bit_count -= code_size;

        if (code == clear) {
            reset();
            continue;
        }
        if (code == eoi) {
            break;
        }

        std::vector<std::uint8_t> entry;
        if (code < static_cast<int>(table.size()) && !table[static_cast<std::size_t>(code)].empty()) {
            entry = table[static_cast<std::size_t>(code)];
        } else if (code == static_cast<int>(table.size()) && !prefix.empty()) {
            entry = prefix;
            entry.push_back(prefix.front());
        } else {
            break; // corrupt stream
        }

        out.insert(out.end(), entry.begin(), entry.end());
        if (!prefix.empty()) {
            std::vector<std::uint8_t> added = prefix;
            added.push_back(entry.front());
            if (table.size() < 4096) {
                table.push_back(std::move(added));
                if (static_cast<int>(table.size()) == (1 << code_size) && code_size < 12) {
                    ++code_size;
                }
            }
        }
        prefix = std::move(entry);
        if (out.size() >= expected_pixels + 4096) {
            break; // safety against runaway streams
        }
    }
    out.resize(std::min(out.size(), expected_pixels));
    return out;
}

/// Deinterlace GIF pixel rows (passes: 0+8k, 4+8k, 2+4k, 1+2k).
std::vector<std::uint8_t> deinterlace(const std::vector<std::uint8_t>& pixels, int width, int height) {
    std::vector<std::uint8_t> out(pixels.size());
    static constexpr int kStarts[4] = {0, 4, 2, 1};
    static constexpr int kSteps[4] = {8, 8, 4, 2};
    std::size_t src = 0;
    for (int pass = 0; pass < 4; ++pass) {
        for (int y = kStarts[pass]; y < height; y += kSteps[pass]) {
            for (int x = 0; x < width && src < pixels.size(); ++x) {
                out[static_cast<std::size_t>(y * width + x)] = pixels[src++];
            }
        }
    }
    return out;
}

} // namespace

AnimatedImage::AnimatedImage(int width, int height, std::vector<Frame> frames)
    : width_(width), height_(height), frames_(std::move(frames)) {}

const AnimatedImage::Frame& AnimatedImage::frame_at(std::chrono::milliseconds elapsed) const {
    return frames_[static_cast<std::size_t>(frame_index_at(elapsed))];
}

int AnimatedImage::frame_index_at(std::chrono::milliseconds elapsed) const {
    if (frames_.size() < 2) {
        return 0;
    }
    int total_cs = 0;
    for (const Frame& frame : frames_) {
        total_cs += frame.delay_cs;
    }
    if (total_cs <= 0) {
        return 0;
    }
    int pos_cs = static_cast<int>(elapsed.count() / 10) % total_cs;
    for (std::size_t i = 0; i < frames_.size(); ++i) {
        if (pos_cs < frames_[i].delay_cs) {
            return static_cast<int>(i);
        }
        pos_cs -= frames_[i].delay_cs;
    }
    return static_cast<int>(frames_.size() - 1);
}

AnimatedImage AnimatedImage::from_image(TerminalImage image) {
    const int w = image.width();
    const int h = image.height();
    return AnimatedImage(w, h, {{std::move(image), 0}});
}

std::optional<AnimatedImage> AnimatedImage::load_gif(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return std::nullopt;
    }
    const std::vector<std::uint8_t> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (data.size() < 13) {
        return std::nullopt;
    }

    Reader r(data);
    const std::uint8_t h0 = r.u8();
    const std::uint8_t h1 = r.u8();
    const std::uint8_t h2 = r.u8();
    if (h0 != 'G' || h1 != 'I' || h2 != 'F') {
        return std::nullopt;
    }
    r.u16(); // "87a"/"89a" as two bytes
    r.u8();

    const int width = r.u16();
    const int height = r.u16();
    if (width <= 0 || height <= 0) {
        return std::nullopt;
    }
    const std::uint8_t packed = r.u8();
    r.u8(); // background color index
    r.u8(); // aspect

    std::vector<GifColor> global_table;
    if ((packed & 0x80) != 0) {
        const int count = 1 << ((packed & 0x07) + 1);
        global_table.reserve(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i) {
            global_table.push_back({r.u8(), r.u8(), r.u8()});
        }
    }

    AnimatedImage result(width, height, {});
    std::vector<std::uint8_t> canvas(static_cast<std::size_t>(width * height * 4), 0);
    std::vector<std::uint8_t> previous;
    int delay_cs = 10;
    int disposal = 0;
    int transparent_index = -1;

    while (!r.done()) {
        const std::uint8_t introducer = r.u8();
        if (introducer == 0x3B) {
            break; // trailer
        }
        if (introducer == 0x21) { // extension
            const std::uint8_t label = r.u8();
            if (label == 0xF9) { // graphic control
                r.u8(); // block size (4)
                const std::uint8_t gce = r.u8();
                disposal = (gce >> 2) & 0x07;
                delay_cs = r.u16();
                transparent_index = (gce & 0x01) != 0 ? r.u8() : -1;
                if (transparent_index < 0) {
                    r.u8(); // transparent index byte is present regardless
                }
                r.u8(); // terminator
            } else {
                r.sub_blocks(); // skip comment/application/plain-text
            }
            continue;
        }
        if (introducer != 0x2C) {
            break; // unknown block
        }

        const int fx = r.u16();
        const int fy = r.u16();
        const int fw = r.u16();
        const int fh = r.u16();
        const std::uint8_t fpacked = r.u8();

        std::vector<GifColor> table = global_table;
        if ((fpacked & 0x80) != 0) {
            const int count = 1 << ((fpacked & 0x07) + 1);
            table.clear();
            table.reserve(static_cast<std::size_t>(count));
            for (int i = 0; i < count; ++i) {
                table.push_back({r.u8(), r.u8(), r.u8()});
            }
        }

        const int min_code_size = r.u8();
        const std::vector<std::uint8_t> compressed = r.sub_blocks();
        std::vector<std::uint8_t> pixels =
            lzw_decode(compressed, min_code_size, static_cast<std::size_t>(fw * fh));
        if (pixels.size() < static_cast<std::size_t>(fw * fh)) {
            return std::nullopt;
        }
        if ((fpacked & 0x40) != 0) {
            pixels = deinterlace(pixels, fw, fh);
        }

        if (disposal == 3) {
            previous = canvas; // restore-to-previous
        }
        for (int y = 0; y < fh; ++y) {
            for (int x = 0; x < fw; ++x) {
                const std::uint8_t index = pixels[static_cast<std::size_t>(y * fw + x)];
                if (index == transparent_index || index >= table.size()) {
                    continue;
                }
                const std::size_t dst = static_cast<std::size_t>((fy + y) * width + (fx + x)) * 4;
                if (fy + y >= height || fx + x >= width) {
                    continue;
                }
                canvas[dst] = table[index].r;
                canvas[dst + 1] = table[index].g;
                canvas[dst + 2] = table[index].b;
                canvas[dst + 3] = 255;
            }
        }

        result.frames_.push_back({TerminalImage(width, height, canvas), delay_cs});

        if (disposal == 2) { // restore to background (transparent)
            for (int y = 0; y < fh; ++y) {
                for (int x = 0; x < fw; ++x) {
                    if (fy + y >= height || fx + x >= width) {
                        continue;
                    }
                    std::fill_n(canvas.begin() + static_cast<std::ptrdiff_t>(
                                    (static_cast<std::size_t>(fy + y) * width + (fx + x)) * 4),
                                4, 0);
                }
            }
        } else if (disposal == 3) {
            canvas = previous;
        }
        delay_cs = 10;
        disposal = 0;
        transparent_index = -1;
    }

    if (result.frames_.empty()) {
        return std::nullopt;
    }
    return result;
}

} // namespace tuinator
