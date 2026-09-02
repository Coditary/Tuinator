#pragma once

#include <tuinator/backend/terminal_backend.hpp>
#include <tuinator/core/geometry.hpp>
#include <tuinator/render/canvas.hpp>
#include <tuinator/render/dirty_region.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/widget.hpp>

#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <vector>

namespace tuinator {

using TimerId = int;

class Application {
public:
    Application();
    explicit Application(std::unique_ptr<TerminalBackend> backend);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void set_root(std::unique_ptr<Widget> root);
    Widget* root() const { return root_.get(); }

    void set_theme(Theme theme);
    const Theme& theme() const { return theme_; }

    TerminalBackend& backend() { return *backend_; }
    const TerminalBackend& backend() const { return *backend_; }

    Size terminal_size() const;
    void quit();
    void refresh_focus();
    int run();

    TimerId set_interval(int interval_ms, std::function<void()> callback);
    TimerId set_timeout(int interval_ms, std::function<void()> callback);
    void cancel_timer(TimerId id);

    // Draw one frame immediately (useful for profiling and tests).
    void present();

private:
    struct TimerEntry {
        TimerId id = 0;
        int interval_ms = 0;
        bool repeat = false;
        bool active = true;
        std::function<void()> callback;
        std::chrono::steady_clock::time_point next_fire{};
    };

    void ensure_terminal();
    void request_redraw();
    void request_redraw(Rect region);
    int run_headless();
    bool should_run_headless() const;
    void layout_root();
    void rebuild_focus_list();
    void update_focus();
    void focus_next();
    void focus_prev();
    void focus_widget(Widget* widget);
    void handle_event(const Event& event);
    void render();
    void process_timers();
    void poll_idle();
    int compute_poll_timeout_ms() const;
    bool any_widget_needs_periodic_idle() const;
    bool shell_terminal_active() const;
    void sync_mouse_cursor_policy();

    std::unique_ptr<TerminalBackend> backend_;
    std::unique_ptr<Widget> root_;
    Theme theme_;
    std::vector<Widget*> focusable_;
    std::vector<TimerEntry> timers_;
    TimerId next_timer_id_ = 1;
    std::size_t focus_index_ = 0;
    bool running_ = false;
    DirtyRegion dirty_region_;
    bool terminal_ready_ = false;
    bool clear_framebuffer_ = false;
};

} // namespace tuinator
