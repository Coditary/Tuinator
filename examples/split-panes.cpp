#include <tuinator/tuinator.hpp>

#include <memory>
#include <string>

namespace {

constexpr tuinator::BorderEdges kBorderless{false, false, false, false};

class SplitPaneRoot : public tuinator::VBox {
  public:
    explicit SplitPaneRoot(tuinator::BoxOptions options) : tuinator::VBox(options) {}

    bool wants_full_screen() const override { return true; }
};

std::unique_ptr<tuinator::Panel> make_region_panel(const std::string& title, const tuinator::Theme& theme,
                                                   const tuinator::Style& body_style, const std::string& body) {
    auto panel = std::make_unique<tuinator::Panel>(title, theme.border, theme.heading);
    panel->set_border_edges(kBorderless);
    auto content = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0, .padding = 1});
    content->add_child(std::make_unique<tuinator::Label>(body, body_style));
    panel->set_content(std::move(content));
    return panel;
}

std::unique_ptr<tuinator::Widget> make_file_list(const tuinator::Theme& theme) {
    auto list = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0, .padding = 1});
    list->add_child(std::make_unique<tuinator::Label>("src/", theme.accent));
    list->add_child(std::make_unique<tuinator::Label>("  main.cpp", theme.label));
    list->add_child(std::make_unique<tuinator::Label>("  app.hpp", theme.muted));
    list->add_child(std::make_unique<tuinator::Label>("include/", theme.accent));
    list->add_child(std::make_unique<tuinator::Label>("  tuinator.hpp", theme.muted));
    list->add_child(std::make_unique<tuinator::Label>("tests/", theme.accent));
    list->add_child(std::make_unique<tuinator::Label>("  split_pane.cpp", theme.label));
    return list;
}

} // namespace

int main() {
    tuinator::Application app;
    app.set_theme(tuinator::dark_theme({.glyphs = tuinator::GlyphSet::Unicode}));
    const tuinator::Theme& theme = app.theme();

    auto root = std::make_unique<SplitPaneRoot>(tuinator::BoxOptions{.gap = 0, .padding = 0});

    auto menu = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2, .padding = 1});
    menu->add_child(std::make_unique<tuinator::Label>("File", theme.accent));
    menu->add_child(std::make_unique<tuinator::Label>("Edit", theme.accent));
    menu->add_child(std::make_unique<tuinator::Label>("View", theme.accent));
    menu->add_child(std::make_unique<tuinator::Label>("Drag dividers | nested H+V splits | q quit", theme.muted));
    root->add_child(std::move(menu));

    auto explorer = std::make_unique<tuinator::Panel>("Explorer", theme.border, theme.heading);
    explorer->set_border_edges(kBorderless);
    explorer->set_content(make_file_list(theme));

    auto editor = make_region_panel(
        "Editor", theme, theme.success,
        "int main() {\n    // Drag the vertical divider between editor and preview.\n    return 0;\n}");

    auto preview = make_region_panel("Preview", theme, theme.danger,
                                     "Rendered output\n\nResize horizontally against the editor pane.");

    auto editor_row = std::make_unique<tuinator::SplitPane>(std::move(editor), std::move(preview),
                                                            tuinator::SplitPaneOptions{
                                                                .orientation = tuinator::SplitOrientation::Horizontal,
                                                                .first_size = 36,
                                                            });

    auto terminal = make_region_panel(
        "Terminal", theme, theme.accent,
        "$ make split-panes\n$ ./build/tuinator-split-panes\n\nDrag the horizontal divider above this panel.");

    auto editor_column = std::make_unique<tuinator::SplitPane>(std::move(editor_row), std::move(terminal),
                                                               tuinator::SplitPaneOptions{
                                                                   .orientation = tuinator::SplitOrientation::Vertical,
                                                                   .first_size = 18,
                                                               });
    editor_column->set_flex(1);

    auto workspace = std::make_unique<tuinator::SplitPane>(std::move(explorer), std::move(editor_column),
                                                           tuinator::SplitPaneOptions{
                                                               .orientation = tuinator::SplitOrientation::Horizontal,
                                                               .first_size = 24,
                                                               .outer_border = true,
                                                           });
    workspace->set_flex(1);
    root->add_child(std::move(workspace));

    root->add_child(std::make_unique<tuinator::StatusBar>(
        "IDE split test — move all three dividers (sidebar | editor/preview | terminal)", theme.text_input_focused));

    app.set_root(std::move(root));
    return app.run();
}
