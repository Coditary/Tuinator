#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/terminal/ansi_terminal_buffer.hpp>
#include <tuinator/terminal/pty_session.hpp>
#include <tuinator/widgets/widget.hpp>

#include <atomic>
#include <functional>
#include <mutex>
#include <string>

namespace tuinator {

struct ShellTerminalStyle {
    Style default_cell{};
    Style cursor{};
};

/// Interactive shell backed by a real PTY (your $SHELL, prompt, aliases, …).
class ShellTerminal : public Widget {
  public:
    explicit ShellTerminal(ShellTerminalStyle style = {});

    bool start(const std::string& shell_command = {});
    bool is_running() const { return pty_.is_running(); }

    void set_on_exit(std::function<void()> callback) { on_exit_ = std::move(callback); }

    const AnsiTerminalBuffer& buffer() const { return buffer_; }

    std::string_view widget_type_name() const override { return "ShellTerminal"; }

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    void on_idle() override;
    bool needs_periodic_idle() const override { return pty_.is_running(); }
    bool wants_full_screen() const override { return true; }
    bool wants_initial_focus() const override { return true; }
    bool is_shell_terminal() const override { return true; }
    bool is_focusable() const override { return true; }

  private:
    void on_pty_output(std::string_view bytes);
    void on_pty_exit();

    ShellTerminalStyle style_;
    PtySession pty_;
    AnsiTerminalBuffer buffer_;
    mutable std::mutex buffer_mutex_;
    std::atomic<bool> output_pending_{false};
    std::function<void()> on_exit_;
};

} // namespace tuinator
