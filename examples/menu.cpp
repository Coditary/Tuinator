#include <tuinator/tuinator.hpp>

#include <memory>
#include <string>
#include <vector>

namespace {

class MenuRoot : public tuinator::VBox {
  public:
    explicit MenuRoot(tuinator::BoxOptions options) : tuinator::VBox(options) {}

    bool wants_full_screen() const override { return true; }

    void set_menu_bar(tuinator::MenuBar* menu) { menu_ = menu; }

    bool handle_event(const tuinator::Event& event) override {
        if (menu_ == nullptr) {
            return tuinator::VBox::handle_event(event);
        }

        const auto* key = std::get_if<tuinator::KeyPress>(&event);
        if (key != nullptr && key->character >= '1' && key->character <= '3') {
            const char* style_name = nullptr;
            switch (key->character) {
            case '1': style_name = "classic"; break;
            case '2': style_name = "mac"; break;
            case '3': style_name = "minimal"; break;
            default: break;
            }
            if (style_name != nullptr) {
                if (const tuinator::MenuBarLook* look = tuinator::menu_bar_look_named(style_name)) {
                    menu_->apply_look(*look);
                    return true;
                }
            }
        }

        return tuinator::VBox::handle_event(event);
    }

  private:
    tuinator::MenuBar* menu_ = nullptr;
};

tuinator::MenuItem action_item(std::string label, std::function<void()> action, std::string shortcut = {},
                               std::string icon = {}, std::string hint = {}, bool enabled = true) {
    tuinator::MenuItem item;
    item.label = std::move(label);
    item.action = std::move(action);
    item.shortcut = std::move(shortcut);
    item.icon = std::move(icon);
    item.hint = std::move(hint);
    item.enabled = enabled;
    return item;
}

void register_palette_action(tuinator::ActionRegistry& registry, std::string id, std::string label,
                             std::string category, std::string shortcut, std::string hint,
                             std::function<void()> callback) {
    tuinator::RegisteredAction action;
    action.id = std::move(id);
    action.label = std::move(label);
    action.category = std::move(category);
    action.shortcut = std::move(shortcut);
    action.hint = std::move(hint);
    action.callback = std::move(callback);
    registry.register_action(std::move(action));
}

std::vector<tuinator::CommandPaletteEntry> build_palette_entries(const tuinator::ActionRegistry& registry,
                                                                 std::function<void()> quit_action) {
    std::vector<tuinator::CommandPaletteEntry> entries;
    for (const tuinator::RegisteredAction* action : registry.all()) {
        entries.push_back({
            action->id,
            tuinator::parse_menu_mnemonic(action->label).text,
            action->category,
            action->shortcut,
            action->callback,
        });
    }
    entries.push_back({"app.quit", "Quit Application", "App", "Ctrl+Q", std::move(quit_action)});
    return entries;
}

} // namespace

int main() {
    tuinator::Application app;
    const tuinator::Theme theme = app.theme();

    auto status_message = std::make_shared<std::string>("Ready — hover menu items for hints");
    auto status = std::make_unique<tuinator::StatusBar>(*status_message, theme.muted);
    tuinator::StatusBar* status_ptr = status.get();

    auto desktop = std::make_unique<tuinator::Desktop>();
    tuinator::Desktop* desktop_ptr = desktop.get();

    auto actions = std::make_shared<tuinator::ActionRegistry>();
    register_palette_action(*actions, "file.new", "&New", "File", "Ctrl+N", "Create a blank document",
                            [status_ptr, status_message]() {
                                *status_message = "Action: New";
                                status_ptr->set_text(*status_message);
                            });
    register_palette_action(*actions, "edit.copy", "&Copy", "Edit", "Ctrl+C", "Copy selection to clipboard",
                            [status_ptr, status_message]() {
                                *status_message = "Action: Copy";
                                status_ptr->set_text(*status_message);
                            });
    register_palette_action(*actions, "view.palette", "Command &Palette", "View", "Ctrl+P", "Search all commands",
                            [desktop_ptr]() { desktop_ptr->show_command_palette(); });
    desktop_ptr->set_action_registry(actions);

    auto menu = std::make_unique<tuinator::MenuBar>(theme.label, theme.button_focused);
    tuinator::MenuBar* menu_ptr = menu.get();
    if (const tuinator::MenuBarLook* look = tuinator::menu_bar_look_named("mac")) {
        menu_ptr->apply_look(*look);
    }

    const auto notify = [status_ptr, status_message](const std::string& text) {
        *status_message = text;
        status_ptr->set_text(*status_message);
    };

    menu_ptr->set_menus({
        {"&File",
         {
             action_item(
                 "&New", [notify]() { notify("File -> New"); }, "Ctrl+N", "*", "Create a new document"),
             action_item(
                 "&Open", [notify]() { notify("File -> Open"); }, "Ctrl+O", ">", "Open a file"),
             tuinator::MenuItem::submenu("&Open recent",
                                         {
                                             action_item(
                                                 "file_1.txt", [notify]() { notify("Opened file_1.txt"); }, {}, "1"),
                                             action_item(
                                                 "file_2.txt", [notify]() { notify("Opened file_2.txt"); }, {}, "2"),
                                             action_item(
                                                 "notes.md", [notify]() { notify("Opened notes.md"); }, {}, "N"),
                                         },
                                         {}, ">"),
             tuinator::MenuItem::submenu(
                 "&Export",
                 {
                     tuinator::MenuItem::submenu("&Format",
                                                 {
                                                     action_item("&PDF", [notify]() { notify("Export PDF"); }),
                                                     action_item("&PNG", [notify]() { notify("Export PNG"); }),
                                                     action_item("&SVG", [notify]() { notify("Export SVG"); }),
                                                 }),
                     action_item("&Current view", [notify]() { notify("Export current view"); }),
                 }),
             tuinator::MenuItem::separator(),
             action_item(
                 "Save &as...", [notify]() { notify("File -> Save as"); }, "Ctrl+S", "S"),
             tuinator::MenuItem::separator(),
             action_item(
                 "E&xit", [&app]() { app.quit(); }, "Ctrl+Q", "X", "Close the application"),
         }},
        {"&Edit",
         {
             action_item(
                 "&Undo", [notify]() { notify("Edit -> Undo"); }, "Ctrl+Z", "U", "Undo last change", false),
             action_item(
                 "&Redo", [notify]() { notify("Edit -> Redo"); }, "Ctrl+Y", "R", "Redo last undone change", false),
             tuinator::MenuItem::separator(),
             action_item(
                 "Cu&t", [notify]() { notify("Edit -> Cut"); }, "Ctrl+X", "T"),
             action_item(
                 "&Copy", [notify]() { notify("Edit -> Copy"); }, "Ctrl+C", "C"),
             action_item(
                 "&Paste", [notify]() { notify("Edit -> Paste"); }, "Ctrl+V", "V"),
             tuinator::MenuItem::separator(),
             tuinator::MenuItem::checkbox("&Line numbers", true, [notify]() { notify("Toggled line numbers"); }),
             tuinator::MenuItem::checkbox("&Word wrap", false, [notify]() { notify("Toggled word wrap"); }),
         }},
        {"&View",
         {
             tuinator::MenuItem::checkbox("&Toolbar", true),
             tuinator::MenuItem::checkbox("&Status bar", true),
             tuinator::MenuItem::separator(),
             action_item(
                 "Zoom &in", [notify]() { notify("View -> Zoom in"); }, "Ctrl++", "+"),
             action_item(
                 "Zoom out", [notify]() { notify("View -> Zoom out"); }, "Ctrl+-", "-", "Decrease zoom", false),
             tuinator::MenuItem::separator(),
             tuinator::MenuItem::submenu(
                 "&Theme",
                 {
                     action_item("&Classic",
                                 [menu_ptr, notify]() {
                                     if (const tuinator::MenuBarLook* look = tuinator::menu_bar_look_named("classic")) {
                                         menu_ptr->apply_look(*look);
                                     }
                                     notify("Theme: classic");
                                 }),
                     action_item("&Mac",
                                 [menu_ptr, notify]() {
                                     if (const tuinator::MenuBarLook* look = tuinator::menu_bar_look_named("mac")) {
                                         menu_ptr->apply_look(*look);
                                     }
                                     notify("Theme: mac");
                                 }),
                     action_item("&Minimal",
                                 [menu_ptr, notify]() {
                                     if (const tuinator::MenuBarLook* look = tuinator::menu_bar_look_named("minimal")) {
                                         menu_ptr->apply_look(*look);
                                     }
                                     notify("Theme: minimal");
                                 }),
                 }),
         }},
        {"&Help",
         {
             action_item("&About Tuinator", [notify]() { notify("Tuinator menu showcase"); }),
             action_item("&Docs", [notify]() { notify("Open docs/API.md"); }),
         }},
    });

    menu_ptr->set_on_action(
        [notify](const std::string& menu_name, const std::string& item) { notify(menu_name + " -> " + item); });
    menu_ptr->set_on_hint([status_ptr, status_message](const std::string& hint) {
        if (!hint.empty()) {
            *status_message = hint;
            status_ptr->set_text(*status_message);
        }
    });

    desktop_ptr->set_context_menu_items({
        action_item(
            "&Copy", [notify]() { notify("Context -> Copy"); }, "Ctrl+C", "C", "Copy selection"),
        action_item(
            "&Paste", [notify]() { notify("Context -> Paste"); }, "Ctrl+V", "V", "Paste from clipboard"),
        tuinator::MenuItem::separator(),
        tuinator::MenuItem::submenu(
            "&More",
            {
                action_item("&Refresh", [notify]() { notify("Context -> Refresh"); }),
                action_item("&Inspect", [notify]() { notify("Context -> Inspect"); }),
                tuinator::MenuItem::submenu(
                    "De&bug",
                    {
                        action_item("Log &level", [notify]() { notify("Context -> Log level"); }),
                        action_item("&Profiler", [notify]() { notify("Context -> Profiler"); }),
                    }),
            }),
        tuinator::MenuItem::separator(),
        action_item(
            "&Quit", [&app]() { app.quit(); }, "Ctrl+Q", "X"),
    });
    desktop_ptr->set_command_palette_entries(build_palette_entries(*actions, [&app]() { app.quit(); }));

    auto header = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2, .padding = 0});
    menu->set_flex(1);
    header->add_child(std::move(menu));
    header->add_child(std::make_unique<tuinator::Label>("tuinator-menu", theme.accent));

    auto body = std::make_unique<tuinator::Panel>("Menu catalog", theme.border, theme.heading, theme.glyphs);
    auto panel_body = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0, .padding = 1});
    panel_body->add_child(std::make_unique<tuinator::Label>("MenuBar item types in this demo", theme.heading));
    panel_body->add_child(
        std::make_unique<tuinator::Label>("  Action        normal clickable entry with callback", theme.muted));
    panel_body->add_child(std::make_unique<tuinator::Label>(
        "  Submenu       nested panel to the right (File -> Open recent)", theme.muted));
    panel_body->add_child(
        std::make_unique<tuinator::Label>("  Separator     horizontal rule between groups", theme.muted));
    panel_body->add_child(
        std::make_unique<tuinator::Label>("  Checkbox      toggle item, stays open ([x] / [ ])", theme.muted));
    panel_body->add_child(
        std::make_unique<tuinator::Label>("  Disabled      dimmed, not activatable (Edit -> Undo)", theme.muted));
    panel_body->add_child(
        std::make_unique<tuinator::Label>("  Shortcut      right-aligned label (Ctrl+S)", theme.muted));
    panel_body->add_child(
        std::make_unique<tuinator::Label>("  Mnemonic      &letter opens item (Alt+F, Alt+O)", theme.muted));
    panel_body->add_child(std::make_unique<tuinator::Label>("  Icon          prefix glyph before label", theme.muted));
    panel_body->add_child(
        std::make_unique<tuinator::Label>("  Hint          shown in status bar on selection", theme.muted));
    panel_body->add_child(std::make_unique<tuinator::Label>("", theme.muted));
    panel_body->add_child(std::make_unique<tuinator::Label>("Other menu surfaces", theme.heading));
    panel_body->add_child(
        std::make_unique<tuinator::Label>("  ContextMenu   right-click the panel below", theme.muted));
    panel_body->add_child(std::make_unique<tuinator::Label>("  CommandPalette  Ctrl+P fuzzy search", theme.muted));
    panel_body->add_child(
        std::make_unique<tuinator::Label>("  ActionRegistry  global shortcuts (Ctrl+N, Ctrl+C)", theme.muted));
    panel_body->add_child(
        std::make_unique<tuinator::Label>("  Style presets   View -> Theme or keys 1/2/3", theme.muted));
    panel_body->add_child(std::make_unique<tuinator::Label>("", theme.muted));
    panel_body->add_child(std::make_unique<tuinator::Label>(
        "Tab = focus menu | Arrows + Enter | Right-click = context | Ctrl+P = palette | q = quit", theme.muted));
    body->set_content(std::move(panel_body));

    auto root = std::make_unique<MenuRoot>(tuinator::BoxOptions{.gap = 0, .padding = 0});
    root->set_menu_bar(menu_ptr);
    root->add_child(std::move(header));
    root->add_child(std::move(body));
    root->add_child(std::move(status));

    desktop->set_background(std::move(root));
    app.set_root(std::move(desktop));

    return app.run();
}
