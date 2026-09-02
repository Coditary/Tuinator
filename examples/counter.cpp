#include <tuinator/tuinator.hpp>

#include <memory>
#include <string>

int main() {
    tuinator::Application app;

    int count = 0;

    auto root = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 2});

    root->add_child(std::make_unique<tuinator::Label>(
        "Counter",
        tuinator::Style{.foreground = tuinator::Color::Cyan, .bold = true}));

    auto count_label = std::make_unique<tuinator::Label>(
        "Value: 0",
        tuinator::Style{.foreground = tuinator::Color::White, .bold = true});
    auto* count_label_ptr = count_label.get();
    root->add_child(std::move(count_label));

    auto update = [count_label_ptr, &count]() {
        count_label_ptr->set_text("Value: " + std::to_string(count));
    };

    auto controls = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});
    controls->add_child(std::make_unique<tuinator::Button>(
        "-",
        [&]() {
            --count;
            update();
        },
        tuinator::Style{.foreground = tuinator::Color::Red}));

    controls->add_child(std::make_unique<tuinator::Button>(
        "Reset",
        [&]() {
            count = 0;
            update();
        },
        tuinator::Style{.foreground = tuinator::Color::Yellow}));

    controls->add_child(std::make_unique<tuinator::Button>(
        "+",
        [&]() {
            ++count;
            update();
        },
        tuinator::Style{.foreground = tuinator::Color::Green}));

    root->add_child(std::move(controls));

    root->add_child(std::make_unique<tuinator::Label>(
        "Click buttons or use Tab | q to quit",
        tuinator::Style{.foreground = tuinator::Color::Yellow, .dim = true}));

    app.set_root(std::move(root));
    return app.run();
}
