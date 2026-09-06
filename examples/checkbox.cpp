#include <tuinator/tuinator.hpp>

#include <cstring>
#include <memory>
#include <string>

namespace {

class CheckboxRoot : public tuinator::VBox {
  public:
    explicit CheckboxRoot(tuinator::BoxOptions options) : tuinator::VBox(options) {}

    bool wants_full_screen() const override { return true; }
};

void add_section_header(tuinator::VBox& box, const char* title, const tuinator::Theme& theme) {
    box.add_child(std::make_unique<tuinator::Separator>(theme.border));
    box.add_child(std::make_unique<tuinator::Label>(title, theme.heading));
}

void add_checkbox(tuinator::VBox& box, const tuinator::Theme& theme, tuinator::StatusBar* status, const char* label,
                  bool checked, const char* style_name) {
    auto checkbox = std::make_unique<tuinator::Checkbox>(
        label, checked, style_name,
        [status, label](bool value) { status->set_text(std::string(label) + (value ? " enabled" : " disabled")); },
        theme);
    box.add_child(std::move(checkbox));
}

} // namespace

int main() {
    tuinator::Application app;
    const tuinator::Theme& theme = app.theme();

    auto status = std::make_unique<tuinator::StatusBar>("Ready", theme.muted);
    tuinator::StatusBar* status_ptr = status.get();

    auto content = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0, .padding = 1});
    content->add_child(std::make_unique<tuinator::Label>("tui-checkbox Demo - Interactive Mode", theme.heading));
    content->add_child(std::make_unique<tuinator::Label>("A customizable checkbox widget for Tuinator", theme.muted));

    add_section_header(*content, "Basic Checkbox", theme);
    add_checkbox(*content, theme, status_ptr, "Enable notifications", true, "basic");

    add_section_header(*content, "Styled Checkboxes", theme);
    add_checkbox(*content, theme, status_ptr, "Info state", false, "info");
    add_checkbox(*content, theme, status_ptr, "Warning state", true, "warning");
    add_checkbox(*content, theme, status_ptr, "Error state", false, "error");

    add_section_header(*content, "Emoji & Unicode Symbols", theme);
    add_checkbox(*content, theme, status_ptr, "Check mark style", true, "checkmark");
    add_checkbox(*content, theme, status_ptr, "Circle style", false, "circle");
    add_checkbox(*content, theme, status_ptr, "Diamond style", true, "diamond");

    add_section_header(*content, "Custom ASCII Symbols", theme);
    add_checkbox(*content, theme, status_ptr, "ASCII style", true, "ascii");
    add_checkbox(*content, theme, status_ptr, "Asterisk", false, "asterisk");
    add_checkbox(*content, theme, status_ptr, "Plus sign", true, "plus");
    add_checkbox(*content, theme, status_ptr, "X/O style", false, "xo");

    add_section_header(*content, "Custom Options", theme);
    tuinator::CheckboxOptions custom = tuinator::checkbox_options_default(theme);
    custom.glyphs = {"<", ">", " "};
    custom.marker_checked_style.foreground = tuinator::Color::Green;
    custom.label_style = theme.accent;
    custom.label_checked_style.foreground = tuinator::Color::Cyan;
    auto custom_box =
        std::make_unique<tuinator::Checkbox>("Fully custom glyphs + colors", true, custom, [status_ptr](bool value) {
            status_ptr->set_text(std::string("Fully custom glyphs + colors") + (value ? " enabled" : " disabled"));
        });
    content->add_child(std::move(custom_box));

    auto scroll = std::make_unique<tuinator::ScrollView>(std::move(content),
                                                         tuinator::ScrollViewOptions{.width = 72, .height = 22});
    scroll->set_flex(1);

    auto root = std::make_unique<CheckboxRoot>(tuinator::BoxOptions{.gap = 0, .padding = 0});
    root->add_child(std::move(scroll));
    root->add_child(
        std::make_unique<tuinator::Label>("Up/Down Navigate  |  Space Toggle  |  Tab Next  |  q Quit", theme.muted));
    root->add_child(std::move(status));

    app.set_root(std::move(root));
    return app.run();
}
