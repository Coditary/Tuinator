#include <tuinator/tuinator.hpp>

#include <memory>
#include <sstream>
#include <string>

namespace {

class FullscreenRoot : public tuinator::VBox {
  public:
    explicit FullscreenRoot(tuinator::BoxOptions options) : tuinator::VBox(options) {}

    bool wants_full_screen() const override { return true; }
};

std::unique_ptr<tuinator::Widget> make_long_text(const std::string& prefix, int count, const tuinator::Style& style,
                                                 bool wide) {
    auto list = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0, .padding = 1});
    for (int i = 1; i <= count; ++i) {
        std::string line = prefix + "  line " + std::to_string(i);
        if (wide) {
            line += "  |  lorem ipsum dolor sit amet consectetur adipiscing elit sed do eiusmod tempor";
        }
        list->add_child(std::make_unique<tuinator::Label>(line, style));
    }
    return list;
}

std::string format_offsets(const tuinator::ScrollView* left, const tuinator::ScrollView* right) {
    std::ostringstream out;
    if (left != nullptr) {
        out << "Left  x=" << left->scroll_x() << " y=" << left->scroll_y();
    }
    if (right != nullptr) {
        out << "   |   Right x=" << right->scroll_x() << " y=" << right->scroll_y();
    }
    return out.str();
}

} // namespace

int main() {
    tuinator::Application app;
    app.set_theme(tuinator::dark_theme({.glyphs = tuinator::GlyphSet::Unicode}));
    const tuinator::Theme& theme = app.theme();

    auto root = std::make_unique<FullscreenRoot>(tuinator::BoxOptions{.gap = 0, .padding = 0});

    auto header = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0, .padding = 1});
    header->add_child(std::make_unique<tuinator::Label>("SplitPane + dual scrollbars", theme.heading));
    header->add_child(
        std::make_unique<tuinator::Label>("Hover over the left or right scrollbar (or content) and wheel/drag. "
                                          "Only the pane under the cursor should move.",
                                          theme.muted));

    auto status = std::make_unique<tuinator::Label>("Left x=0 y=0   |   Right x=0 y=0", theme.accent);
    auto* status_ptr = status.get();
    header->add_child(std::move(status));
    root->add_child(std::move(header));

    auto left_panel = std::make_unique<tuinator::Panel>("Left pane", theme.border, theme.heading);
    left_panel->set_border_edges({false, false, false, false});
    tuinator::ScrollViewOptions left_options{
        .width = 24,
        .height = 12,
        .scrollbars = tuinator::scrollbar_options(theme, tuinator::ScrollbarPreset::Classic)
                          .with_thumb_style(tuinator::style_fg(tuinator::Rgb::hex(0x55AAFF))),
    };
    auto left_scroll =
        std::make_unique<tuinator::ScrollView>(make_long_text("LEFT", 48, theme.label, true), std::move(left_options));
    auto* left_ptr = left_scroll.get();
    left_panel->set_content(std::move(left_scroll));

    auto right_panel = std::make_unique<tuinator::Panel>("Right pane", theme.border, theme.heading);
    right_panel->set_border_edges({false, false, false, false});
    tuinator::ScrollViewOptions right_options{
        .width = 24,
        .height = 12,
        .scrollbars = tuinator::scrollbar_options(theme, tuinator::ScrollbarPreset::Thin)
                          .with_thumb_style(tuinator::style_fg(tuinator::Rgb::hex(0x66FF99))),
    };
    auto right_scroll = std::make_unique<tuinator::ScrollView>(make_long_text("RIGHT", 48, theme.success, true),
                                                               std::move(right_options));
    auto* right_ptr = right_scroll.get();
    right_panel->set_content(std::move(right_scroll));

    auto split = std::make_unique<tuinator::SplitPane>(std::move(left_panel), std::move(right_panel),
                                                       tuinator::SplitPaneOptions{
                                                           .orientation = tuinator::SplitOrientation::Horizontal,
                                                           .first_size = 40,
                                                           .outer_border = true,
                                                       });
    split->set_flex(1);
    root->add_child(std::move(split));

    root->add_child(std::make_unique<tuinator::StatusBar>(
        "q quit  |  drag divider  |  wheel on scrollbar track/thumb/content  |  wide text enables horizontal scroll",
        theme.text_input_focused));

    app.set_root(std::move(root));

    std::string last_status;
    app.set_interval(100, [status_ptr, left_ptr, right_ptr, &last_status]() {
        const std::string next = format_offsets(left_ptr, right_ptr);
        if (next != last_status) {
            last_status = next;
            status_ptr->set_text(next);
        }
    });

    return app.run();
}
