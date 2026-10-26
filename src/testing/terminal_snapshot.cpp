#include <tuinator/testing/terminal_snapshot.hpp>

#include <tuinator/render/color.hpp>
#include <tuinator/render/graphics_encode.hpp>

#include <algorithm>
#include <fstream>

namespace tuinator::testing {

namespace {

constexpr Rgb kDefaultForeground{200, 200, 200};
constexpr Rgb kDefaultBackground{30, 30, 30};

Rgb ansi_color(Color color) {
    switch (color) {
    case Color::Black: return {0, 0, 0};
    case Color::Red: return {220, 50, 47};
    case Color::Green: return {80, 200, 120};
    case Color::Yellow: return {220, 200, 50};
    case Color::Blue: return {80, 120, 220};
    case Color::Magenta: return {200, 80, 200};
    case Color::Cyan: return {80, 200, 220};
    case Color::White: return {230, 230, 230};
    default: return kDefaultForeground;
    }
}

Rgb resolve_foreground(const Style& style) {
    if (style.foreground_rgb.has_value()) {
        return *style.foreground_rgb;
    }
    if (style.foreground != Color::Default) {
        return ansi_color(style.foreground);
    }
    return kDefaultForeground;
}

Rgb resolve_background(const Style& style) {
    if (style.background_rgb.has_value()) {
        return *style.background_rgb;
    }
    if (style.background != Color::Default) {
        return ansi_color(style.background);
    }
    return kDefaultBackground;
}

void put_pixel(std::vector<std::uint8_t>& rgba, int width, int height, int x, int y, Rgb color) {
    if (x < 0 || y < 0 || x >= width || y >= height) {
        return;
    }
    const std::size_t index = static_cast<std::size_t>((y * width + x) * 4);
    rgba[index] = color.r;
    rgba[index + 1] = color.g;
    rgba[index + 2] = color.b;
    rgba[index + 3] = 255;
}

void draw_cell(std::vector<std::uint8_t>& rgba, int image_width, int image_height, int cell_x, int cell_y,
               TerminalPngOptions options, const MemoryTerminalBackend::Cell& cell) {
    const Rgb bg = resolve_background(cell.style);
    const Rgb fg = resolve_foreground(cell.style);

    for (int y = 0; y < options.cell_height; ++y) {
        for (int x = 0; x < options.cell_width; ++x) {
            put_pixel(rgba, image_width, image_height, cell_x + x, cell_y + y, bg);
        }
    }

    const char ch = cell.ch != '\0' ? cell.ch : ' ';
    if (ch == ' ') {
        return;
    }

    // Simple glyph: foreground core with brightness from character code (readable layout snapshots).
    const int core_w = std::max(2, options.cell_width - 4);
    const int core_h = std::max(2, options.cell_height - 6);
    const int origin_x = cell_x + (options.cell_width - core_w) / 2;
    const int origin_y = cell_y + (options.cell_height - core_h) / 2;
    const int shade = static_cast<unsigned char>(ch) % 64;
    Rgb tone{
        static_cast<std::uint8_t>(std::min(255, fg.r + shade / 2)),
        static_cast<std::uint8_t>(std::min(255, fg.g + shade / 3)),
        static_cast<std::uint8_t>(std::min(255, fg.b + shade / 4)),
    };
    for (int y = 0; y < core_h; ++y) {
        for (int x = 0; x < core_w; ++x) {
            put_pixel(rgba, image_width, image_height, origin_x + x, origin_y + y, tone);
        }
    }
}

} // namespace

std::vector<std::uint8_t> terminal_cells_to_png(const MemoryTerminalBackend& backend, TerminalPngOptions options) {
    const Size term = backend.terminal_size();
    if (term.width <= 0 || term.height <= 0) {
        return {};
    }

    const int image_width = term.width * options.cell_width;
    const int image_height = term.height * options.cell_height;
    std::vector<std::uint8_t> rgba(static_cast<std::size_t>(image_width * image_height * 4), 0);

    const auto& cells = backend.cells();
    if (cells.empty()) {
        return {};
    }

    for (int row = 0; row < term.height; ++row) {
        if (row >= static_cast<int>(cells.size())) {
            break;
        }
        const int row_width = std::min(term.width, static_cast<int>(cells[static_cast<std::size_t>(row)].size()));
        for (int col = 0; col < row_width; ++col) {
            const auto& cell = cells[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)];
            draw_cell(rgba, image_width, image_height, col * options.cell_width, row * options.cell_height, options,
                      cell);
        }
    }

    TerminalImage image(image_width, image_height, std::move(rgba));
    return rgba_to_png(image);
}

bool write_terminal_png(const MemoryTerminalBackend& backend, const std::filesystem::path& path,
                        TerminalPngOptions options) {
    const std::vector<std::uint8_t> png = terminal_cells_to_png(backend, options);
    if (png.empty()) {
        return false;
    }

    std::ofstream out(path, std::ios::binary);
    if (!out) {
        return false;
    }
    out.write(reinterpret_cast<const char*>(png.data()), static_cast<std::streamsize>(png.size()));
    return static_cast<bool>(out);
}

std::string terminal_cells_to_ansi(const MemoryTerminalBackend& backend, Theme /*theme*/) {
    const Size term = backend.terminal_size();
    const auto& cells = backend.cells();
    std::string out;

    for (int row = 0; row < term.height && row < static_cast<int>(cells.size()); ++row) {
        Style current{};
        const int row_width = std::min(term.width, static_cast<int>(cells[static_cast<std::size_t>(row)].size()));
        for (int col = 0; col < row_width; ++col) {
            const auto& cell = cells[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)];
            if (cell.style.foreground != current.foreground || cell.style.background != current.background ||
                cell.style.bold != current.bold) {
                out += "\033[0";
                if (cell.style.bold) {
                    out += ";1";
                }
                const Rgb fg = resolve_foreground(cell.style);
                out += ";38;2;" + std::to_string(fg.r) + ';' + std::to_string(fg.g) + ';' + std::to_string(fg.b);
                const Rgb bg = resolve_background(cell.style);
                out += ";48;2;" + std::to_string(bg.r) + ';' + std::to_string(bg.g) + ';' + std::to_string(bg.b) +
                       'm';
                current = cell.style;
            }
            out.push_back(cell.ch != '\0' ? cell.ch : ' ');
        }
        out += "\033[0m\n";
    }

    return out;
}

} // namespace tuinator::testing
