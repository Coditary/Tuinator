#include <tuinator/testing/app_driver.hpp>

#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>

namespace tuinator::testing {

AppDriver::AppDriver(Size terminal_size, std::unique_ptr<Widget> root)
    : app_(MemoryTerminalBackend::create(terminal_size)) {
    memory_ = dynamic_cast<MemoryTerminalBackend*>(&app_.backend());
    app_.set_pointer_hover_tracking(true);
    app_.set_root(std::move(root));
    present();
}

void AppDriver::present() { app_.present(); }

void AppDriver::step_frame() { app_.step_frame(); }

Widget& AppDriver::require(std::string_view selector) { return *require_by_selector(root(), selector); }

void AppDriver::focus(std::string_view selector) {
    Widget& widget = require(selector);
    app_.focus_widget(&widget);
    present();
}

void AppDriver::click(std::string_view selector) {
    present();
    Widget& widget = require(selector);
    const Rect bounds = widget.bounds();
    if (bounds.width <= 0 || bounds.height <= 0) {
        throw std::runtime_error("Cannot click widget with empty bounds");
    }

    app_.focus_widget(&widget);

    MouseEvent press;
    press.position = {bounds.x + bounds.width / 2, bounds.y + bounds.height / 2};
    press.button = MouseButton::Left;
    press.action = MouseAction::Press;
    press.left_pressed = true;
    app_.dispatch_event(press);

    MouseEvent release = press;
    release.action = MouseAction::Release;
    release.left_pressed = false;
    app_.dispatch_event(release);

    MouseEvent click = press;
    click.action = MouseAction::Click;
    click.left_pressed = false;
    app_.dispatch_event(click);
}

void AppDriver::activate(std::string_view selector) {
    Widget& widget = require(selector);
    if (auto* activatable = as_activatable(&widget)) {
        app_.focus_widget(&widget);
        activatable->activate();
        present();
        return;
    }
    click(selector);
}

void AppDriver::press_key(Key key, char character) {
    KeyPress press;
    press.key = key;
    press.character = character;
    if (character != '\0') {
        press.text = std::string(1, character);
    }
    app_.dispatch_event(press);
}

void AppDriver::type_text(std::string_view text) {
    for (const char ch : text) {
        KeyPress press;
        press.key = Key::None;
        press.character = ch;
        press.text = std::string(1, ch);
        app_.dispatch_event(press);
    }
}

void AppDriver::type_into(std::string_view selector, std::string_view text) {
    focus(selector);
    type_text(text);
}

void AppDriver::hover_at(Point position) {
    MouseEvent move;
    move.position = position;
    move.action = MouseAction::Move;
    app_.dispatch_event(move);
}

void AppDriver::resize_terminal(Size size) {
    Resize resize;
    resize.width = size.width;
    resize.height = size.height;
    app_.dispatch_event(resize);
}

void AppDriver::hover(std::string_view selector) {
    present();
    const Widget& widget = require(selector);
    const Rect bounds = widget.bounds();
    hover_at({bounds.x + bounds.width / 2, bounds.y + bounds.height / 2});
}

RegionStyleAnalysis AppDriver::analyze_selector(std::string_view selector) const {
    const Widget* widget = find_by_selector(root(), selector);
    if (widget == nullptr) {
        throw std::runtime_error(std::string("No widget with test selector: ") + std::string(selector));
    }
    return analyze_region(backend(), widget->bounds());
}

std::string AppDriver::selector_styles_json(std::string_view selector) const {
    return region_style_json(analyze_selector(selector));
}

std::string AppDriver::snapshot_text() const { return backend().snapshot(); }

std::string AppDriver::ui_tree_json(UiTreeOptions options) const {
    if (options.include_screen_text && !options.screen_text.has_value()) {
        options.screen_text = snapshot_text();
    }
    const MemoryTerminalBackend* render = options.include_render_styles ? &backend() : nullptr;
    return build_ui_tree_json(root(), options, render);
}

std::string AppDriver::snapshot_ansi() const { return terminal_cells_to_ansi(backend()); }

bool AppDriver::write_snapshot_png(const std::filesystem::path& path, TerminalPngOptions options) const {
    return write_terminal_png(backend(), path, options);
}

} // namespace tuinator::testing
