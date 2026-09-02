#include <tuinator/tuinator.hpp>

#include <memory>
#include <string>

int main() {
    tuinator::Application app;

    auto status = std::make_unique<tuinator::Label>(
        "Last clicked: (none)",
        tuinator::Style{.foreground = tuinator::Color::White});
    auto* status_ptr = status.get();

    auto root = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});

    root->add_child(std::make_unique<tuinator::Label>(
        "Focus & Buttons",
        tuinator::Style{.foreground = tuinator::Color::Cyan, .bold = true}));

    root->add_child(std::make_unique<tuinator::Label>(
        "Use Tab / Shift+Tab to move between buttons"));

    auto row1 = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});
    row1->add_child(std::make_unique<tuinator::Button>(
        "Alpha",
        [status_ptr]() { status_ptr->set_text("Last clicked: Alpha"); },
        tuinator::Style{.foreground = tuinator::Color::Green}));
    row1->add_child(std::make_unique<tuinator::Button>(
        "Beta",
        [status_ptr]() { status_ptr->set_text("Last clicked: Beta"); },
        tuinator::Style{.foreground = tuinator::Color::Blue}));
    row1->add_child(std::make_unique<tuinator::Button>(
        "Gamma",
        [status_ptr]() { status_ptr->set_text("Last clicked: Gamma"); },
        tuinator::Style{.foreground = tuinator::Color::Magenta}));
    root->add_child(std::move(row1));

    auto row2 = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});
    row2->add_child(std::make_unique<tuinator::Button>(
        "One",
        [status_ptr]() { status_ptr->set_text("Last clicked: One"); }));
    row2->add_child(std::make_unique<tuinator::Button>(
        "Two",
        [status_ptr]() { status_ptr->set_text("Last clicked: Two"); }));
    row2->add_child(std::make_unique<tuinator::Button>(
        "Quit",
        [&app]() { app.quit(); },
        tuinator::Style{.foreground = tuinator::Color::Red}));
    root->add_child(std::move(row2));

    root->add_child(std::move(status));

    root->add_child(std::make_unique<tuinator::Label>(
        "Click or Tab to focus | Enter/Space activates",
        tuinator::Style{.foreground = tuinator::Color::Yellow, .dim = true}));

    app.set_root(std::move(root));
    return app.run();
}
