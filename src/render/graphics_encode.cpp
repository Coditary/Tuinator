#include <tuinator/render/graphics_encode.hpp>

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <zlib.h>

namespace tuinator {

namespace {

std::string base64_encode(const std::uint8_t* data, std::size_t size) {
    static constexpr char kAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string out;
    out.reserve(((size + 2) / 3) * 4);

    for (std::size_t index = 0; index < size; index += 3) {
        const std::uint32_t chunk = (static_cast<std::uint32_t>(data[index]) << 16U) |
                                    ((index + 1 < size ? static_cast<std::uint32_t>(data[index + 1]) : 0U) << 8U) |
                                    (index + 2 < size ? static_cast<std::uint32_t>(data[index + 2]) : 0U);

        out.push_back(kAlphabet[(chunk >> 18U) & 0x3FU]);
        out.push_back(kAlphabet[(chunk >> 12U) & 0x3FU]);
        out.push_back(index + 1 < size ? kAlphabet[(chunk >> 6U) & 0x3FU] : '=');
        out.push_back(index + 2 < size ? kAlphabet[chunk & 0x3FU] : '=');
    }

    return out;
}

std::uint32_t crc32_update(std::uint32_t crc, const std::uint8_t* data, std::size_t size) {
    return static_cast<std::uint32_t>(::crc32(crc, data, static_cast<uInt>(size)));
}

void append_u32_be(std::vector<std::uint8_t>& out, std::uint32_t value) {
    out.push_back(static_cast<std::uint8_t>((value >> 24U) & 0xFFU));
    out.push_back(static_cast<std::uint8_t>((value >> 16U) & 0xFFU));
    out.push_back(static_cast<std::uint8_t>((value >> 8U) & 0xFFU));
    out.push_back(static_cast<std::uint8_t>(value & 0xFFU));
}

void append_chunk(std::vector<std::uint8_t>& out, const char* type, const std::vector<std::uint8_t>& data) {
    append_u32_be(out, static_cast<std::uint32_t>(data.size()));
    out.push_back(static_cast<std::uint8_t>(type[0]));
    out.push_back(static_cast<std::uint8_t>(type[1]));
    out.push_back(static_cast<std::uint8_t>(type[2]));
    out.push_back(static_cast<std::uint8_t>(type[3]));
    out.insert(out.end(), data.begin(), data.end());
    const std::uint32_t crc =
        crc32_update(crc32_update(0, reinterpret_cast<const std::uint8_t*>(type), 4), data.data(), data.size());
    append_u32_be(out, crc);
}

std::vector<std::uint8_t> build_png(const TerminalImage& image) {
    if (image.empty()) {
        return {};
    }

    std::vector<std::uint8_t> raw;
    raw.reserve(static_cast<std::size_t>(image.height() * (1 + image.width() * 4)));
    for (int y = 0; y < image.height(); ++y) {
        raw.push_back(0);
        const std::size_t row_start = static_cast<std::size_t>(y * image.width() * 4);
        raw.insert(raw.end(), image.rgba().begin() + static_cast<std::ptrdiff_t>(row_start),
                   image.rgba().begin() + static_cast<std::ptrdiff_t>(row_start + image.width() * 4));
    }

    uLongf compressed_size = compressBound(static_cast<uLong>(raw.size()));
    std::vector<std::uint8_t> compressed(compressed_size);
    if (compress2(compressed.data(), &compressed_size, raw.data(), static_cast<uLong>(raw.size()), Z_BEST_SPEED) !=
        Z_OK) {
        return {};
    }
    compressed.resize(compressed_size);

    std::vector<std::uint8_t> png;
    png.insert(png.end(), {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A});

    std::vector<std::uint8_t> ihdr;
    ihdr.reserve(13);
    append_u32_be(ihdr, static_cast<std::uint32_t>(image.width()));
    append_u32_be(ihdr, static_cast<std::uint32_t>(image.height()));
    ihdr.push_back(8);
    ihdr.push_back(6);
    ihdr.push_back(0);
    ihdr.push_back(0);
    ihdr.push_back(0);
    append_chunk(png, "IHDR", ihdr);
    append_chunk(png, "IDAT", compressed);
    append_chunk(png, "IEND", {});

    return png;
}

std::string kitty_transmit(const TerminalImage& image) {
    if (image.empty()) {
        return {};
    }

    const std::vector<std::uint8_t> png = build_png(image);
    if (png.empty()) {
        return {};
    }

    const std::string payload = base64_encode(png.data(), png.size());
    std::ostringstream out;

    constexpr std::size_t kChunkSize = 4096;
    bool had_data = false;
    for (std::size_t offset = 0; offset < payload.size(); offset += kChunkSize) {
        const std::size_t length = std::min(kChunkSize, payload.size() - offset);
        const std::string chunk = payload.substr(offset, length);
        had_data = true;

        if (offset == 0) {
            out << "\033_Ga=t,f=100,i=1,q=2,m=1;" << chunk << "\033\\";
            continue;
        }

        out << "\033_Gm=1;" << chunk << "\033\\";
    }

    if (had_data) {
        out << "\033_Gm=0;\033\\";
    }

    return out.str();
}

std::string kitty_place(int cell_cols, int cell_rows) {
    std::ostringstream out;
    out << "\033_Ga=p,i=1,c=" << cell_cols << ",r=" << cell_rows << ",C=1,q=2;\033\\";
    return out.str();
}

std::string encode_iterm2(const TerminalImage& image, int cell_x, int cell_y, int cell_width, int cell_height) {
    const std::vector<std::uint8_t> png = build_png(image);
    if (png.empty()) {
        return {};
    }

    const std::string payload = base64_encode(png.data(), png.size());
    std::ostringstream out;
    out << "\033]1337;File=inline=1;size=" << std::max(1, cell_width) << "x" << std::max(1, cell_height)
        << ";preserveAspectRatio=1";
    if (cell_x > 0) {
        out << ";x=" << cell_x;
    }
    if (cell_y > 0) {
        out << ";y=" << cell_y;
    }
    out << ':' << payload << '\007';
    return out.str();
}

std::string encode_sixel(const TerminalImage& image, int cell_x, int cell_y) {
    if (image.empty()) {
        return {};
    }

    const int width = std::min(image.width(), 64);
    const int height = std::min(image.height(), 64);
    const TerminalImage scaled = image.resized(width, height);

    std::ostringstream out;
    out << "\033P0;0;0q\"1;1;" << (cell_x + 1) << ";" << (cell_y + 1);
    for (int y = 0; y < height; ++y) {
        out << '#' << (y % 6);
        for (int x = 0; x < width; ++x) {
            const std::size_t index = static_cast<std::size_t>((y * width + x) * 4);
            const auto& rgba = scaled.rgba();
            const int color = 16 + ((rgba[index] / 51) * 36 + (rgba[index + 1] / 51) * 6 + (rgba[index + 2] / 51));
            out << static_cast<char>('?' + (color % 63));
        }
        out << '$';
    }
    out << "\033\\";
    return out.str();
}

} // namespace

std::string encode_kitty_transmit(const TerminalImage& image) { return kitty_transmit(image); }

std::string encode_kitty_place(int cell_cols, int cell_rows) { return kitty_place(cell_cols, cell_rows); }

std::uint32_t terminal_image_content_hash(const TerminalImage& image) {
    if (image.empty()) {
        return 0;
    }

    return static_cast<std::uint32_t>(::crc32(0, image.rgba().data(), static_cast<uInt>(image.rgba().size())));
}

std::vector<std::uint8_t> rgba_to_png(const TerminalImage& image) { return build_png(image); }

std::string encode_terminal_image(GraphicsProtocol protocol, const TerminalImage& image, int cell_x, int cell_y,
                                  int cell_width, int cell_height) {
    switch (protocol) {
    case GraphicsProtocol::Kitty:
        (void)cell_x;
        (void)cell_y;
        return kitty_transmit(image);
    case GraphicsProtocol::Iterm2: return encode_iterm2(image, cell_x, cell_y, cell_width, cell_height);
    case GraphicsProtocol::Sixel: return encode_sixel(image, cell_x, cell_y);
    case GraphicsProtocol::None: return {};
    }

    return {};
}

} // namespace tuinator
