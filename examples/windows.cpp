#include <tuinator/tuinator.hpp>

#include <memory>
#include <string>

namespace {

std::unique_ptr<tuinator::Widget> make_panel(std::string title, std::string body) {
    auto panel = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    panel->add_child(std::make_unique<tuinator::Label>(
        std::move(title),
        tuinator::Style{.foreground = tuinator::Color::Cyan, .bold = true}));
    panel->add_child(std::make_unique<tuinator::Label>(std::move(body)));
    return panel;
}

std::unique_ptr<tuinator::WindowHost> make_nested_host() {
    auto host = std::make_unique<tuinator::WindowHost>();

    host->create_window(
        "Inner A",
        {2, 2, 19, 7},
        make_panel("Inner A", "Stays inside."));

    host->create_window(
        "Inner B",
        {23, 2, 19, 7},
        make_panel("Inner B", "Clamped resize."));

    return host;
}

} // namespace

int main() {
    tuinator::Application app;
    auto desktop = std::make_unique<tuinator::Desktop>();

    auto background = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    background->add_child(std::make_unique<tuinator::Label>(
        "Tuinator Desktop",
        tuinator::Style{.foreground = tuinator::Color::White, .bold = true}));
    background->add_child(std::make_unique<tuinator::Label>("Drag outer + inner title bars | Resize with #"));
    background->add_child(std::make_unique<tuinator::Label>(
        "Esc closes inner first, then outer | q quits",
        tuinator::Style{.foreground = tuinator::Color::Yellow, .dim = true}));
    desktop->set_background(std::move(background));

    desktop->create_window(
        "Container",
        {6, 3, 44, 16},
        make_nested_host());

    desktop->create_window(
        "Editor",
        {34, 4, 30, 10},
        make_panel("Editor", "Normal top-level\nfloating window."));

    desktop->create_window(
        "Tools",
        {52, 4, 26, 8},
        make_panel("Tools", "Click to focus.\nI stay on top."));

    app.set_root(std::move(desktop));
    return app.run();
}
