#pragma once

#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/render/theme.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace tuinator::testing {

struct TerminalPngOptions {
    int cell_width = 10;
    int cell_height = 18;
    Theme theme = dark_theme();
};

/// Rasterize terminal cells to PNG bytes (ASCII glyphs, ANSI colors).
std::vector<std::uint8_t> terminal_cells_to_png(const MemoryTerminalBackend& backend,
                                                  TerminalPngOptions options = {});

bool write_terminal_png(const MemoryTerminalBackend& backend, const std::filesystem::path& path,
                        TerminalPngOptions options = {});

/// SGR-colored ANSI dump (useful for reviewing in a terminal).
std::string terminal_cells_to_ansi(const MemoryTerminalBackend& backend, Theme theme = dark_theme());

} // namespace tuinator::testing
