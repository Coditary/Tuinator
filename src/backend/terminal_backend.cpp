#include <tuinator/backend/terminal_backend.hpp>
#include <tuinator/render/terminal_image.hpp>

#if defined(TUINATOR_BACKEND_ANSI)
#include "backend/ansi_backend.hpp"
#elif defined(TUINATOR_BACKEND_NCURSES) || defined(TUINATOR_BACKEND_PDCURSES)
#include "backend/curses_backend.hpp"
#endif

namespace tuinator {

void TerminalBackend::draw_image(int x, int y, Size cell_size, const TerminalImage& image) {
    (void)x;
    (void)y;
    (void)cell_size;
    (void)image;
}

std::unique_ptr<TerminalBackend> TerminalBackend::create() {
#if defined(TUINATOR_BACKEND_ANSI)
    return std::make_unique<detail::AnsiBackend>();
#elif defined(TUINATOR_BACKEND_NCURSES) || defined(TUINATOR_BACKEND_PDCURSES)
    return std::make_unique<detail::CursesBackend>();
#else
    return nullptr;
#endif
}

std::unique_ptr<TerminalBackend> TerminalBackend::create_ncurses() { return create(); }

} // namespace tuinator
