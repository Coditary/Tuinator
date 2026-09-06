#pragma once

#include <tuinator/render/graphics_protocol.hpp>
#include <tuinator/render/terminal_image.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace tuinator {

/// Upload image bytes to the terminal (Kitty a=t). Does not display.
std::string encode_kitty_transmit(const TerminalImage& image);

/// Re-display a previously transmitted Kitty image at the current cursor.
std::string encode_kitty_place(int cell_cols, int cell_rows);

std::uint32_t terminal_image_content_hash(const TerminalImage& image);

/// Encode an image for the given protocol. Returns empty on failure.
std::string encode_terminal_image(GraphicsProtocol protocol, const TerminalImage& image, int cell_x, int cell_y,
                                  int cell_width, int cell_height);

std::vector<std::uint8_t> rgba_to_png(const TerminalImage& image);

} // namespace tuinator
