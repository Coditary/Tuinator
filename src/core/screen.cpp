#include <tuinator/core/screen.hpp>

namespace tuinator {

Screen::Screen(BoxOptions options) : VBox(options) {}

int run_screen(Application& app, std::unique_ptr<Screen> root) {
    app.set_root(std::move(root));
    return app.run();
}

} // namespace tuinator
