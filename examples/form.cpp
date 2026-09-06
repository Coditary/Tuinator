#include <tuinator/tuinator.hpp>

#include <memory>
#include <string>

int main() {
    tuinator::Application app;
    const tuinator::Theme& theme = app.theme();

    auto form = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});

    form->add_child(std::make_unique<tuinator::Label>("Login", theme.heading));

    form->add_child(std::make_unique<tuinator::Label>("Username", theme.label));

    auto username =
        std::make_unique<tuinator::TextInput>(tuinator::TextInputOptions{.min_width = 24, .placeholder = "guest"},
                                              theme.text_input, theme.text_input_focused);
    auto* username_ptr = username.get();
    form->add_child(std::move(username));

    form->add_child(std::make_unique<tuinator::Label>("Password", theme.label));

    auto password =
        std::make_unique<tuinator::TextInput>(tuinator::TextInputOptions{.min_width = 24, .placeholder = "secret"},
                                              theme.text_input, theme.text_input_focused);
    form->add_child(std::move(password));

    form->add_child(std::make_unique<tuinator::Separator>(theme.border));

    auto preview = std::make_unique<tuinator::Label>("Signed in as: guest", theme.muted);
    auto* preview_ptr = preview.get();
    form->add_child(std::move(preview));

    username_ptr->set_on_change([preview_ptr](const std::string& value) {
        preview_ptr->set_text("Signed in as: " + (value.empty() ? std::string("guest") : value));
    });

    auto actions = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});
    actions->add_child(std::make_unique<tuinator::Button>("OK", [&app]() { app.quit(); }, theme.success));
    actions->add_child(std::make_unique<tuinator::Button>("Cancel", [&app]() { app.quit(); }, theme.danger));
    form->add_child(std::move(actions));

    form->add_child(std::make_unique<tuinator::Label>("Tab between fields | type to edit | q to quit", theme.muted));

    app.set_root(std::move(form));
    return app.run();
}
