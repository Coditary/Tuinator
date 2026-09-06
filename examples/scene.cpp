#include <tuinator/tuinator.hpp>

#include "scenes/form_scene.generated.hpp"

int main() {
    tuinator::Application app;
    auto scene = tuinator::scene::form::build(app, app.theme());
    app.set_root(std::move(scene.root));
    return app.run();
}
