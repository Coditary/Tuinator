#include <tuinator/tuinator.hpp>

#include <memory>

/// Canonical minimal Tuinator app — see docs/golden-examples.md.
int main() {
    tuinator::Application app;

    auto root = tuinator::make_screen({.gap = 1, .padding = 1});
    const tuinator::Theme theme = tuinator::dark_theme();

    root->add_child(std::make_unique<tuinator::Label>("Golden App", theme.heading));
    root->add_child(std::make_unique<tuinator::Label>("VBox root + paint_bounds_background on every widget",
                                                      theme.muted));
    root->add_child(std::make_unique<tuinator::Label>("q to quit", theme.muted));

    return tuinator::run_screen(app, std::move(root));
}
