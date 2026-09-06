#include <tuinator/tuinator.hpp>

#include <memory>
#include <string>

namespace {

std::unique_ptr<tuinator::Widget> make_list_row(int index, const tuinator::Theme& theme, tuinator::Label* status) {
    auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});

    row->add_child(std::make_unique<tuinator::Label>("Item " + std::to_string(index), theme.label));

    row->add_child(std::make_unique<tuinator::Toggle>(
        "Enable", index % 3 == 0,
        [status](bool checked) {
            const std::string state = checked ? "on" : "off";
            status->set_text("Last toggle: " + state);
        },
        theme.button, theme.success));

    return row;
}

std::unique_ptr<tuinator::Widget> make_wide_row(int index, const tuinator::Theme& theme) {
    auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 1});
    row->add_child(
        std::make_unique<tuinator::Label>("Row " + std::to_string(index) + " | alpha beta gamma delta", theme.label));
    row->add_child(
        std::make_unique<tuinator::Label>("extra-wide tail section " + std::to_string(index * 11), theme.muted));
    return row;
}

} // namespace

int main() {
    tuinator::Application app;
    const tuinator::Theme& theme = app.theme();

    auto root = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});

    root->add_child(std::make_unique<tuinator::Label>("ScrollView demo (vertical + horizontal)", theme.heading));
    root->add_child(std::make_unique<tuinator::Label>(
        "Mouse: click toggles | drag scrollbars | wheel/tile scroll | Tab moves focus", theme.muted));

    auto status = std::make_unique<tuinator::Label>("Last toggle: (none)", theme.accent);
    auto* status_ptr = status.get();

    auto panels = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});

    auto left_column = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    left_column->add_child(std::make_unique<tuinator::Label>("Classic scrollbar", theme.muted));

    auto list = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    auto* list_ptr = list.get();
    for (int i = 1; i <= 20; ++i) {
        list->add_child(make_list_row(i, theme, status_ptr));
    }

    tuinator::ScrollViewOptions vertical_options{
        .width = 34,
        .height = 12,
        .scrollbars = tuinator::scrollbar_options(theme, tuinator::ScrollbarPreset::Classic),
    };

    auto vertical_scroll = std::make_unique<tuinator::ScrollView>(std::move(list), vertical_options);
    auto* vertical_scroll_ptr = vertical_scroll.get();
    left_column->add_child(std::move(vertical_scroll));
    panels->add_child(std::move(left_column));

    auto right_column = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    right_column->add_child(std::make_unique<tuinator::Label>("Custom thin + orange thumb", theme.muted));

    auto wide = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    for (int i = 1; i <= 14; ++i) {
        wide->add_child(make_wide_row(i, theme));
    }

    tuinator::ScrollViewOptions horizontal_options{
        .width = 34,
        .height = 12,
        .scrollbars = tuinator::scrollbar_options(theme, tuinator::ScrollbarPreset::Thin)
                          .with_thumb_style(tuinator::style_fg(tuinator::Rgb::hex(0xFFAA44))),
    };

    right_column->add_child(std::make_unique<tuinator::ScrollView>(std::move(wide), horizontal_options));
    panels->add_child(std::move(right_column));

    root->add_child(std::move(panels));

    int next_index = 21;
    root->add_child(std::make_unique<tuinator::Button>(
        "Add row",
        [&app, list_ptr, vertical_scroll_ptr, status_ptr, &theme, &next_index]() {
            list_ptr->add_child(make_list_row(next_index, theme, status_ptr));
            ++next_index;
            vertical_scroll_ptr->refresh_content();
            app.refresh_focus();
            status_ptr->set_text("Added item " + std::to_string(next_index - 1));
        },
        theme.button));

    root->add_child(std::move(status));

    app.set_root(std::move(root));
    return app.run();
}
