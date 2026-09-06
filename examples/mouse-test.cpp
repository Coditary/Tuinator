#include <tuinator/tuinator.hpp>

#include <memory>
#include <string>

int main() {
    tuinator::Application app;

    auto root = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 2});

    root->add_child(std::make_unique<tuinator::Label>(
        "Mouse test", tuinator::Style{.foreground = tuinator::Color::Cyan, .bold = true}));

    auto status = std::make_unique<tuinator::Label>("Click the button or press q to quit",
                                                    tuinator::Style{.foreground = tuinator::Color::White});
    auto* status_ptr = status.get();
    root->add_child(std::move(status));

    auto actions = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});
    actions->add_child(std::make_unique<tuinator::Button>(
        "Click me", [status_ptr]() { status_ptr->set_text("Button clicked!"); },
        tuinator::Style{.foreground = tuinator::Color::Green}));
    root->add_child(std::move(actions));

    root->add_child(
        std::make_unique<tuinator::Label>("Debug log: /tmp/tuinator-mouse.log (with TUINATOR_MOUSE_DEBUG=1)",
                                          tuinator::Style{.foreground = tuinator::Color::Yellow, .dim = true}));

    app.set_root(std::move(root));
    return app.run();
}
