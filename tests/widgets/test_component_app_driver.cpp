#include <tuinator/core/screen.hpp>
#include <tuinator/layout/box.hpp>
#include <tuinator/testing/app_driver.hpp>
#include <tuinator/testing/widget_query.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/containers/panel.hpp>
#include <tuinator/widgets/containers/split_pane.hpp>
#include <tuinator/widgets/controls/button.hpp>
#include <tuinator/widgets/controls/checkbox.hpp>
#include <tuinator/widgets/controls/text_input.hpp>
#include <tuinator/widgets/controls/toggle.hpp>
#include <tuinator/widgets/display/label.hpp>

#include "component_expect.hpp"
#include "test_harness.hpp"

#include <memory>

namespace {

std::unique_ptr<tuinator::Widget> make_label(std::string text, std::string test_id = {}) {
    auto label = std::make_unique<tuinator::Label>(std::move(text));
    if (!test_id.empty()) {
        label->set_test_id(std::move(test_id));
    }
    return label;
}

} // namespace

TUINATOR_TEST(button_click_triggers_callback) {
    bool fired = false;
    auto root = tuinator::make_screen({.padding = 1});
    auto button = std::make_unique<tuinator::Button>("Go", [&fired]() { fired = true; });
    button->set_test_id("go");
    root->add_child(std::move(button));

    tuinator::testing::AppDriver driver({24, 6}, std::move(root));
    TUINATOR_CHECK(!fired);
    driver.click("go");
    TUINATOR_CHECK(fired);
}

TUINATOR_TEST(button_enter_when_focused_triggers_callback) {
    bool fired = false;
    auto root = tuinator::make_screen({.padding = 1});
    auto button = std::make_unique<tuinator::Button>("OK", [&fired]() { fired = true; });
    button->set_test_id("ok");
    root->add_child(std::move(button));

    tuinator::testing::AppDriver driver({24, 6}, std::move(root));
    driver.focus("ok");
    driver.press_key(tuinator::Key::Enter);
    TUINATOR_CHECK(fired);
}

TUINATOR_TEST(button_hover_marks_widget_hovered) {
    auto root = tuinator::make_screen({.padding = 1});
    auto button = std::make_unique<tuinator::Button>("Hover me");
    button->set_test_id("hover-btn");
    root->add_child(std::move(button));

    tuinator::testing::AppDriver driver({32, 6}, std::move(root));
    driver.hover("hover-btn");
    const tuinator::Widget* widget = tuinator::testing::find_by_selector(driver.root(), "hover-btn");
    TUINATOR_CHECK(widget != nullptr);
    TUINATOR_CHECK(widget->is_hovered());
}

TUINATOR_TEST(checkbox_space_toggles_checked_state) {
    auto root = tuinator::make_screen({.padding = 1});
    auto box = std::make_unique<tuinator::Checkbox>("Enable", false);
    box->set_test_id("enable");
    root->add_child(std::move(box));

    tuinator::testing::AppDriver driver({32, 6}, std::move(root));
    const auto* toggle = tuinator::as_toggle_control(&driver.require("enable"));
    TUINATOR_CHECK(toggle != nullptr);
    TUINATOR_CHECK(!toggle->is_checked());

    driver.focus("enable");
    driver.press_key(tuinator::Key::None, ' ');
    TUINATOR_CHECK(toggle->is_checked());
}

TUINATOR_TEST(text_input_shows_placeholder_then_typed_value) {
    auto root = tuinator::make_screen({.padding = 1});
    // Another focusable first so the field starts unfocused (placeholder is visible when empty).
    root->add_child(std::make_unique<tuinator::Button>("Other"));
    auto field = std::make_unique<tuinator::TextInput>(
        tuinator::TextInputOptions{.min_width = 16, .placeholder = "name here"});
    field->set_test_id("name");
    root->add_child(std::move(field));

    tuinator::testing::AppDriver driver({40, 8}, std::move(root));
    TUINATOR_CHECK(tuinator::test::expect::tree_contains(driver.ui_tree_json(), "\"placeholder\":\"name here\""));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "name here"));

    driver.type_into("name", "Ada");
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Ada"));
    TUINATOR_CHECK(tuinator::test::expect::tree_contains(driver.ui_tree_json(), "\"value\":\"Ada\""));
}

TUINATOR_TEST(vbox_lists_children_top_to_bottom) {
    auto root = tuinator::make_screen({.gap = 0, .padding = 1});
    root->add_child(make_label("Alpha", "a"));
    root->add_child(make_label("Beta", "b"));
    root->add_child(make_label("Gamma", "c"));

    tuinator::testing::AppDriver driver({24, 10}, std::move(root));
    const std::string screen = driver.snapshot_text();
    const std::size_t pos_a = screen.find("Alpha");
    const std::size_t pos_b = screen.find("Beta");
    const std::size_t pos_c = screen.find("Gamma");
    TUINATOR_CHECK(pos_a != std::string::npos);
    TUINATOR_CHECK(pos_b != std::string::npos);
    TUINATOR_CHECK(pos_c != std::string::npos);
    TUINATOR_CHECK(pos_a < pos_b);
    TUINATOR_CHECK(pos_b < pos_c);
}

TUINATOR_TEST(hbox_places_children_on_one_row) {
    auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});
    row->set_test_id("row");
    auto left = std::make_unique<tuinator::Button>("One");
    left->set_test_id("one");
    auto right = std::make_unique<tuinator::Button>("Two");
    right->set_test_id("two");
    row->add_child(std::move(left));
    row->add_child(std::move(right));

    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(std::move(row));

    tuinator::testing::AppDriver driver({40, 6}, std::move(root));
    const tuinator::Widget& one = driver.require("one");
    const tuinator::Widget& two = driver.require("two");
    TUINATOR_CHECK_EQ(one.bounds().y, two.bounds().y);
    TUINATOR_CHECK(one.bounds().x < two.bounds().x);
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "[One]"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "[Two]"));
}

TUINATOR_TEST(nested_panels_show_outer_and_inner_titles) {
    auto inner = std::make_unique<tuinator::Panel>("Inner");
    inner->set_test_id("inner-panel");
    inner->set_content(make_label("Workspace"));

    auto outer = std::make_unique<tuinator::Panel>("Outer");
    outer->set_test_id("outer-panel");
    outer->set_content(std::move(inner));

    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(std::move(outer));

    tuinator::testing::AppDriver driver({48, 14}, std::move(root));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Outer"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Inner"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Workspace"));
}

TUINATOR_TEST(split_pane_shows_both_pane_labels) {
    auto split = std::make_unique<tuinator::SplitPane>(make_label("LEFT"), make_label("RIGHT"),
                                                       tuinator::SplitPaneOptions{
                                                           .orientation = tuinator::SplitOrientation::Horizontal,
                                                           .first_size = 14,
                                                       });
    split->set_test_id("split");

    auto root = tuinator::make_screen({.padding = 0});
    root->add_child(std::move(split));

    tuinator::testing::AppDriver driver({50, 10}, std::move(root));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "LEFT"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "RIGHT"));
}

TUINATOR_TEST(terminal_resize_narrows_full_width_child) {
    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(make_label("Responsive", "fill"));

    tuinator::testing::AppDriver driver({60, 8}, std::move(root));
    const int wide = tuinator::test::expect::widget_bounds_width(driver, "fill");
    driver.resize_terminal({28, 8});
    const int narrow = tuinator::test::expect::widget_bounds_width(driver, "fill");

    TUINATOR_CHECK(wide > 0);
    TUINATOR_CHECK(narrow > 0);
    TUINATOR_CHECK(wide > narrow);
    TUINATOR_CHECK_EQ(driver.app().terminal_size().width, 28);
}

TUINATOR_TEST(nested_split_inside_panel_scales_within_screen) {
    auto split = std::make_unique<tuinator::SplitPane>(make_label("Editor"), make_label("Tree"),
                                                       tuinator::SplitPaneOptions{
                                                           .orientation = tuinator::SplitOrientation::Horizontal,
                                                           .first_size = 20,
                                                       });
    auto panel = std::make_unique<tuinator::Panel>("Project");
    panel->set_test_id("project");
    panel->set_content(std::move(split));

    auto root = tuinator::make_screen({.padding = 1});
    root->add_child(std::move(panel));

    tuinator::testing::AppDriver driver({72, 16}, std::move(root));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Project"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Editor"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Tree"));

    const tuinator::Widget& panel_widget = driver.require("project");
    TUINATOR_CHECK(panel_widget.bounds().width <= driver.app().terminal_size().width);
    TUINATOR_CHECK(panel_widget.bounds().height <= driver.app().terminal_size().height);

    driver.resize_terminal({40, 12});
    TUINATOR_CHECK(driver.require("project").bounds().width <= 40);
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Editor"));
}
