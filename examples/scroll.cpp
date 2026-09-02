#include <tuinator/tuinator.hpp>

#include <memory>
#include <string>

namespace {

std::unique_ptr<tuinator::Widget> make_list_row(
    int index,
    const tuinator::Theme& theme,
    tuinator::Label* status) {
    auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});

    row->add_child(std::make_unique<tuinator::Label>(
        "Item " + std::to_string(index),
        theme.label));

    row->add_child(std::make_unique<tuinator::Toggle>(
        "Enable",
        index % 3 == 0,
        [status](bool checked) {
            const std::string state = checked ? "on" : "off";
            status->set_text("Last toggle: " + state);
        },
        theme.button,
        theme.success));

    return row;
}

} // namespace

int main() {
    tuinator::Application app;
    const tuinator::Theme& theme = app.theme();

    auto root = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});

    root->add_child(std::make_unique<tuinator::Label>("ScrollView + interactive list", theme.heading));
    root->add_child(std::make_unique<tuinator::Label>(
        "Tab / Up/Down = move between toggles | Wheel/PgUp/Dn = scroll | Space toggles",
        theme.muted));

    auto status = std::make_unique<tuinator::Label>("Last toggle: (none)", theme.accent);
    auto* status_ptr = status.get();

    auto list = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    auto* list_ptr = list.get();
    for (int i = 1; i <= 20; ++i) {
        list->add_child(make_list_row(i, theme, status_ptr));
    }

    auto scroll = std::make_unique<tuinator::ScrollView>(
        std::move(list),
        tuinator::ScrollViewOptions{.width = 48, .height = 12});
    auto* scroll_ptr = scroll.get();
    root->add_child(std::move(scroll));

    int next_index = 21;
    root->add_child(std::make_unique<tuinator::Button>(
        "Add row",
        [&app, list_ptr, scroll_ptr, status_ptr, &theme, &next_index]() {
            list_ptr->add_child(make_list_row(next_index, theme, status_ptr));
            ++next_index;
            scroll_ptr->refresh_content();
            app.refresh_focus();
            status_ptr->set_text("Added item " + std::to_string(next_index - 1));
        },
        theme.button));

    root->add_child(std::move(status));

    app.set_root(std::move(root));
    return app.run();
}
