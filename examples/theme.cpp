#include <tuinator/tuinator.hpp>

#include <memory>
#include <string>

namespace {

void add_style_row(tuinator::VBox& box, const tuinator::Theme& theme, const char* name, const tuinator::Style& style) {
    auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});
    row->add_child(std::make_unique<tuinator::Label>(name, theme.muted));
    row->add_child(std::make_unique<tuinator::Label>("The quick brown fox", style));
    box.add_child(std::move(row));
}

std::unique_ptr<tuinator::Widget> build_glyph_sample(
    const tuinator::Theme& theme,
    const char* title,
    const char* body) {
    auto panel = std::make_unique<tuinator::Panel>(title, theme.border, theme.heading, theme.glyphs);
    auto content = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    content->add_child(std::make_unique<tuinator::Label>(body, theme.label));
    content->add_child(std::make_unique<tuinator::Button>("Action", [] {}, theme.button));
    panel->set_content(std::move(content));
    return panel;
}

std::unique_ptr<tuinator::Widget> build_color_preview(const tuinator::Theme& theme, const char* title) {
    auto box = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    box->add_child(std::make_unique<tuinator::Label>(title, theme.heading));
    add_style_row(*box, theme, "label", theme.label);
    add_style_row(*box, theme, "accent", theme.accent);
    add_style_row(*box, theme, "muted", theme.muted);
    return box;
}

} // namespace

int main() {
    tuinator::Application app;
    const tuinator::Theme base = app.theme();

    auto root = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    root->add_child(std::make_unique<tuinator::Label>("Themes and border glyphs", base.heading));

    auto glyph_row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});
    glyph_row->add_child(build_glyph_sample(
        tuinator::dark_theme({.glyphs = tuinator::GlyphSet::Ascii}),
        "ASCII",
        "Fallback + - | borders"));
    glyph_row->add_child(build_glyph_sample(
        tuinator::dark_theme({.glyphs = tuinator::GlyphSet::Unicode}),
        "Unicode",
        "Box drawing: corners and lines"));
    glyph_row->add_child(build_glyph_sample(
        tuinator::dark_theme({.glyphs = tuinator::GlyphSet::UnicodeRounded}),
        "Rounded",
        "Softer corners where supported"));
    root->add_child(std::move(glyph_row));

    root->add_child(std::make_unique<tuinator::Separator>(base.border));

    auto colors = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});
    colors->add_child(build_color_preview(tuinator::dark_theme(), "Dark"));
    colors->add_child(build_color_preview(tuinator::light_theme(), "Light"));
    root->add_child(std::move(colors));

    const tuinator::GlyphSet detected = tuinator::detect_glyph_set();
    const char* detected_name = "ASCII";
    if (detected == tuinator::GlyphSet::Unicode) {
        detected_name = "Unicode";
    } else if (detected == tuinator::GlyphSet::UnicodeRounded) {
        detected_name = "Rounded";
    }

    root->add_child(std::make_unique<tuinator::Label>(
        std::string("Auto-detected glyphs: ") + detected_name
            + " | TUINATOR_GLYPHS=ascii|unicode|rounded | q to quit",
        base.muted));

    app.set_root(std::move(root));
    return app.run();
}
