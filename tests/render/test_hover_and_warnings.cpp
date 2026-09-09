#include <tuinator/core/application.hpp>
#include <tuinator/layout/box.hpp>
#include <tuinator/render/stylesheet.hpp>
#include <tuinator/widgets/controls/button.hpp>
#include <tuinator/widgets/controls/toggle.hpp>
#include <tuinator/widgets/display/progress_bar.hpp>
#include <tuinator/widgets/views/tree_view.hpp>

#include "support/test_harness.hpp"

TUINATOR_TEST(stylesheet_hover_pseudo_selector) {
    const tuinator::Stylesheet sheet = tuinator::Stylesheet::load_from_string(R"(
Button {
  color: white;
}

Button:hover {
  color: yellow;
}
)");

    tuinator::Button button("Go");
    TUINATOR_CHECK_EQ(sheet.resolve_text_style(button).foreground, tuinator::Color::White);

    button.set_hovered(true);
    TUINATOR_CHECK_EQ(sheet.resolve_text_style(button).foreground, tuinator::Color::Yellow);
}

TUINATOR_TEST(application_collects_stylesheet_warnings) {
    tuinator::Application app;
    auto root = std::make_unique<tuinator::VBox>();
    root->add_child(std::make_unique<tuinator::ProgressBar>(0.5));
    app.set_root(std::move(root));

    app.set_stylesheet(tuinator::Stylesheet::load_from_string(R"(
ProgressBar {
  unknown-option: true;
}
)"));

    TUINATOR_CHECK(!app.stylesheet_warnings().empty());
}

TUINATOR_TEST(toggle_treeview_progressbar_have_type_names) {
    tuinator::Toggle toggle("Flag");
    tuinator::TreeView tree;
    tuinator::ProgressBar bar(0.5);

    TUINATOR_CHECK_EQ(toggle.widget_type_name(), "Toggle");
    TUINATOR_CHECK_EQ(tree.widget_type_name(), "TreeView");
    TUINATOR_CHECK_EQ(bar.widget_type_name(), "ProgressBar");
    TUINATOR_CHECK(tuinator::as_selectable_list(&tree) != nullptr);
}
