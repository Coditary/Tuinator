#include <tuinator/render/animated_image.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "test_harness.hpp"

namespace {

/// Minimal GIF89a writer for tests: global 256-color table, LZW stream of
/// 9-bit literal codes (min code size 8 keeps every code at 9 bits here).
std::vector<std::uint8_t> make_test_gif(int width, int height,
                                        const std::vector<std::vector<std::uint8_t>>& frames,
                                        const std::vector<int>& delays_cs) {
    std::vector<std::uint8_t> out = {'G', 'I', 'F', '8', '9', 'a'};
    const auto u16 = [&](int value) {
        out.push_back(static_cast<std::uint8_t>(value & 0xFF));
        out.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFF));
    };
    u16(width);
    u16(height);
    out.push_back(0xF7); // GCT present, 256 entries
    out.push_back(0);
    out.push_back(0);
    for (int i = 0; i < 256; ++i) {
        out.push_back(static_cast<std::uint8_t>(i));
        out.push_back(static_cast<std::uint8_t>(255 - i));
        out.push_back(static_cast<std::uint8_t>(i * 2));
    }

    for (std::size_t f = 0; f < frames.size(); ++f) {
        out.insert(out.end(), {0x21, 0xF9, 0x04, 0x00}); // GCE, no disposal/transparency
        u16(delays_cs[f]);
        out.insert(out.end(), {0x00, 0x00});

        out.push_back(0x2C); // image descriptor
        u16(0);
        u16(0);
        u16(width);
        u16(height);
        out.push_back(0x00);

        out.push_back(8); // LZW min code size
        std::vector<int> codes = {256}; // CLEAR
        codes.insert(codes.end(), frames[f].begin(), frames[f].end());
        codes.push_back(257); // EOI
        std::vector<std::uint8_t> bytes;
        std::uint32_t acc = 0;
        int nbits = 0;
        for (const int code : codes) {
            acc |= static_cast<std::uint32_t>(code) << nbits;
            nbits += 9;
            while (nbits >= 8) {
                bytes.push_back(static_cast<std::uint8_t>(acc & 0xFF));
                acc >>= 8;
                nbits -= 8;
            }
        }
        if (nbits > 0) {
            bytes.push_back(static_cast<std::uint8_t>(acc & 0xFF));
        }
        std::size_t pos = 0;
        while (pos < bytes.size()) {
            const std::size_t chunk = std::min(bytes.size() - pos, static_cast<std::size_t>(255));
            out.push_back(static_cast<std::uint8_t>(chunk));
            out.insert(out.end(), bytes.begin() + static_cast<std::ptrdiff_t>(pos),
                       bytes.begin() + static_cast<std::ptrdiff_t>(pos + chunk));
            pos += chunk;
        }
        out.push_back(0x00);
    }
    out.push_back(0x3B);
    return out;
}

std::string write_temp_gif(const std::vector<std::uint8_t>& bytes) {
    const auto path = std::filesystem::temp_directory_path() / "tuinator_test_anim.gif";
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return path.string();
}

} // namespace

TUINATOR_TEST(animated_image_decodes_gif_frames) {
    const auto gif = make_test_gif(2, 2, {{1, 2, 3, 4}, {4, 3, 2, 1}}, {10, 20});
    const auto image = tuinator::AnimatedImage::load_gif(write_temp_gif(gif));
    TUINATOR_CHECK(image.has_value());
    TUINATOR_CHECK_EQ(image->width(), 2);
    TUINATOR_CHECK_EQ(image->height(), 2);
    TUINATOR_CHECK_EQ(image->frames().size(), 2U);
    TUINATOR_CHECK(image->animated());
    TUINATOR_CHECK_EQ(image->frames()[0].delay_cs, 10);
    TUINATOR_CHECK_EQ(image->frames()[1].delay_cs, 20);

    // GCT entry i = (i, 255-i, i*2); pixel order is row-major.
    const auto& rgba = image->frames()[0].image.rgba();
    TUINATOR_CHECK_EQ(rgba[0], 1);   // r of index 1
    TUINATOR_CHECK_EQ(rgba[1], 254); // g of index 1
    TUINATOR_CHECK_EQ(rgba[3], 255); // opaque
    TUINATOR_CHECK_EQ(rgba[4], 2);   // second pixel = index 2

    const auto& rgba2 = image->frames()[1].image.rgba();
    TUINATOR_CHECK_EQ(rgba2[0], 4); // first pixel of frame 2 = index 4
}

TUINATOR_TEST(animated_image_frame_at_loops) {
    const auto gif = make_test_gif(1, 1, {{1}, {2}}, {10, 20}); // 100ms + 200ms
    const auto image = tuinator::AnimatedImage::load_gif(write_temp_gif(gif));
    TUINATOR_CHECK(image.has_value());

    using namespace std::chrono_literals;
    TUINATOR_CHECK_EQ(image->frame_at(0ms).image.rgba()[0], 1);
    TUINATOR_CHECK_EQ(image->frame_at(99ms).image.rgba()[0], 1);
    TUINATOR_CHECK_EQ(image->frame_at(100ms).image.rgba()[0], 2);
    TUINATOR_CHECK_EQ(image->frame_at(299ms).image.rgba()[0], 2);
    TUINATOR_CHECK_EQ(image->frame_at(300ms).image.rgba()[0], 1); // looped
}

TUINATOR_TEST(animated_image_rejects_non_gif) {
    const auto path = std::filesystem::temp_directory_path() / "tuinator_test_not.gif";
    {
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        out << "not a gif at all";
    }
    TUINATOR_CHECK(!tuinator::AnimatedImage::load_gif(path.string()).has_value());
    TUINATOR_CHECK(!tuinator::AnimatedImage::load_gif("/nonexistent/x.gif").has_value());
}

TUINATOR_TEST(animated_image_from_still_is_single_frame) {
    auto image = tuinator::AnimatedImage::from_image(tuinator::TerminalImage::gradient(4, 2));
    TUINATOR_CHECK(!image.animated());
    TUINATOR_CHECK_EQ(image.frame_at(std::chrono::milliseconds(9999)).image.width(), 4);
}
