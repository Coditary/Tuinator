#include <tuinator/layout/box.hpp>
#include <tuinator/render/style_resolver.hpp>
#include <tuinator/render/stylesheet.hpp>
#include <tuinator/render/widget_options.hpp>
#include <tuinator/widgets/containers/panel.hpp>
#include <tuinator/widgets/containers/split_pane.hpp>
#include <tuinator/widgets/controls/text_input.hpp>
#include <tuinator/widgets/display/label.hpp>

#include "support/test_harness.hpp"

TUINATOR_TEST(widget_options_parses_bool_int_string) {
    tuinator::WidgetOptions options;
    options.set("gap", "4");
    options.set("outer-border", "true");
    options.set("orientation", "vertical");
    options.set("min_width", "32");

    TUINATOR_CHECK(options.has("gap"));
    TUINATOR_CHECK(options.has("outer-border"));
    TUINATOR_CHECK(options.has("min-width"));
    TUINATOR_CHECK_EQ(options.int_or("gap", 0), 4);
    TUINATOR_CHECK_EQ(options.int_or("min-width", 0), 32);
    TUINATOR_CHECK(options.bool_or("outer-border", false));
    TUINATOR_CHECK_EQ(options.string_or("orientation", "horizontal"), "vertical");
    TUINATOR_CHECK(!options.has("missing"));
    TUINATOR_CHECK_EQ(options.int_or("missing", 7), 7);
}

TUINATOR_TEST(stylesheet_resolves_widget_options_with_cascade) {
    const tuinator::Stylesheet sheet = tuinator::Stylesheet::load_from_string(R"(
SplitPane {
  first-size: 20;
  outer-border: false;
}

#workspace {
  first-size: 36;
  outer-border: true;
  min-pane-size: 8;
}
)");

    tuinator::SplitPane generic(nullptr, nullptr);
    TUINATOR_CHECK_EQ(sheet.resolve_options(generic).int_or("first-size", 0), 20);
    TUINATOR_CHECK(!sheet.resolve_options(generic).bool_or("outer-border", true));

    generic.set_widget_id("workspace");
    const tuinator::WidgetOptions workspace = sheet.resolve_options(generic);
    TUINATOR_CHECK_EQ(workspace.int_or("first-size", 0), 36);
    TUINATOR_CHECK(workspace.bool_or("outer-border", false));
    TUINATOR_CHECK_EQ(workspace.int_or("min-pane-size", 0), 8);
}

TUINATOR_TEST(apply_stylesheet_updates_split_pane_options) {
    const tuinator::Stylesheet sheet = tuinator::Stylesheet::load_from_string(R"(
#workspace {
  first-size: 40;
  outer-border: true;
  orientation: vertical;
  divider-hit-slop: 4;
}
)");

    tuinator::SplitPane split(nullptr, nullptr);
    split.set_widget_id("workspace");
    split.set_options({.first_size = 10, .outer_border = false});

    const tuinator::StyleResolver resolver{tuinator::dark_theme(), &sheet};
    split.apply_stylesheet(resolver);

    TUINATOR_CHECK_EQ(split.options().first_size, 40);
    TUINATOR_CHECK(split.options().outer_border);
    TUINATOR_CHECK_EQ(split.options().orientation, tuinator::SplitOrientation::Vertical);
    TUINATOR_CHECK_EQ(split.options().divider_hit_slop, 4);
}

TUINATOR_TEST(apply_stylesheet_updates_panel_border_edges) {
    const tuinator::Stylesheet sheet = tuinator::Stylesheet::load_from_string(R"(
Panel {
  border-top: false;
  border-left: false;
}
)");

    tuinator::Panel panel("Demo");
    const tuinator::StyleResolver resolver{tuinator::dark_theme(), &sheet};
    panel.apply_stylesheet(resolver);

    TUINATOR_CHECK(!panel.border_edges().top);
    TUINATOR_CHECK(panel.border_edges().right);
    TUINATOR_CHECK(panel.border_edges().bottom);
    TUINATOR_CHECK(!panel.border_edges().left);
}

TUINATOR_TEST(apply_stylesheet_updates_text_input_and_box_layout) {
    const tuinator::Stylesheet sheet = tuinator::Stylesheet::load_from_string(R"(
TextInput {
  min-width: 42;
  placeholder: typed-here;
}

VBox {
  gap: 3;
  padding: 2;
}
)");

    tuinator::TextInput input;
    tuinator::VBox box;
    const tuinator::StyleResolver resolver{tuinator::dark_theme(), &sheet};

    input.apply_stylesheet(resolver);
    box.apply_stylesheet(resolver);

    TUINATOR_CHECK_EQ(input.preferred_size().width, 44);
    TUINATOR_CHECK_EQ(input.placeholder(), "typed-here");

    TUINATOR_CHECK_EQ(box.gap(), 3);
    TUINATOR_CHECK_EQ(box.padding(), 2);
}

TUINATOR_TEST(text_input_set_options_overrides_constructor_defaults) {
    tuinator::TextInput input({.min_width = 10, .placeholder = "old"});
    input.set_options({.min_width = 28, .placeholder = "new"});
    TUINATOR_CHECK_EQ(input.preferred_size().width, 30);
    TUINATOR_CHECK_EQ(input.placeholder(), "new");
}

TUINATOR_TEST(apply_stylesheet_to_tree_applies_descendants) {
    const tuinator::Stylesheet sheet = tuinator::Stylesheet::load_from_string(R"(
#root {
  flex: 2;
}

Label {
  color: red;
}
)");

    auto root = std::make_unique<tuinator::VBox>();
    root->set_widget_id("root");
    auto child = std::make_unique<tuinator::Label>("child");
    root->add_child(std::move(child));

    const tuinator::StyleResolver resolver{tuinator::dark_theme(), &sheet};
    tuinator::apply_stylesheet_to_tree(*root, resolver);

    TUINATOR_CHECK_EQ(root->flex(), 2);
}
