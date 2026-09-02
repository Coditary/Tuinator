#include <tuinator/tuinator.hpp>

#include <memory>
#include <string>
#include <vector>

namespace {

class ThrobberRoot : public tuinator::VBox {
public:
    explicit ThrobberRoot(tuinator::BoxOptions options)
        : tuinator::VBox(options) {}

    bool wants_full_screen() const override { return true; }
};

} // namespace

int main() {
    tuinator::Application app;
    const tuinator::Theme& theme = app.theme();

    auto content = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    content->add_child(std::make_unique<tuinator::Label>(
        "Press q to exit. Default throbber (random-looking step) plus named sets.",
        theme.heading));
    content->add_child(std::make_unique<tuinator::Label>(
        "Throbbers / loading spinners",
        theme.muted));

    std::vector<tuinator::Throbber*> throbbers;
    auto grid = std::make_unique<tuinator::Grid>(tuinator::GridOptions{.columns = 3, .gap = 2});

    for (const tuinator::ThrobberSet& set : tuinator::all_throbber_sets()) {
        auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 1});
        auto throbber = std::make_unique<tuinator::Throbber>(set, theme.accent);
        throbbers.push_back(throbber.get());
        row->add_child(std::move(throbber));
        row->add_child(std::make_unique<tuinator::Label>(set.title, theme.label));
        grid->add_child(std::move(row));
    }

    content->add_child(std::move(grid));
    content->add_child(std::make_unique<tuinator::Label>(
        "q to quit",
        theme.muted));

    auto scroll = std::make_unique<tuinator::ScrollView>(
        std::move(content),
        tuinator::ScrollViewOptions{.width = 90, .height = 24});
    scroll->set_flex(1);

    auto root = std::make_unique<ThrobberRoot>(tuinator::BoxOptions{.gap = 0, .padding = 0});
    root->add_child(std::move(scroll));
    app.set_root(std::move(root));

    app.set_interval(50, [throbbers]() {
        for (tuinator::Throbber* throbber : throbbers) {
            throbber->tick(50);
        }
    });

    return app.run();
}
