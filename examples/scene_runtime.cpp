#include <tuinator/tuinator.hpp>
#include <tuinator/scene/scene.hpp>

#include "scene_handlers.hpp"

#include <iostream>

int main(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : "examples/scenes/form.scene.json";

    tuinator::Application app;
    tuinator::scene::HandlerRegistry handlers;
    handlers.register_handler("quit", [](tuinator::Application& application) {
        scene_handlers::quit(application);
    });

    tuinator::scene::LoadOptions options;
    options.handlers = &handlers;

    try {
        auto scene = tuinator::scene::load_and_build(path, app, app.theme(), options);
        app.set_root(std::move(scene.root));
    } catch (const std::exception& ex) {
        std::cerr << "Failed to load scene: " << ex.what() << '\n';
        return 1;
    }

    return app.run();
}
