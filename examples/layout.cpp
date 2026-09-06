#include <tuinator/tuinator.hpp>

#include <memory>
#include <string>

int main() {
    tuinator::Application app;

    auto root = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});

    root->add_child(std::make_unique<tuinator::Label>(
        "Layout Demo (VBox + HBox)", tuinator::Style{.foreground = tuinator::Color::Cyan, .bold = true}));

    auto row1 = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});
    row1->add_child(std::make_unique<tuinator::Label>("[ A ]"));
    row1->add_child(std::make_unique<tuinator::Label>("[ B ]"));
    row1->add_child(std::make_unique<tuinator::Label>("[ C ]"));
    root->add_child(std::move(row1));

    auto row2 = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 4});
    row2->add_child(std::make_unique<tuinator::Label>("left", tuinator::Style{.foreground = tuinator::Color::Green}));
    row2->add_child(
        std::make_unique<tuinator::Label>("right", tuinator::Style{.foreground = tuinator::Color::Magenta}));
    root->add_child(std::move(row2));

    auto nested = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    nested->add_child(std::make_unique<tuinator::Label>("Nested VBox:"));
    nested->add_child(std::make_unique<tuinator::Label>("  line 1"));
    nested->add_child(std::make_unique<tuinator::Label>("  line 2"));
    nested->add_child(std::make_unique<tuinator::Label>("  line 3"));
    root->add_child(std::move(nested));

    root->add_child(
        std::make_unique<tuinator::Label>("Resize the terminal to see layout reflow",
                                          tuinator::Style{.foreground = tuinator::Color::Yellow, .dim = true}));
    root->add_child(std::make_unique<tuinator::Label>(
        "Press q to quit", tuinator::Style{.foreground = tuinator::Color::Yellow, .dim = true}));

    app.set_root(std::move(root));
    return app.run();
}
