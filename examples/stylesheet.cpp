#include <tuinator/tuinator.hpp>

#include <iostream>
#include <memory>

namespace {

constexpr tuinator::BorderEdges kBorderless{false, false, false, false};

std::unique_ptr<tuinator::Panel> make_panel(const std::string& title, std::unique_ptr<tuinator::Widget> content) {
    auto panel = std::make_unique<tuinator::Panel>(title);
    panel->set_border_edges(kBorderless);
    panel->set_content(std::move(content));
    return panel;
}

} // namespace

int main(int argc, char** argv) {
    tuinator::Application app;

    const char* theme_path = TUINATOR_SOURCE_DIR "/examples/app.theme";
    if (argc > 1) {
        theme_path = argv[1];
    }

    try {
        app.load_stylesheet(theme_path);
    } catch (const std::exception& error) {
        std::cerr << "Failed to load stylesheet: " << error.what() << '\n';
        return 1;
    }

    const tuinator::Theme& theme = app.theme();

    auto root = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0, .padding = 1});
    root->set_widget_id("root");

    auto header = std::make_unique<tuinator::Label>("Stylesheet demo (examples/app.theme)", theme.heading);
    header->set_widget_id("title");
    root->add_child(std::move(header));

    auto hint = std::make_unique<tuinator::Label>("Global Label color + .accent / .muted classes | q quit");
    hint->add_widget_class("muted");
    root->add_child(std::move(hint));

    auto explorer = make_panel("Explorer", std::make_unique<tuinator::Label>("src/main.cpp"));
    explorer->add_widget_class("sidebar");

    auto editor = make_panel("Editor", std::make_unique<tuinator::Label>("Edit with stylesheet-driven colors"));
    auto preview = make_panel("Preview", std::make_unique<tuinator::Label>("Preview pane"));
    auto editor_row = std::make_unique<tuinator::SplitPane>(
        std::move(editor), std::move(preview),
        tuinator::SplitPaneOptions{.orientation = tuinator::SplitOrientation::Horizontal, .first_size = 28});

    auto terminal = make_panel("Terminal", std::make_unique<tuinator::Label>("$ ./build/tuinator-stylesheet"));
    auto editor_column = std::make_unique<tuinator::SplitPane>(
        std::move(editor_row), std::move(terminal),
        tuinator::SplitPaneOptions{.orientation = tuinator::SplitOrientation::Vertical, .first_size = 12});

    auto workspace = std::make_unique<tuinator::SplitPane>(
        std::move(explorer), std::move(editor_column),
        tuinator::SplitPaneOptions{
            .orientation = tuinator::SplitOrientation::Horizontal,
            .first_size = 22,
            .outer_border = true,
        });
    workspace->set_widget_id("workspace");
    root->add_child(std::move(workspace));

    app.set_root(std::move(root));
    return app.run();
}
