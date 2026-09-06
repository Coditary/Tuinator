// Generated from hello.scene.json. Do not edit by hand.

#include "hello_scene.generated.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <functional>

namespace tuinator {
namespace scene {
namespace hello {

BuildResult build(tuinator::Application& app, const tuinator::Theme& theme) {
    auto VBox_1 = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 2});
    auto Label_2 = std::make_unique<tuinator::Label>("Hello Tuinator", tuinator::Style{.foreground = tuinator::Color::Cyan, .bold = true});
    VBox_1->add_child(std::move(Label_2));
    auto Label_3 = std::make_unique<tuinator::Label>("Built from a JSON scene file", theme.muted);
    VBox_1->add_child(std::move(Label_3));
    auto Button_4 = std::make_unique<tuinator::Button>("Quit", [&app]() { scene_handlers::quit(app); }, theme.success);
    VBox_1->add_child(std::move(Button_4));

    BuildResult result;
    result.root = std::move(VBox_1);
    return result;
}

} // namespace
} // namespace
} // namespace
