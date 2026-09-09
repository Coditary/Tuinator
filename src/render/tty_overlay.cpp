#include <tuinator/render/tty_overlay.hpp>

#include <algorithm>
#include <string>

namespace tuinator {

void clear_tty_overlay_region(FILE* output, Rect region, Size terminal_size) {
    if (output == nullptr) {
        return;
    }

    region = intersect(region, {{0, 0}, terminal_size});
    if (region.width <= 0 || region.height <= 0) {
        return;
    }

    const std::string spaces(static_cast<std::size_t>(region.width), ' ');
    for (int y = region.y; y < region.bottom(); ++y) {
        std::fprintf(output, "\033[%d;%dH\033[0m", y + 1, region.x + 1);
        std::fwrite(spaces.data(), 1, spaces.size(), output);
    }
    std::fflush(output);
}

} // namespace tuinator
