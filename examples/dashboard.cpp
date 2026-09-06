#include <tuinator/tuinator.hpp>

#include <memory>
#include <string>

namespace {

class DashboardRoot : public tuinator::VBox {
  public:
    explicit DashboardRoot(tuinator::BoxOptions options) : tuinator::VBox(options) {}

    bool wants_full_screen() const override { return true; }
};

std::unique_ptr<tuinator::Panel> make_demo_panel(const std::string& title, const tuinator::Theme& theme,
                                                 const std::string& body) {
    auto panel = std::make_unique<tuinator::Panel>(title, theme.border, theme.heading);
    panel->set_content(std::make_unique<tuinator::Label>(body, theme.label));
    return panel;
}

std::unique_ptr<tuinator::Widget> make_home_tab(const tuinator::Theme& theme, tuinator::StatusBar* status,
                                                tuinator::Tabs* tabs, tuinator::Application* app,
                                                int* extra_tab_counter) {
    auto content = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    content->add_child(std::make_unique<tuinator::Label>("Welcome to Tuinator", theme.heading));
    content->add_child(std::make_unique<tuinator::Label>(
        "1-4 = switch view tabs | Tab = focus buttons/toggles | click tab titles", theme.muted));
    content->add_child(std::make_unique<tuinator::Button>(
        "Say hello", [status]() { status->set_text("Hello from the Home tab"); }, theme.button));
    content->add_child(std::make_unique<tuinator::Button>(
        "+ Add tab (code: tabs->add_tab)",
        [tabs, app, status, extra_tab_counter, &theme]() {
            ++(*extra_tab_counter);
            const std::string title = "Extra " + std::to_string(*extra_tab_counter);
            tabs->add_tab(title, std::make_unique<tuinator::Label>(
                                     "New tab created with tabs->add_tab(\"" + title + "\", ...)", theme.label));
            app->refresh_focus();
            status->set_text("Added tab: " + title);
        },
        theme.button));
    return content;
}

std::unique_ptr<tuinator::Widget> make_grid_tab(const tuinator::Theme& theme) {
    auto grid = std::make_unique<tuinator::Grid>(tuinator::GridOptions{.columns = 2, .gap = 1});
    grid->add_child(std::make_unique<tuinator::Label>("Name", theme.label));
    grid->add_child(std::make_unique<tuinator::Label>("Value", theme.label));
    grid->add_child(std::make_unique<tuinator::Label>("CPU", theme.muted));
    grid->add_child(std::make_unique<tuinator::Label>("12%", theme.accent));
    grid->add_child(std::make_unique<tuinator::Label>("Memory", theme.muted));
    grid->add_child(std::make_unique<tuinator::Label>("4.2 GB", theme.accent));
    grid->add_child(std::make_unique<tuinator::Label>("Uptime", theme.muted));
    grid->add_child(std::make_unique<tuinator::Label>("3h 14m", theme.accent));
    return grid;
}

std::unique_ptr<tuinator::Widget> make_settings_tab(const tuinator::Theme& theme, tuinator::StatusBar* status) {
    auto content = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    content->add_child(std::make_unique<tuinator::Label>("Settings", theme.heading));
    content->add_child(std::make_unique<tuinator::Toggle>(
        "Dark mode", true,
        [status](bool enabled) { status->set_text(enabled ? "Dark mode enabled" : "Dark mode disabled"); },
        theme.button, theme.success));
    content->add_child(std::make_unique<tuinator::Toggle>(
        "Notifications", false,
        [status](bool enabled) { status->set_text(enabled ? "Notifications on" : "Notifications off"); }, theme.button,
        theme.success));
    return content;
}

std::unique_ptr<tuinator::Widget> make_nested_tab(const tuinator::Theme& theme, tuinator::StatusBar* status,
                                                  int* panel_counter) {
    auto root = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});

    root->add_child(std::make_unique<tuinator::Label>(
        "Panels in code: auto p = std::make_unique<Panel>(\"Title\"); p->set_content(...);", theme.muted));

    auto panels = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 0});
    auto* panels_ptr = panels.get();
    panels_ptr->add_child(make_demo_panel("Panel 1", theme, "Each panel is clipped inside the outer box."));

    auto outer = std::make_unique<tuinator::Panel>("Outer container", theme.border, theme.heading);
    outer->set_content(std::move(panels));

    root->add_child(std::move(outer));

    root->add_child(std::make_unique<tuinator::Button>(
        "+ Add panel inside outer box",
        [panels_ptr, status, panel_counter, &theme]() {
            ++(*panel_counter);
            const std::string title = "Panel " + std::to_string(*panel_counter);
            panels_ptr->add_child(
                make_demo_panel(title, theme, "Created with panels->add_child(make_unique<Panel>(...))"));
            status->set_text("Added " + title + " — see examples/dashboard.cpp");
        },
        theme.button));

    return root;
}

} // namespace

int main() {
    tuinator::Application app;
    const tuinator::Theme& theme = app.theme();

    int extra_tab_counter = 0;
    int panel_counter = 1;

    auto root = std::make_unique<DashboardRoot>(tuinator::BoxOptions{.gap = 0, .padding = 0});

    auto menu = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2, .padding = 1});
    menu->add_child(std::make_unique<tuinator::Label>("File", theme.accent));
    menu->add_child(std::make_unique<tuinator::Label>("Edit", theme.accent));
    menu->add_child(std::make_unique<tuinator::Label>("View", theme.accent));
    menu->add_child(
        std::make_unique<tuinator::Label>("Keys: 1-4 views | Tab focus | arrows on tab bar | q quit", theme.muted));
    root->add_child(std::move(menu));

    auto status =
        std::make_unique<tuinator::StatusBar>("Ready — press 4 for nested panels demo", theme.text_input_focused);
    auto* status_ptr = status.get();

    auto sidebar = std::make_unique<tuinator::Panel>("Navigation", theme.border, theme.heading);
    {
        auto nav = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0, .padding = 1});
        nav->add_child(std::make_unique<tuinator::Label>("Dashboard", theme.label));
        nav->add_child(std::make_unique<tuinator::Label>("Reports", theme.muted));
        nav->add_child(std::make_unique<tuinator::Label>("Users", theme.muted));
        nav->add_child(std::make_unique<tuinator::Label>("Logs", theme.muted));
        sidebar->set_content(std::move(nav));
    }

    auto tabs = std::make_unique<tuinator::Tabs>(tuinator::TabsOptions{
        .tab_style = theme.label,
        .selected_tab_style = theme.button_focused,
    });
    auto* tabs_ptr = tabs.get();

    tabs->add_tab("Home", make_home_tab(theme, status_ptr, tabs_ptr, &app, &extra_tab_counter));
    tabs->add_tab("Grid", make_grid_tab(theme));
    tabs->add_tab("Settings", make_settings_tab(theme, status_ptr));
    tabs->add_tab("Nested", make_nested_tab(theme, status_ptr, &panel_counter));

    auto split = std::make_unique<tuinator::SplitPane>(std::move(sidebar), std::move(tabs),
                                                       tuinator::SplitPaneOptions{
                                                           .orientation = tuinator::SplitOrientation::Horizontal,
                                                           .first_size = 22,
                                                           .divider_style = theme.border,
                                                       });
    split->set_flex(1);
    root->add_child(std::move(split));

    root->add_child(std::move(status));

    app.set_root(std::move(root));
    return app.run();
}
