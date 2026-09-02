#include <tuinator/tuinator.hpp>

#include <memory>

int main() {
    tuinator::Application app;

    auto shell = std::make_unique<tuinator::ShellTerminal>();
    shell->set_on_exit([&app] { app.quit(); });
    app.set_root(std::move(shell));
    return app.run();
}
