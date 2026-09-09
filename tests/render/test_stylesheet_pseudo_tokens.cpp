#include <tuinator/layout/grid.hpp>
#include <tuinator/render/style_resolver.hpp>
#include <tuinator/render/stylesheet.hpp>
#include <tuinator/render/widget_option_schema.hpp>
#include <tuinator/widgets/controls/button.hpp>
#include <tuinator/widgets/controls/checkbox.hpp>
#include <tuinator/widgets/controls/combo_box.hpp>

#include "support/test_harness.hpp"

TUINATOR_TEST(stylesheet_pseudo_focused_selector) {
    const tuinator::Stylesheet sheet = tuinator::Stylesheet::load_from_string(R"(
Button {
  color: white;
}

Button:focused {
  color: black;
  background: cyan;
}
)");

    tuinator::Button button("OK");
    TUINATOR_CHECK_EQ(sheet.resolve_text_style(button).foreground, tuinator::Color::White);

    button.set_focused(true);
    const tuinator::Style focused = sheet.resolve_text_style(button);
    TUINATOR_CHECK_EQ(focused.foreground, tuinator::Color::Black);
    TUINATOR_CHECK_EQ(focused.background, tuinator::Color::Cyan);
}

TUINATOR_TEST(stylesheet_pseudo_checked_selector) {
    const tuinator::Stylesheet sheet = tuinator::Stylesheet::load_from_string(R"(
Checkbox {
  color: white;
}

Checkbox:checked {
  color: green;
}
)");

    tuinator::Checkbox checkbox("Flag", false);
    TUINATOR_CHECK_EQ(sheet.resolve_text_style(checkbox).foreground, tuinator::Color::White);

    checkbox.set_checked(true);
    TUINATOR_CHECK_EQ(sheet.resolve_text_style(checkbox).foreground, tuinator::Color::Green);
}

TUINATOR_TEST(stylesheet_theme_tokens_and_references) {
    const tuinator::Stylesheet sheet = tuinator::Stylesheet::load_from_string(R"(
@theme {
  accent: #58a6ff;
}

Button {
  color: $accent;
}
)");

    tuinator::Button button("Go");
    const tuinator::Style style = sheet.resolve_text_style(button);
    TUINATOR_CHECK(style.foreground_rgb.has_value());
    TUINATOR_CHECK_EQ(style.foreground_rgb->r, 0x58);
    TUINATOR_CHECK_EQ(style.foreground_rgb->g, 0xa6);
    TUINATOR_CHECK_EQ(style.foreground_rgb->b, 0xff);
}

TUINATOR_TEST(widget_option_schema_warns_on_unknown_option) {
    const tuinator::Stylesheet sheet = tuinator::Stylesheet::load_from_string(R"(
Grid {
  frist-size: 10;
}
)");

    tuinator::Grid grid;
    const tuinator::WidgetOptions options = sheet.resolve_options(grid);
    const std::vector<std::string> warnings = tuinator::validate_widget_options(grid, options);
    TUINATOR_CHECK(!warnings.empty());
}

TUINATOR_TEST(combo_box_open_pseudo_matches) {
    const tuinator::Stylesheet sheet = tuinator::Stylesheet::load_from_string(R"(
ComboBox {
  color: white;
}

ComboBox:open {
  color: yellow;
}
)");

    tuinator::ComboBox combo;
    combo.set_items({"One", "Two"});
    TUINATOR_CHECK_EQ(sheet.resolve_text_style(combo).foreground, tuinator::Color::White);

    combo.set_focused(true);
    combo.handle_event(tuinator::KeyPress{tuinator::Key::Enter});
    TUINATOR_CHECK(combo.is_open());
    TUINATOR_CHECK_EQ(sheet.resolve_text_style(combo).foreground, tuinator::Color::Yellow);
}
