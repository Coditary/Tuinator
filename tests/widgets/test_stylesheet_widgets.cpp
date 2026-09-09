#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/render/style_resolver.hpp>
#include <tuinator/render/stylesheet.hpp>
#include <tuinator/widgets/containers/panel.hpp>
#include <tuinator/widgets/containers/split_pane.hpp>
#include <tuinator/widgets/controls/button.hpp>
#include <tuinator/widgets/display/label.hpp>

#include "support/render_helper.hpp"
#include "support/test_harness.hpp"

namespace {

tuinator::Stylesheet make_demo_sheet() {
    return tuinator::Stylesheet::load_from_string(R"(
@theme {
  glyph-set: unicode;
}

Button {
  color: white;
  focused-color: black;
  focused-background: cyan;
}

#title {
  color: yellow;
  bold: true;
}

#workspace {
  outer-border: true;
  first-size: 8;
}
)");
}

} // namespace

TUINATOR_TEST(stylesheet_button_focused_style_renders) {
    tuinator::MemoryTerminalBackend backend;
    backend.resize({24, 3});

    const tuinator::Stylesheet sheet = make_demo_sheet();
    const tuinator::Theme theme = tuinator::dark_theme({.glyphs = tuinator::GlyphSet::Unicode});
    const tuinator::StyleResolver resolver{theme, &sheet};

    tuinator::Button button("OK");
    button.set_focused(true);
    button.apply_stylesheet(resolver);
    button.layout({0, 0, 8, 1});

    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx{canvas, theme, &sheet};
    button.paint(ctx);
    backend.end_frame();

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "[OK]"));
}

TUINATOR_TEST(stylesheet_label_color_renders_from_resolver) {
    tuinator::MemoryTerminalBackend backend;
    backend.resize({24, 3});

    const tuinator::Stylesheet sheet = make_demo_sheet();
    const tuinator::Theme theme = tuinator::dark_theme();
    const tuinator::StyleResolver resolver{theme, &sheet};

    tuinator::Label title("Title");
    title.set_widget_id("title");
    title.apply_stylesheet(resolver);
    title.layout({0, 0, 10, 1});

    const tuinator::Style resolved = resolver.text(title, {});
    TUINATOR_CHECK_EQ(resolved.foreground, tuinator::Color::Yellow);
    TUINATOR_CHECK(resolved.bold);
}

TUINATOR_TEST(stylesheet_split_pane_outer_border_option_enables_frame) {
    const tuinator::Stylesheet sheet = make_demo_sheet();
    const tuinator::Theme theme = tuinator::dark_theme({.glyphs = tuinator::GlyphSet::Unicode});
    const tuinator::StyleResolver resolver{theme, &sheet};

    auto left = std::make_unique<tuinator::Label>("L");
    auto right = std::make_unique<tuinator::Label>("R");
    tuinator::SplitPane split(std::move(left), std::move(right));
    split.set_widget_id("workspace");
    split.apply_stylesheet(resolver);

    TUINATOR_CHECK(split.wants_outer_border());
    TUINATOR_CHECK_EQ(split.options().first_size, 8);
}

TUINATOR_TEST(stylesheet_hex_color_and_background) {
    const tuinator::Stylesheet sheet = tuinator::Stylesheet::load_from_string(R"(
Label {
  color: #ff00ff;
  background: #101010;
}
)");

    tuinator::Label label("x");
    const tuinator::Style style = sheet.resolve_text_style(label);
    TUINATOR_CHECK(style.foreground_rgb.has_value());
    TUINATOR_CHECK_EQ(style.foreground_rgb->r, 255);
    TUINATOR_CHECK_EQ(style.foreground_rgb->g, 0);
    TUINATOR_CHECK_EQ(style.foreground_rgb->b, 255);
    TUINATOR_CHECK(style.background_rgb.has_value());
    TUINATOR_CHECK_EQ(style.background_rgb->r, 16);
}

TUINATOR_TEST(stylesheet_comments_do_not_break_id_selectors) {
    const tuinator::Stylesheet sheet = tuinator::Stylesheet::load_from_string(R"(
# comment line

#title {
  color: green;
}
)");

    tuinator::Label label("x");
    label.set_widget_id("title");
    TUINATOR_CHECK_EQ(sheet.resolve_text_style(label).foreground, tuinator::Color::Green);
}
