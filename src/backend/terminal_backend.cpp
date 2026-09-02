#include <tuinator/backend/terminal_backend.hpp>
#include <tuinator/render/terminal_image.hpp>

#include "backend/curses_backend.hpp"

namespace tuinator {

void TerminalBackend::draw_image(int x, int y, Size cell_size, const TerminalImage& image) {
    (void)x;
    (void)y;
    (void)cell_size;
    (void)image;
}

std::unique_ptr<TerminalBackend> TerminalBackend::create() {
    return std::make_unique<detail::CursesBackend>();
}

std::unique_ptr<TerminalBackend> TerminalBackend::create_ncurses() {
    return create();
}

} // namespace tuinator
