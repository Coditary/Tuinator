#include <tuinator/tuinator.hpp>

#include <memory>

int main() {
    tuinator::Application app;

    auto label = std::make_unique<tuinator::Label>(
        "Hello Tuinator",
        tuinator::Style{
            .foreground = tuinator::Color::Cyan,
            .bold = true,
        });

    app.set_root(std::move(label));
    return app.run();
}
