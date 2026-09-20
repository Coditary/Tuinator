#include <tuinator/tuinator.hpp>

#include <memory>
#include <string>

namespace {

const char* kSample = R"md(# Zwei Markdown-Bilder gleichzeitig

Beide Block-Bilder stehen untereinander und sind im Viewport sichtbar:

![Lenna Testbild](examples/assets/lenna.png)

![Spinner](examples/assets/spinner.gif)

*Kitty/Sixel/iTerm2 nötig · q beendet*
)md";

class MarkdownDualImageRoot : public tuinator::VBox {
  public:
    explicit MarkdownDualImageRoot(tuinator::BoxOptions options) : tuinator::VBox(options) {}
    bool wants_full_screen() const override { return true; }
};

} // namespace

int main() {
    tuinator::Application app;
    const tuinator::Theme& theme = app.theme();

    auto root = std::make_unique<MarkdownDualImageRoot>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    root->add_child(std::make_unique<tuinator::Label>("MarkdownView — zwei Bilder", theme.heading));

    tuinator::MarkdownViewOptions view_options;
    view_options.styles = tuinator::markdown_styles_from_theme(theme);
    auto view = std::make_unique<tuinator::MarkdownView>(kSample, view_options);
    view->set_flex(1);

    auto panel = std::make_unique<tuinator::Panel>("Markdown", theme.border, theme.heading);
    panel->set_content(std::move(view));
    panel->set_flex(1);
    root->add_child(std::move(panel));

    root->add_child(std::make_unique<tuinator::Label>(
        "TUINATOR_GRAPHICS=kitty|iterm2|sixel|auto · Fenster hoch genug halten", theme.muted));

    app.set_root(std::move(root));
    return app.run();
}
