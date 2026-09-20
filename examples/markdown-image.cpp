#include <tuinator/tuinator.hpp>

#include <memory>
#include <string>
#include <vector>

namespace {

const char* kSample = R"md(# Markdown Bilder

Drei Fälle im Stream:

## 1. Block-Bild (wird gerendert)

![Lenna Testbild](examples/assets/lenna.png)

## 2. Inline-Bild (bleibt Alt-Text)

Hier mitten im Satz ![Logo](examples/assets/lenna.png) fließt der Text weiter.

## 3. Fehlende Datei (Fallback)

![Gibts nicht](examples/assets/nope.png)

## 4. Animiertes GIF

![Spinner](examples/assets/spinner.gif)

*Stream beendet.* Drücke **q** zum Beenden — oder warte auf den Neustart.
)md";

class MarkdownImageDemoRoot : public tuinator::VBox {
  public:
    explicit MarkdownImageDemoRoot(tuinator::BoxOptions options) : tuinator::VBox(options) {}
    bool wants_full_screen() const override { return true; }
};

} // namespace

int main() {
    tuinator::Application app;
    const tuinator::Theme& theme = app.theme();

    auto root = std::make_unique<MarkdownImageDemoRoot>(tuinator::BoxOptions{.gap = 1, .padding = 1});

    root->add_child(std::make_unique<tuinator::Label>("MarkdownView — Bilder", theme.heading));

    tuinator::MarkdownViewOptions view_options;
    view_options.styles = tuinator::markdown_styles_from_theme(theme);
    view_options.follow_tail = true;
    auto view = std::make_unique<tuinator::MarkdownView>(view_options);
    view->set_flex(1);
    tuinator::MarkdownView* view_ptr = view.get();

    auto panel = std::make_unique<tuinator::Panel>("Image Stream", theme.border, theme.heading);
    panel->set_content(std::move(view));
    panel->set_flex(1);
    root->add_child(std::move(panel));

    root->add_child(std::make_unique<tuinator::Label>(
        "Braucht Kitty/Sixel/iTerm2 · TUINATOR_GRAPHICS=kitty erzwingt · q beendet", theme.muted));

    const std::string sample = kSample;
    std::vector<std::string> chunks;
    std::size_t pos = 0;
    while (pos < sample.size()) {
        std::size_t end = pos;
        while (end < sample.size() && sample[end] != ' ' && sample[end] != '\n') {
            ++end;
        }
        while (end < sample.size() && (sample[end] == ' ' || sample[end] == '\n') && chunks.size() % 3 != 2) {
            ++end;
            if (sample[end - 1] == '\n') {
                break;
            }
        }
        chunks.push_back(sample.substr(pos, end - pos));
        pos = end;
    }

    auto state = std::make_shared<std::size_t>(0);
    app.set_interval(30, [&app, view_ptr, chunks, state]() mutable {
        if (*state < chunks.size()) {
            view_ptr->append(chunks[*state]);
            ++(*state);
        } else if (*state < chunks.size() + 60) {
            ++(*state);
        } else {
            *state = 0;
            view_ptr->clear();
        }
    });

    app.set_root(std::move(root));
    return app.run();
}
