#pragma once

#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/render/paint_context.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/widget.hpp>

namespace tuinator::test {

inline void render_root(Widget& root, MemoryTerminalBackend& backend, BorderGlyphs glyphs = ascii_border_glyphs()) {
    if (!backend.terminal_size().width || !backend.terminal_size().height) {
        backend.init();
    }

    const Size term = backend.terminal_size();
    root.layout({0, 0, term.width, term.height});

    backend.begin_frame();
    Canvas canvas(backend);
    canvas.set_glyphs(std::move(glyphs));
    const Theme theme = dark_theme();
    PaintContext ctx{canvas, theme};
    root.paint(ctx);
    backend.end_frame();
}

inline PaintContext make_paint_context(Canvas& canvas, const Theme& theme = dark_theme()) {
    canvas.set_glyphs(theme.glyphs);
    return PaintContext{canvas, theme};
}

inline char cell_at(const MemoryTerminalBackend& backend, int x, int y) {
    const auto& cells = backend.cells();
    if (y < 0 || y >= static_cast<int>(cells.size())) {
        return '\0';
    }
    if (x < 0 || x >= static_cast<int>(cells[static_cast<std::size_t>(y)].size())) {
        return '\0';
    }
    return cells[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)].ch;
}

inline bool row_contains(const MemoryTerminalBackend& backend, int y, std::string_view text) {
    const auto& cells = backend.cells();
    if (y < 0 || y >= static_cast<int>(cells.size())) {
        return false;
    }

    std::string row;
    for (const auto& cell : cells[static_cast<std::size_t>(y)]) {
        row.push_back(cell.ch ? cell.ch : ' ');
    }

    return row.find(text) != std::string::npos;
}

} // namespace tuinator::test
