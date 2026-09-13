#include <tuinator/render/style_resolver.hpp>
#include <tuinator/render/stylesheet.hpp>
#include <tuinator/widgets/containers/panel.hpp>
#include <tuinator/widgets/containers/split_pane.hpp>
#include <tuinator/widgets/controls/button.hpp>
#include <tuinator/widgets/display/label.hpp>
#include <tuinator/widgets/views/list_view.hpp>

#include "support/test_harness.hpp"

TUINATOR_TEST(stylesheet_parses_theme_and_rules) {
    const tuinator::Stylesheet sheet = tuinator::Stylesheet::load_from_string(R"(
@theme {
  glyph-set: unicode;
}

* {
  color: white;
}

Label {
  color: cyan;
}

#title {
  color: yellow;
  bold: true;
}

.sidebar Label {
  color: magenta;
}
)");

    TUINATOR_CHECK_EQ(sheet.theme_options().glyphs, tuinator::GlyphSet::Unicode);

    tuinator::Label typed("typed");
    TUINATOR_CHECK_EQ(sheet.resolve_text_style(typed).foreground, tuinator::Color::Cyan);

    tuinator::Panel global_panel("Global");
    TUINATOR_CHECK_EQ(sheet.resolve_border_style(global_panel).foreground, tuinator::Color::White);

    tuinator::Label titled("titled");
    titled.set_widget_id("title");
    const tuinator::Style title_style = sheet.resolve_text_style(titled);
    TUINATOR_CHECK_EQ(title_style.foreground, tuinator::Color::Yellow);
    TUINATOR_CHECK(title_style.bold);

    tuinator::Panel sidebar("Sidebar");
    sidebar.set_widget_id("sidebar");
    sidebar.add_widget_class("sidebar");
    auto nested = std::make_unique<tuinator::Label>("nested");
    sidebar.set_content(std::move(nested));
    TUINATOR_CHECK_EQ(sheet.resolve_text_style(*sidebar.content()).foreground, tuinator::Color::Magenta);
}

TUINATOR_TEST(stylesheet_resolves_splitpane_divider_color) {
    const tuinator::Stylesheet sheet = tuinator::Stylesheet::load_from_string(R"(
SplitPane {
  divider-color: dim cyan;
}

#workspace {
  divider-color: magenta;
}
)");

    tuinator::SplitPane split(nullptr, nullptr);
    const tuinator::Style generic = sheet.resolve_divider_style(split);
    TUINATOR_CHECK_EQ(generic.foreground, tuinator::Color::Cyan);
    TUINATOR_CHECK(generic.dim);

    split.set_widget_id("workspace");
    TUINATOR_CHECK_EQ(sheet.resolve_divider_style(split).foreground, tuinator::Color::Magenta);
}

TUINATOR_TEST(style_resolver_focused_and_selected) {
    const tuinator::Stylesheet sheet = tuinator::Stylesheet::load_from_string(R"(
Button {
  color: white;
  focused-color: black;
  focused-background: cyan;
}

ListView {
  color: white;
  selected-color: yellow;
}
)");

    const tuinator::Theme theme = tuinator::dark_theme();
    const tuinator::StyleResolver resolver{theme, &sheet};

    tuinator::Button button("OK");
    const tuinator::Style focused = resolver.focused(button, {});
    TUINATOR_CHECK_EQ(focused.foreground, tuinator::Color::Black);
    TUINATOR_CHECK_EQ(focused.background, tuinator::Color::Cyan);

    tuinator::ListView list;
    TUINATOR_CHECK_EQ(resolver.selected(list).foreground, tuinator::Color::Yellow);
}

TUINATOR_TEST(stylesheet_focused_preserves_widget_fallback) {
    const tuinator::Stylesheet sheet = tuinator::Stylesheet::load_from_string(R"(
Button {
  color: white;
  background: blue;
}
)");

    tuinator::Button button("OK");
    tuinator::Style focused_fallback{};
    focused_fallback.foreground = tuinator::Color::Black;
    focused_fallback.background = tuinator::Color::Cyan;

    const tuinator::Style focused = sheet.resolve_focused_style(button, focused_fallback);
    TUINATOR_CHECK_EQ(focused.foreground, tuinator::Color::Black);
    TUINATOR_CHECK_EQ(focused.background, tuinator::Color::Cyan);
}

TUINATOR_TEST(stylesheet_border_style_overrides_glyphs) {
    const tuinator::Stylesheet sheet = tuinator::Stylesheet::load_from_string(R"(
Panel {
  border-style: heavy;
}
)");

    tuinator::Panel panel("Panel");
    const tuinator::Theme theme = tuinator::dark_theme();
    const tuinator::BorderGlyphs glyphs = sheet.resolve_border_glyphs(panel, theme);
    const tuinator::BorderGlyphs heavy = tuinator::unicode_heavy_border_glyphs();
    TUINATOR_CHECK_EQ(glyphs.horizontal, heavy.horizontal);
}
