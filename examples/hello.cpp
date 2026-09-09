#include <tuinator/tuinator.hpp>

#include <memory>

int main() {
    tuinator::Application app;
    const tuinator::Theme theme = tuinator::dark_theme();

    auto root = tuinator::make_screen({.gap = 1, .padding = 2});
    root->add_child(std::make_unique<tuinator::Label>("Hello Tuinator", theme.heading));

    return tuinator::run_screen(app, std::move(root));
}
