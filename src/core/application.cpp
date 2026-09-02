#include <tuinator/core/application.hpp>

#include <tuinator/debug/startup_profiler.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/scroll_view.hpp>
#include <tuinator/widgets/panel.hpp>
#include <tuinator/widgets/split_pane.hpp>
#include <tuinator/widgets/tabs.hpp>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <variant>

namespace tuinator {

namespace {

const char* mouse_action_name(MouseAction action) {
    switch (action) {
    case MouseAction::Press:   return "press";
    case MouseAction::Release: return "release";
    case MouseAction::Click:   return "click";
    case MouseAction::Move:    return "move";
    case MouseAction::WheelUp:   return "wheel_up";
    case MouseAction::WheelDown: return "wheel_down";
    }
    return "unknown";
}

void debug_mouse_dispatch(const MouseEvent& mouse, bool hit, bool handled) {
    const char* debug = std::getenv("TUINATOR_MOUSE_DEBUG");
    if (debug == nullptr || debug[0] == '\0' || std::strcmp(debug, "0") == 0) {
        return;
    }

    if (FILE* log = std::fopen("/tmp/tuinator-mouse.log", "a")) {
        std::fprintf(log,
                     "tuinator-mouse: dispatch action=%s x=%d y=%d hit=%d handled=%d\n",
                     mouse_action_name(mouse.action),
                     mouse.position.x,
                     mouse.position.y,
                     hit ? 1 : 0,
                     handled ? 1 : 0);
        std::fclose(log);
    }
}

bool is_mouse_interaction(const MouseEvent& mouse) {
    return mouse.action == MouseAction::Press
        || mouse.action == MouseAction::Release
        || mouse.action == MouseAction::Click;
}

void ensure_focus_visible(Widget* node, Widget* target) {
    if (node == nullptr || target == nullptr) {
        return;
    }

    if (auto* scroll = dynamic_cast<ScrollView*>(node)) {
        if (scroll->contains_widget(target)) {
            scroll->ensure_visible(target);
            return;
        }

        if (Widget* content = scroll->content()) {
            ensure_focus_visible(content, target);
        }
        return;
    }

    for (const auto& child : node->children()) {
        ensure_focus_visible(child.get(), target);
    }
}

bool is_viewport_scroll_key(const KeyPress& key) {
    switch (key.key) {
    case Key::PageUp:
    case Key::PageDown:
    case Key::Home:
    case Key::End:
        return true;
    default:
        return false;
    }
}

ScrollView* find_scroll_view_for_widget(Widget* node, const Widget* target) {
    if (node == nullptr || target == nullptr) {
        return nullptr;
    }

    if (auto* scroll = dynamic_cast<ScrollView*>(node)) {
        if (scroll->contains_widget(target)) {
            return scroll;
        }

        if (Widget* content = scroll->content()) {
            if (ScrollView* nested = find_scroll_view_for_widget(content, target)) {
                return nested;
            }
        }
        return nullptr;
    }

    for (const auto& child : node->children()) {
        if (ScrollView* found = find_scroll_view_for_widget(child.get(), target)) {
            return found;
        }
    }

    return nullptr;
}

ScrollView* find_first_scroll_view(Widget* node) {
    if (node == nullptr) {
        return nullptr;
    }

    if (auto* scroll = dynamic_cast<ScrollView*>(node)) {
        return scroll;
    }

    for (const auto& child : node->children()) {
        if (ScrollView* found = find_first_scroll_view(child.get())) {
            return found;
        }
    }

    return nullptr;
}

Widget* find_pointer_active_widget(Widget* node) {
    if (node == nullptr) {
        return nullptr;
    }

    if (node->pointer_active()) {
        return node;
    }

    if (auto* panel = dynamic_cast<Panel*>(node)) {
        if (Widget* content = panel->content()) {
            if (Widget* found = find_pointer_active_widget(content)) {
                return found;
            }
        }
    }

    if (auto* scroll = dynamic_cast<ScrollView*>(node)) {
        if (Widget* content = scroll->content()) {
            if (Widget* found = find_pointer_active_widget(content)) {
                return found;
            }
        }
    }

    for (const auto& child : node->children()) {
        if (Widget* found = find_pointer_active_widget(child.get())) {
            return found;
        }
    }

    return nullptr;
}

MouseEvent adjust_mouse_for_widget(Widget* root, Widget* target, const MouseEvent& mouse) {
    MouseEvent adjusted = mouse;
    if (ScrollView* scroll = find_scroll_view_for_widget(root, target)) {
        adjusted.position.y += scroll->scroll_y();
    }
    return adjusted;
}

bool dispatch_scroll_keys(Widget* root, Widget* focused, const Event& event) {
    ScrollView* scroll = find_scroll_view_for_widget(root, focused);
    if (scroll == nullptr) {
        scroll = find_first_scroll_view(root);
    }

    return scroll != nullptr && scroll->try_scroll(event);
}

bool dispatch_tab_number(Widget* node, int index) {
    if (node == nullptr || index < 0) {
        return false;
    }

    Tabs* tabs = nullptr;

    const auto visit = [&](Widget* current, const auto& visit_ref) -> void {
        if (current == nullptr || tabs != nullptr) {
            return;
        }

        if (auto* found = dynamic_cast<Tabs*>(current)) {
            tabs = found;
            return;
        }

        for (const auto& child : current->children()) {
            visit_ref(child.get(), visit_ref);
        }

        if (auto* panel = dynamic_cast<Panel*>(current)) {
            visit_ref(panel->content(), visit_ref);
        }

        if (auto* split = dynamic_cast<SplitPane*>(current)) {
            visit_ref(split->first(), visit_ref);
            visit_ref(split->second(), visit_ref);
        }

        if (auto* scroll = dynamic_cast<ScrollView*>(current)) {
            visit_ref(scroll->content(), visit_ref);
        }
    };

    visit(node, visit);

    if (tabs == nullptr || index >= static_cast<int>(tabs->tab_count())) {
        return false;
    }

    tabs->set_selected_index(index);
    return true;
}

} // namespace

Application::Application()
    : Application(TerminalBackend::create()) {}

Application::Application(std::unique_ptr<TerminalBackend> backend)
    : backend_(std::move(backend)),
      theme_(dark_theme()) {}

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
    startup_profile_mark("application.after_backend_init");
}

void Application::set_theme(Theme theme) {
    theme_ = std::move(theme);
    request_redraw();
}

void Application::set_root(std::unique_ptr<Widget> root) {
    startup_profile_mark("application.before_set_root");
    root_ = std::move(root);

    if (root_) {
        root_->set_on_dirty([this]() { request_redraw(); });
        root_->set_on_layout([this]() {
            layout_root();
            request_redraw();
        });

        if (terminal_ready_) {
            layout_root();
            rebuild_focus_list();
            update_focus();
        }
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

void Application::quit() {
    running_ = false;
}

TimerId Application::set_interval(int interval_ms, std::function<void()> callback) {
    TimerEntry entry{};
    entry.id = next_timer_id_++;
    entry.interval_ms = std::max(1, interval_ms);
    entry.repeat = true;
    entry.callback = std::move(callback);
    entry.next_fire =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(entry.interval_ms);
    timers_.push_back(std::move(entry));
    return timers_.back().id;
}

TimerId Application::set_timeout(int interval_ms, std::function<void()> callback) {
    TimerEntry entry{};
    entry.id = next_timer_id_++;
    entry.interval_ms = std::max(1, interval_ms);
    entry.repeat = false;
    entry.callback = std::move(callback);
    entry.next_fire =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(entry.interval_ms);
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

    timers_.erase(
        std::remove_if(timers_.begin(), timers_.end(),
                       [](const TimerEntry& timer) { return !timer.active; }),
        timers_.end());
}

int Application::compute_poll_timeout_ms() const {
    int timeout = backend_->pointer_active() ? 16 : -1;

    const auto now = std::chrono::steady_clock::now();
    int min_timer_ms = -1;

    for (const TimerEntry& timer : timers_) {
        if (!timer.active) {
            continue;
        }

        const auto remaining =
            std::chrono::duration_cast<std::chrono::milliseconds>(timer.next_fire - now).count();
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

    timers_.erase(
        std::remove_if(timers_.begin(), timers_.end(),
                       [](const TimerEntry& timer) { return !timer.active; }),
        timers_.end());

    if (fired) {
        request_redraw();
    }
}

void Application::refresh_focus() {
    rebuild_focus_list();
    update_focus();
}

void Application::request_redraw() {
    dirty_ = true;
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
}

void Application::update_focus() {
    for (Widget* widget : focusable_) {
        widget->set_focused(false);
    }

    if (!focusable_.empty()) {
        focusable_[focus_index_]->set_focused(true);
        ensure_focus_visible(root_.get(), focusable_[focus_index_]);
    }

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
        } else {
            process_timers();
        }

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
        layout_root();
        request_redraw();
        return;
    }

    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
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
        const bool motion_while_dragging =
            mouse->action == MouseAction::Move && mouse->left_pressed;
        const bool needs_redraw =
            mouse->action != MouseAction::Move || motion_while_dragging || handled;
        if (needs_redraw) {
            request_redraw();
        } else if (mouse->action == MouseAction::Move) {
            backend_->refresh_mouse_cursor();
        }

        debug_mouse_dispatch(*mouse, hit, handled);
        return;
    }

    if (const auto* key = std::get_if<KeyPress>(&event)) {
        if (key->character == 17) {
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

        if (root_ && root_->handle_event(event)) {
            request_redraw();
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
                ScrollView* scroll = find_scroll_view_for_widget(root_.get(), focused);

                if (scroll != nullptr && focused != nullptr && scroll->contains_widget(focused)) {
                    std::vector<Widget*> items;
                    scroll->collect_focusable(items);
                    const auto it = std::find(items.begin(), items.end(), focused);
                    if (it != items.end()) {
                        const int index = static_cast<int>(it - items.begin());
                        const int next = index + delta;
                        if (next >= 0 && next < static_cast<int>(items.size())) {
                            focus_widget(items[static_cast<std::size_t>(next)]);
                            return;
                        }

                        scroll->scroll_by(delta);
                        request_redraw();
                        return;
                    }
                }

                if (ScrollView* any_scroll = find_first_scroll_view(root_.get())) {
                    any_scroll->scroll_by(delta);
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
    if (!terminal_ready_ || !dirty_) {
        return;
    }

    startup_profile_mark("render.begin_frame");
    backend_->begin_frame();

    if (root_) {
        Canvas canvas(*backend_);
        canvas.set_glyphs(theme_.glyphs);
        root_->paint(canvas);
    }

    startup_profile_mark("render.before_refresh");
    backend_->end_frame();
    startup_profile_mark("render.after_refresh");
    dirty_ = false;
}

} // namespace tuinator
