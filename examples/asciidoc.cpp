#include <tuinator/tuinator.hpp>

#include <memory>
#include <string>
#include <vector>

namespace {

const char* kSample = R"adoc(= Tuinator AsciiDoc

Streaming *AsciiDoc* direkt ins Terminal — _ohne_ Browser, ohne Flackern.

== Features

* Toleranter Parser: `*unclosed` bleibt *literal*
* Inline `code`, [line-through]#strike# und _italics_
* https://github.com[Links] und bare URLs wie https://example.com/docs
* Verschachtelte Listen:
. Erstens
. Zweitens
** verschachtelt

____
"Simplicity is prerequisite for reliability."
— Edsger W. Dijkstra
____

NOTE: Admonitions rendern als Quote mit fettem Label.

== Bilder

image::examples/assets/lenna.png[Lenna Testbild]

=== Code

[source,cpp]
----
#include <tuinator/tuinator.hpp>

int main() {
    tuinator::Application app;
    auto view = std::make_unique<tuinator::AsciiDocView>("= Hi");
    return 0;
}
----

'''

_Stream beendet._ Drücke *q* zum Beenden — oder warte auf den Neustart.
)adoc";

class AsciiDocDemoRoot : public tuinator::VBox {
  public:
    explicit AsciiDocDemoRoot(tuinator::BoxOptions options) : tuinator::VBox(options) {}
    bool wants_full_screen() const override { return true; }
};

} // namespace

int main() {
    tuinator::Application app;
    const tuinator::Theme& theme = app.theme();

    auto root = std::make_unique<AsciiDocDemoRoot>(tuinator::BoxOptions{.gap = 1, .padding = 1});

    root->add_child(std::make_unique<tuinator::Label>("AsciiDocView — Live Stream", theme.heading));

    tuinator::MarkdownViewOptions view_options;
    view_options.styles = tuinator::markdown_styles_from_theme(theme);
    view_options.follow_tail = true;
    auto view = std::make_unique<tuinator::AsciiDocView>(view_options);
    view->set_flex(1);
    tuinator::AsciiDocView* view_ptr = view.get();

    auto panel = std::make_unique<tuinator::Panel>("AI Response", theme.border, theme.heading);
    panel->set_content(std::move(view));
    panel->set_flex(1);
    root->add_child(std::move(panel));

    root->add_child(std::make_unique<tuinator::Label>(
        "Wheel/PgUp/PgDn scrollt · q beendet", theme.muted));

    // Simulated token stream: word-sized chunks like an AI backend emits.
    const std::string sample = kSample;
    std::vector<std::string> chunks;
    std::size_t pos = 0;
    while (pos < sample.size()) {
        std::size_t end = pos;
        while (end < sample.size() && sample[end] != ' ' && sample[end] != '\n') {
            ++end;
        }
        while (end < sample.size() && (sample[end] == ' ' || sample[end] == '\n') && chunks.size() % 3 != 2) {
            ++end; // keep some whitespace attached, but let newlines land separately
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
            ++(*state); // ~2s pause at the end
        } else {
            *state = 0;
            view_ptr->clear();
        }
    });

    app.set_root(std::move(root));
    return app.run();
}
