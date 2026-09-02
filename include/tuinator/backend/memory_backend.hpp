#pragma once

#include <tuinator/backend/terminal_backend.hpp>
#include <tuinator/render/terminal_image.hpp>

#include <deque>
#include <optional>
#include <string>
#include <vector>

namespace tuinator {

// In-memory terminal backend for tests and snapshot rendering (no real TTY).
class MemoryTerminalBackend : public TerminalBackend {
public:
    struct Cell {
        char ch = ' ';
        Style style{};
    };

    struct ImageDraw {
        int x = 0;
        int y = 0;
        Size cell_size{};
        TerminalImage image;
    };

    explicit MemoryTerminalBackend(Size size = {80, 24});

    void init() override;
    void shutdown() override;

    Size terminal_size() const override;
    std::optional<Event> poll_event() override;
    std::optional<Event> poll_event_nonblocking() override;

    void begin_frame() override;
    void end_frame() override;

    void draw_text(int x, int y, std::string_view text, Style style) override;
    void draw_image(int x, int y, Size cell_size, const TerminalImage& image) override;

    void set_poll_timeout_ms(int timeout_ms) override;

    void push_event(Event event);
    void resize(Size size);

    const std::vector<std::vector<Cell>>& cells() const { return cells_; }
    const std::vector<ImageDraw>& image_draws() const { return image_draws_; }
    std::string snapshot() const;

    static std::unique_ptr<MemoryTerminalBackend> create(Size size = {80, 24});

private:
    void ensure_cell(int x, int y);

    Size size_;
    std::vector<std::vector<Cell>> cells_;
    std::vector<ImageDraw> image_draws_;
    std::deque<Event> events_;
    int poll_timeout_ms_ = -1;
    bool initialized_ = false;
};

} // namespace tuinator
