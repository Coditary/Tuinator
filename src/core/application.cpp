#include <tuinator/backend/inline_backend.hpp>
#include <tuinator/core/application.hpp>
#include <tuinator/render/style_resolver.hpp>
#include <tuinator/debug/debug_paint.hpp>
#include <tuinator/debug/startup_profiler.hpp>
#include <tuinator/render/graphics_protocol.hpp>
#include <tuinator/render/paint_context.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/capabilities.hpp>
#include <tuinator/widgets/containers/split_pane.hpp>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <variant>

namespace tuinator {

namespace {

const char* mouse_action_name(MouseAction action) {
    switch (action) {
    case MouseAction::Press: return "press";
    case MouseAction::Release: return "release";
    case MouseAction::Click: return "click";
    case MouseAction::Move: return "move";
    case MouseAction::WheelUp: return "wheel_up";
    case MouseAction::WheelDown: return "wheel_down";
    case MouseAction::WheelLeft: return "wheel_left";
    case MouseAction::WheelRight: return "wheel_right";
    }
    return "unknown";
}

void debug_mouse_dispatch(const MouseEvent& mouse, bool hit, bool handled) {
    const char* debug = std::getenv("TUINATOR_MOUSE_DEBUG");
    if (debug == nullptr || debug[0] == '\0' || std::strcmp(debug, "0") == 0) {
        return;
    }

    if (FILE* log = std::fopen("/tmp/tuinator-mouse.log", "a")) {
        std::fprintf(log, "tuinator-mouse: dispatch action=%s x=%d y=%d hit=%d handled=%d\n",
                     mouse_action_name(mouse.action), mouse.position.x, mouse.position.y, hit ? 1 : 0, handled ? 1 : 0);
        std::fclose(log);
    }
}

bool is_mouse_interaction(const MouseEvent& mouse) {
    return mouse.action == MouseAction::Press || mouse.action == MouseAction::Release ||
           mouse.action == MouseAction::Click;
}

bool is_viewport_scroll_key(const KeyPress& key) {
    switch (key.key) {
    case Key::PageUp:
    case Key::PageDown:
    case Key::Home:
    case Key::End: return true;
    default: return false;
    }
}

bool widget_wants_hover_redraw(Widget* root, Point position) {
    if (root == nullptr) {
        return false;
    }

    Widget* target = root->hit_test(position);
    return target != nullptr && target->wants_hover_redraw();
}

bool widget_is_shell_terminal(const Widget* widget) { return widget != nullptr && widget->is_shell_terminal(); }

} // namespace

Application::Application() : Application(TerminalBackend::create()) {}

Application::Application(std::unique_ptr<TerminalBackend> backend)
    : backend_(std::move(backend)), theme_(dark_theme()) {}

Application::~Application() {
    if (backend_ && terminal_ready_) {
        backend_->shutdown();
    }
}

void Application::ensure_terminal() {
    if (terminal_ready_) {
        return;
    }

    startup_profile_mark("application.before_backend_init");
    backend_->init();
    terminal_ready_ = true;
    sync_mouse_cursor_policy();
    startup_profile_mark("application.after_backend_init");
}

void Application::set_theme(Theme theme) {
    theme_ = std::move(theme);
    request_redraw();
}

void Application::set_stylesheet(Stylesheet stylesheet) {
    stylesheet_ = std::move(stylesheet);
    ThemeOptions options = stylesheet_->theme_options();
    if (options.border_style || options.glyphs != GlyphSet::Auto) {
        set_theme(dark_theme(options));
    }
    sync_stylesheet();
    request_redraw();
}

void Application::load_stylesheet(const std::filesystem::path& path) {
    set_stylesheet(Stylesheet::load_from_file(path));
}

const Stylesheet* Application::stylesheet() const {
    return stylesheet_.has_value() ? &*stylesheet_ : nullptr;
}

void Application::sync_stylesheet() {
    if (!root_ || !stylesheet_.has_value()) {
        stylesheet_warnings_.clear();
        return;
    }

    const StyleResolver resolver(theme_, stylesheet());
    stylesheet_warnings_.clear();
    apply_stylesheet_to_tree(*root_, resolver, &stylesheet_warnings_);
    for (const std::string& warning : stylesheet_warnings_) {
        std::cerr << "[tuinator:stylesheet] " << warning << '\n';
    }
}

void Application::update_hover(Point position) {
    Widget* target = nullptr;
    if (root_ != nullptr) {
        Widget* hit = root_->hit_test(position);
        if (hit != nullptr && hit->wants_hover()) {
            target = hit;
        }
    }

    if (hovered_widget_ == target) {
        return;
    }

    if (hovered_widget_ != nullptr) {
        hovered_widget_->set_hovered(false);
    }

    hovered_widget_ = target;

    if (hovered_widget_ != nullptr) {
        hovered_widget_->set_hovered(true);
    }

    request_redraw();
}

void Application::set_root(std::unique_ptr<Widget> root) {
    startup_profile_mark("application.before_set_root");
    root_ = std::move(root);

    if (root_) {
        root_->set_on_dirty([this](Rect region) { request_redraw(region); });
        root_->set_on_layout([this]() {
            layout_root();
            request_redraw();
        });
        sync_stylesheet();

        if (terminal_ready_) {
            layout_root();
            rebuild_focus_list();
            update_focus();
        } else {
            sync_mouse_cursor_policy();
        }
    } else {
        sync_mouse_cursor_policy();
    }

    request_redraw();
    startup_profile_mark("application.after_set_root");
}

Size Application::terminal_size() const {
    if (!terminal_ready_) {
        return {80, 24};
    }
    return backend_->terminal_size();
}

void Application::quit() { running_ = false; }

TimerId Application::set_interval(int interval_ms, std::function<void()> callback) {
    TimerEntry entry{};
    entry.id = next_timer_id_++;
    entry.interval_ms = std::max(1, interval_ms);
    entry.repeat = true;
    entry.callback = std::move(callback);
    entry.next_fire = std::chrono::steady_clock::now() + std::chrono::milliseconds(entry.interval_ms);
    timers_.push_back(std::move(entry));
    return timers_.back().id;
}

TimerId Application::set_timeout(int interval_ms, std::function<void()> callback) {
    TimerEntry entry{};
    entry.id = next_timer_id_++;
    entry.interval_ms = std::max(1, interval_ms);
    entry.repeat = false;
    entry.callback = std::move(callback);
    entry.next_fire = std::chrono::steady_clock::now() + std::chrono::milliseconds(entry.interval_ms);
    timers_.push_back(std::move(entry));
    return timers_.back().id;
}

void Application::cancel_timer(TimerId id) {
    for (TimerEntry& timer : timers_) {
        if (timer.id == id) {
            timer.active = false;
            break;
        }
    }

    timers_.erase(std::remove_if(timers_.begin(), timers_.end(), [](const TimerEntry& timer) { return !timer.active; }),
                  timers_.end());
}

int Application::compute_poll_timeout_ms() const {
    constexpr int kPeriodicIdlePollMs = 16;
    int timeout = backend_->pointer_active() ? kPeriodicIdlePollMs : -1;

    if (timeout < 0 && any_widget_needs_periodic_idle()) {
        timeout = kPeriodicIdlePollMs;
    }

    const auto now = std::chrono::steady_clock::now();
    int min_timer_ms = -1;

    for (const TimerEntry& timer : timers_) {
        if (!timer.active) {
            continue;
        }

        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(timer.next_fire - now).count();
        const int wait_ms = static_cast<int>(std::max<std::int64_t>(0, remaining));
        if (min_timer_ms < 0 || wait_ms < min_timer_ms) {
            min_timer_ms = wait_ms;
        }
    }

    if (min_timer_ms < 0) {
        return timeout;
    }

    if (timeout < 0) {
        return min_timer_ms;
    }

    return std::min(timeout, min_timer_ms);
}

bool Application::any_widget_needs_periodic_idle() const {
    if (!root_) {
        return false;
    }

    bool needed = false;
    root_->for_each_descendant([&needed](Widget* widget) {
        if (widget->needs_periodic_idle()) {
            needed = true;
        }
    });
    return needed;
}

void Application::process_timers() {
    const auto now = std::chrono::steady_clock::now();
    bool fired = false;

    for (TimerEntry& timer : timers_) {
        if (!timer.active || now < timer.next_fire) {
            continue;
        }

        if (timer.callback) {
            timer.callback();
        }

        fired = true;

        if (timer.repeat) {
            timer.next_fire = now + std::chrono::milliseconds(timer.interval_ms);
        } else {
            timer.active = false;
        }
    }

    timers_.erase(std::remove_if(timers_.begin(), timers_.end(), [](const TimerEntry& timer) { return !timer.active; }),
                  timers_.end());

    if (fired && !dirty_region_.needs_render()) {
        request_redraw();
    }
}

void Application::refresh_focus() {
    rebuild_focus_list();
    update_focus();
}

void Application::request_redraw() { dirty_region_.mark_full(); }

void Application::request_redraw(Rect region) {
    if (region.width <= 0 || region.height <= 0) {
        dirty_region_.mark_full();
        return;
    }

    dirty_region_.mark_rect(region);
}

void Application::poll_idle() {
    if (!root_) {
        return;
    }

    root_->for_each_descendant([](Widget* widget) { widget->on_idle(); });
}

void Application::layout_root() {
    if (!root_) {
        return;
    }

    if (root_) {
        const Rect terminal{{0, 0}, terminal_size()};
        if (root_->wants_full_screen()) {
            root_->layout(terminal);
        } else {
            const auto preferred = root_->preferred_size();
            const Rect placed{
                0,
                0,
                std::min(preferred.width, terminal.width),
                std::min(preferred.height, terminal.height),
            };
            root_->layout(placed);
        }
    }
}

void Application::rebuild_focus_list() {
    focusable_.clear();
    if (root_) {
        root_->collect_focusable(focusable_);
    }

    if (focus_index_ >= focusable_.size()) {
        focus_index_ = 0;
    }

    for (std::size_t i = 0; i < focusable_.size(); ++i) {
        if (focusable_[i]->wants_initial_focus()) {
            focus_index_ = i;
            break;
        }
    }
}

void Application::sync_mouse_cursor_policy() { backend_->set_mouse_cursor_suppressed(shell_terminal_active()); }

bool Application::shell_terminal_active() const {
    if (widget_is_shell_terminal(root_.get())) {
        return true;
    }

    if (focusable_.empty() || focus_index_ >= focusable_.size()) {
        return false;
    }

    return widget_is_shell_terminal(focusable_[focus_index_]);
}

void Application::update_focus() {
    for (Widget* widget : focusable_) {
        widget->set_focused(false);
    }

    if (!focusable_.empty()) {
        focusable_[focus_index_]->set_focused(true);
        ensure_focus_visible(root_.get(), focusable_[focus_index_]);
    }

    sync_mouse_cursor_policy();
    request_redraw();
}

void Application::focus_next() {
    if (focusable_.empty()) {
        return;
    }

    focus_index_ = (focus_index_ + 1) % focusable_.size();
    update_focus();
}

void Application::focus_prev() {
    if (focusable_.empty()) {
        return;
    }

    focus_index_ = (focus_index_ + focusable_.size() - 1) % focusable_.size();
    update_focus();
}

int Application::run_headless() {
    char buffer[64];
    while (true) {
        const ssize_t bytes = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (bytes <= 0) {
            break;
        }

        for (ssize_t i = 0; i < bytes; ++i) {
            if (buffer[i] == 'q' || buffer[i] == 'Q') {
                return 0;
            }
        }
    }

    return 0;
}

bool Application::should_run_headless() const {
    if (dynamic_cast<const InlineTerminalBackend*>(backend_.get()) != nullptr) {
        return false;
    }

    if (!isatty(STDIN_FILENO)) {
        return true;
    }

    const char* headless = std::getenv("TUINATOR_HEADLESS");
    return headless != nullptr && headless[0] != '\0' && std::strcmp(headless, "0") != 0;
}

int Application::run() {
    if (should_run_headless()) {
        return run_headless();
    }

    ensure_terminal();
    running_ = true;
    startup_profile_mark("run.start");

    layout_root();
    rebuild_focus_list();
    update_focus();
    request_redraw();
    render();
    startup_profile_mark("run.after_first_render");

    bool first_poll = true;
    while (running_) {
        if (first_poll) {
            startup_profile_mark("run.before_first_poll");
        }

        backend_->set_poll_timeout_ms(compute_poll_timeout_ms());

        if (const auto event = backend_->poll_event()) {
            if (first_poll) {
                startup_profile_mark("run.after_first_poll");
                first_poll = false;
            }
            handle_event(*event);
            while (const auto pending = backend_->poll_event_nonblocking()) {
                handle_event(*pending);
            }
            poll_idle();
        } else {
            process_timers();
        }

        poll_idle();

        render();
    }

    startup_profile_mark("run.exit");
    return 0;
}

void Application::present() {
    if (should_run_headless()) {
        return;
    }

    ensure_terminal();
    layout_root();
    rebuild_focus_list();
    update_focus();
    request_redraw();
    render();
}

void Application::shutdown_terminal() {
    if (backend_ && terminal_ready_) {
        backend_->shutdown();
        terminal_ready_ = false;
    }
}

void Application::set_alternate_screen(bool enabled) {
    if (backend_) {
        backend_->set_alternate_screen(enabled);
    }
}

void Application::set_clear_on_shutdown(bool enabled) {
    if (backend_) {
        backend_->set_clear_on_shutdown(enabled);
    }
}

void Application::focus_widget(Widget* widget) {
    if (!widget) {
        return;
    }

    rebuild_focus_list();
    for (std::size_t i = 0; i < focusable_.size(); ++i) {
        if (focusable_[i] == widget) {
            focus_index_ = i;
            update_focus();
            return;
        }
    }
}

void Application::handle_event(const Event& event) {
    if (const auto* resize = std::get_if<Resize>(&event)) {
        (void)resize;
        backend_->invalidate_graphics();
        layout_root();
        request_redraw();
        return;
    }

    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        update_hover(mouse->position);

        const bool hit = root_ && root_->hit_test(mouse->position) != nullptr;
        bool handled = false;

        if (is_mouse_interaction(*mouse) && root_) {
            if (Widget* target = root_->hit_test_focusable(mouse->position)) {
                focus_widget(target);
            }
        }

        if (Widget* capture = find_pointer_active_widget(root_.get())) {
            const MouseEvent adjusted = adjust_mouse_for_widget(root_.get(), capture, *mouse);
            handled = capture->handle_event(adjusted);
        } else if (root_) {
            handled = root_->handle_event(event);
        }

        // Pure hover motion should not erase and repaint the whole frame — that
        // flickers text and re-places Kitty graphics on every pixel of movement.
        const bool motion_while_dragging = mouse->action == MouseAction::Move && mouse->left_pressed;
        const bool hover_needs_redraw =
            mouse->action == MouseAction::Move && widget_wants_hover_redraw(root_.get(), mouse->position);
        const bool needs_redraw =
            mouse->action != MouseAction::Move || motion_while_dragging || handled || hover_needs_redraw;
        if (needs_redraw) {
            request_redraw();
        } else if (mouse->action == MouseAction::Move) {
            backend_->refresh_mouse_cursor();
        }

        debug_mouse_dispatch(*mouse, hit, handled);
        return;
    }

    if (const auto* key = std::get_if<KeyPress>(&event)) {
        const bool shell_focused = !focusable_.empty() && focus_index_ < focusable_.size() &&
                                   focusable_[focus_index_] != nullptr && focusable_[focus_index_]->is_shell_terminal();

        if (key->character == 17 && !shell_focused) {
            quit();
            return;
        }

        if (key->key == Key::Tab || key->key == Key::BackTab) {
            if (root_ && root_->handle_event(event)) {
                request_redraw();
                return;
            }

            if (key->key == Key::Tab) {
                focus_next();
            } else {
                focus_prev();
            }
            return;
        }

        if (dispatch_keyboard_capture(root_.get(), event)) {
            if (!shell_terminal_active()) {
                request_redraw();
            }
            return;
        }

        if (root_ && root_->handle_event(event)) {
            if (!shell_terminal_active()) {
                request_redraw();
            }
            return;
        }

        if (shell_focused) {
            return;
        }

        if (key->character == 'q' || key->character == 'Q') {
            quit();
            return;
        }

        if (key->key == Key::Escape) {
            quit();
            return;
        }

        if (root_ && key->character >= '1' && key->character <= '9') {
            const int tab_index = key->character - '1';
            if (dispatch_tab_number(root_.get(), tab_index)) {
                layout_root();
                refresh_focus();
                request_redraw();
                return;
            }
        }

        if (root_) {
            Widget* focused = focusable_.empty() ? nullptr : focusable_[focus_index_];

            if (key->key == Key::Up || key->key == Key::Down) {
                const int delta = key->key == Key::Down ? 1 : -1;
                Scrollable* scroll = find_scrollable_for_widget(root_.get(), focused);

                if (scroll != nullptr && focused != nullptr && scroll->contains_widget(focused)) {
                    std::vector<Widget*> items;
                    if (Widget* content = scroll->scroll_content()) {
                        content->collect_focusable(items);
                    }
                    const auto it = std::find(items.begin(), items.end(), focused);
                    if (it != items.end()) {
                        const int index = static_cast<int>(it - items.begin());
                        const int next = index + delta;
                        if (next >= 0 && next < static_cast<int>(items.size())) {
                            focus_widget(items[static_cast<std::size_t>(next)]);
                            return;
                        }

                        scroll->scroll_by(0, delta);
                        request_redraw();
                        return;
                    }
                }

                if (Scrollable* any_scroll = find_first_scrollable(root_.get())) {
                    any_scroll->scroll_by(0, delta);
                    request_redraw();
                    return;
                }
            }

            if (is_viewport_scroll_key(*key)) {
                if (dispatch_scroll_keys(root_.get(), focused, event)) {
                    request_redraw();
                    return;
                }
            }
        }
    }
}

void Application::render() {
    if (!terminal_ready_ || !dirty_region_.needs_render()) {
        return;
    }

    const Size term = terminal_size();
    const Rect terminal_bounds{{0, 0}, term};

    const bool shell_active = shell_terminal_active();

    BeginFrameOptions frame;
    frame.full_redraw = true;
    frame.clear_buffer = true;
    frame.dirty_region = terminal_bounds;
    Rect paint_clip = terminal_bounds;

    const bool inline_backend = dynamic_cast<const InlineTerminalBackend*>(backend_.get()) != nullptr;
    const bool use_partial = !dirty_region_.is_full() && !shell_active && !inline_backend;

    if (use_partial) {
        paint_clip = intersect(dirty_region_.bounds(), terminal_bounds);
        if (paint_clip.width <= 0 || paint_clip.height <= 0) {
            dirty_region_.clear();
            return;
        }

        frame.full_redraw = false;
        frame.dirty_region = paint_clip;
    }

    debug_paint_begin_frame();

    startup_profile_mark("render.begin_frame");
    backend_->begin_frame(frame);

    if (root_) {
        Canvas canvas(*backend_);
        canvas.set_glyphs(theme_.glyphs);
        canvas.with_clip(paint_clip, [&](Canvas& clipped) {
            PaintContext clipped_ctx{clipped, theme_, stylesheet()};
            root_->paint(clipped_ctx);
            paint_split_dividers(*root_, clipped_ctx);
        });
    }

    startup_profile_mark("render.before_refresh");
    backend_->end_frame();
    startup_profile_mark("render.after_refresh");
    debug_paint_log_frame(use_partial, paint_clip);
    dirty_region_.clear();
}

} // namespace tuinator
