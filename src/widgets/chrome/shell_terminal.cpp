#include <tuinator/widgets/chrome/shell_terminal.hpp>

#include <optional>
#include <variant>

namespace tuinator {

ShellTerminal::ShellTerminal(ShellTerminalStyle style) : style_(std::move(style)) {
    pty_.set_output_handler([this](std::string_view bytes) { on_pty_output(bytes); });
    pty_.set_exit_handler([this] { on_pty_exit(); });
}

bool ShellTerminal::start(const std::string& shell_command) {
    const Size size{
        std::max(1, bounds_.width),
        std::max(1, bounds_.height),
    };
    {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        buffer_.resize(size);
    }
    if (!pty_.start(size, shell_command)) {
        on_pty_output("Failed to start shell PTY.\r\n");
        return false;
    }
    mark_dirty();
    return true;
}

Size ShellTerminal::preferred_size() const {
    return {80, 24};
}

void ShellTerminal::layout(Rect bounds) {
    bounds_ = bounds;
    const Size size{
        std::max(1, bounds.width),
        std::max(1, bounds.height),
    };
    {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        buffer_.resize(size);
    }
    if (pty_.is_running()) {
        pty_.resize(size);
    } else if (bounds.width > 0 && bounds.height > 0) {
        start();
    }
}

void ShellTerminal::paint(PaintContext& ctx) const {
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    const Point origin{0, 0};

    std::lock_guard<std::mutex> lock(buffer_mutex_);
    buffer_.paint(ctx, origin);

    int cursor_row = 0;
    int cursor_col = 0;
    bool cursor_visible = false;
    buffer_.cursor_position(cursor_row, cursor_col, cursor_visible);
    if (is_focused() && cursor_visible) {
        ctx.canvas.set_text_cursor(Point{cursor_col, cursor_row});
    } else {
        ctx.canvas.set_text_cursor(std::nullopt);
    }
}

void ShellTerminal::on_idle() {
    if (output_pending_.exchange(false, std::memory_order_acq_rel)) {
        mark_dirty();
    }
}

void ShellTerminal::on_pty_output(std::string_view bytes) {
    if (bytes.empty()) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        buffer_.feed(bytes);
    }
    output_pending_.store(true, std::memory_order_release);
}

void ShellTerminal::on_pty_exit() {
    output_pending_.store(true, std::memory_order_release);
    if (on_exit_) {
        on_exit_();
    }
}

bool ShellTerminal::handle_event(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (!bounds_.contains(mouse->position)) {
            return false;
        }
        return false;
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key) {
        return false;
    }

    if (!is_focused()) {
        return false;
    }

    if (!pty_.is_running()) {
        if (!start()) {
            return false;
        }
    }

    std::string bytes;
    {
        std::lock_guard<std::mutex> lock(buffer_mutex_);
        bytes = buffer_.keyboard_bytes(*key);
    }
    if (bytes.empty()) {
        return false;
    }

    return pty_.enqueue_input(std::move(bytes));
}

} // namespace tuinator
