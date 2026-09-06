// Generated from form.scene.json. Do not edit by hand.

#include "form_scene.generated.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <functional>

namespace tuinator::scene_binding {

inline std::string combo_selected_item(const ::tuinator::ComboBox* widget) {
    if (widget == nullptr) {
        return {};
    }
    const auto& items = widget->items();
    const int index = widget->selected_index();
    if (index < 0 || index >= static_cast<int>(items.size())) {
        return {};
    }
    return items[index];
}

inline std::string list_selected_item(const ::tuinator::ListView* widget) {
    if (widget == nullptr) {
        return {};
    }
    const auto& items = widget->items();
    const int index = widget->selected_index();
    if (index < 0 || index >= static_cast<int>(items.size())) {
        return {};
    }
    return items[index];
}

} // namespace tuinator::scene_binding

namespace tuinator {
namespace scene {
namespace form {

BuildResult build(tuinator::Application& app, const tuinator::Theme& theme) {
    auto VBox_1 = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    auto Label_2 = std::make_unique<tuinator::Label>("Login", theme.heading);
    VBox_1->add_child(std::move(Label_2));
    auto Label_3 = std::make_unique<tuinator::Label>("Username", theme.label);
    VBox_1->add_child(std::move(Label_3));
    tuinator::TextInputOptions TextInput_4_options;
    TextInput_4_options.min_width = 24;
    TextInput_4_options.placeholder = "guest";
    auto TextInput_4 = std::make_unique<tuinator::TextInput>(TextInput_4_options, theme.text_input, theme.text_input_focused);
    tuinator::TextInput* scene_username_ptr = TextInput_4.get();
    VBox_1->add_child(std::move(TextInput_4));
    auto Label_5 = std::make_unique<tuinator::Label>("Password", theme.label);
    VBox_1->add_child(std::move(Label_5));
    tuinator::TextInputOptions TextInput_6_options;
    TextInput_6_options.min_width = 24;
    TextInput_6_options.placeholder = "secret";
    auto TextInput_6 = std::make_unique<tuinator::TextInput>(TextInput_6_options, theme.text_input, theme.text_input_focused);
    VBox_1->add_child(std::move(TextInput_6));
    auto Separator_7 = std::make_unique<tuinator::Separator>(theme.border);
    VBox_1->add_child(std::move(Separator_7));
    auto Label_8 = std::make_unique<tuinator::Label>("Signed in as: guest", theme.muted);
    tuinator::Label* scene_preview_ptr = Label_8.get();
    VBox_1->add_child(std::move(Label_8));
    auto HBox_9 = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2, .padding = 0});
    auto Button_10 = std::make_unique<tuinator::Button>("OK", [&app]() { scene_handlers::quit(app); }, theme.success);
    HBox_9->add_child(std::move(Button_10));
    auto Button_11 = std::make_unique<tuinator::Button>("Cancel", [&app]() { scene_handlers::quit(app); }, theme.danger);
    HBox_9->add_child(std::move(Button_11));
    VBox_1->add_child(std::move(HBox_9));
    auto Label_12 = std::make_unique<tuinator::Label>("Tab between fields | type to edit | q to quit", theme.muted);
    VBox_1->add_child(std::move(Label_12));

    scene_preview_ptr->set_text("Signed in as: " + (scene_username_ptr->value().empty() ? std::string("guest") : scene_username_ptr->value()) + "");
    scene_username_ptr->set_on_change([scene_preview_ptr](const std::string& value) {
        scene_preview_ptr->set_text("Signed in as: " + (value.empty() ? std::string("guest") : value) + "");
    });
    BuildResult result;
    result.root = std::move(VBox_1);
    return result;
}

} // namespace
} // namespace
} // namespace
