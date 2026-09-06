#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/render/terminal_image.hpp>

#include <algorithm>
#include <sstream>

namespace tuinator {

MemoryTerminalBackend::MemoryTerminalBackend(Size size)
    : size_(size.width > 0 ? size.width : 80, size.height > 0 ? size.height : 24) {}

void MemoryTerminalBackend::init() {
    resize(size_);
    initialized_ = true;
}

void MemoryTerminalBackend::shutdown() {
    initialized_ = false;
    events_.clear();
}

Size MemoryTerminalBackend::terminal_size() const { return size_; }

void MemoryTerminalBackend::resize(Size size) {
    size_ = {std::max(1, size.width), std::max(1, size.height)};
    cells_.assign(static_cast<std::size_t>(size_.height), std::vector<Cell>(static_cast<std::size_t>(size_.width)));
}

std::optional<Event> MemoryTerminalBackend::poll_event() {
    if (!events_.empty()) {
        Event event = std::move(events_.front());
        events_.pop_front();
        return event;
    }

    if (poll_timeout_ms_ == 0) {
        return std::nullopt;
    }

    return std::nullopt;
}

std::optional<Event> MemoryTerminalBackend::poll_event_nonblocking() {
    if (events_.empty()) {
        return std::nullopt;
    }

    Event event = std::move(events_.front());
    events_.pop_front();
    return event;
}

void MemoryTerminalBackend::begin_frame(BeginFrameOptions options) {
    image_draws_.clear();

    if (options.full_redraw) {
        if (options.clear_buffer) {
            for (auto& row : cells_) {
                for (Cell& cell : row) {
                    cell = Cell{};
                }
            }
        }
        return;
    }

    Rect region = intersect(options.dirty_region, {{0, 0}, size_});
    if (region.width <= 0 || region.height <= 0) {
        return;
    }

    for (int y = region.y; y < region.bottom(); ++y) {
        for (int x = region.x; x < region.right(); ++x) {
            cells_[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] = Cell{};
        }
    }
}

void MemoryTerminalBackend::end_frame() {}

void MemoryTerminalBackend::set_text_cursor(std::optional<Point> position) { text_cursor_ = position; }

void MemoryTerminalBackend::ensure_cell(int x, int y) {
    if (y < 0 || y >= size_.height || x < 0 || x >= size_.width) {
        return;
    }

    cells_[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] = Cell{};
}

void MemoryTerminalBackend::draw_text(int x, int y, std::string_view text, Style style) {
    if (text.empty() || y < 0 || y >= size_.height) {
        return;
    }

    int column = x;
    for (char ch : text) {
        if (ch == '\n') {
            break;
        }
        if (column >= size_.width) {
            break;
        }
        if (column >= 0) {
            cells_[static_cast<std::size_t>(y)][static_cast<std::size_t>(column)] = {ch, style};
        }
        ++column;
    }
}

void MemoryTerminalBackend::draw_image(int x, int y, Size cell_size, const TerminalImage& image) {
    if (image.empty()) {
        return;
    }

    image_draws_.push_back(ImageDraw{x, y, cell_size, image});
}

void MemoryTerminalBackend::set_poll_timeout_ms(int timeout_ms) { poll_timeout_ms_ = timeout_ms; }

void MemoryTerminalBackend::push_event(Event event) { events_.push_back(std::move(event)); }

std::string MemoryTerminalBackend::snapshot() const {
    std::ostringstream out;
    for (int row = 0; row < size_.height; ++row) {
        for (int col = 0; col < size_.width; ++col) {
            out << cells_[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)].ch;
        }
        if (row + 1 < size_.height) {
            out << '\n';
        }
    }
    return out.str();
}

std::unique_ptr<MemoryTerminalBackend> MemoryTerminalBackend::create(Size size) {
    return std::make_unique<MemoryTerminalBackend>(size);
}

} // namespace tuinator
