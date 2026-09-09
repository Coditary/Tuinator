#include <tuinator/layout/box.hpp>
#include <tuinator/layout/grid.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/containers/panel.hpp>
#include <tuinator/widgets/containers/scroll_view.hpp>
#include <tuinator/widgets/containers/split_pane.hpp>
#include <tuinator/widgets/controls/button.hpp>
#include <tuinator/widgets/controls/checkbox.hpp>
#include <tuinator/widgets/controls/combo_box.hpp>
#include <tuinator/widgets/controls/slider.hpp>
#include <tuinator/widgets/controls/text_area.hpp>
#include <tuinator/widgets/display/label.hpp>

#include "support/test_harness.hpp"

TUINATOR_TEST(widget_roles_pane_and_split_queries) {
    tuinator::Panel panel("Explorer");
    auto split = std::make_unique<tuinator::SplitPane>(nullptr, nullptr);

    TUINATOR_CHECK(tuinator::as_pane(&panel) != nullptr);
    TUINATOR_CHECK(tuinator::as_bordered_pane(&panel) != nullptr);
    TUINATOR_CHECK(tuinator::as_single_child_container(&panel) != nullptr);
    TUINATOR_CHECK(tuinator::as_splittable(&panel) == nullptr);

    TUINATOR_CHECK(tuinator::as_binary_container(split.get()) != nullptr);
    TUINATOR_CHECK(tuinator::as_splittable(split.get()) != nullptr);
    TUINATOR_CHECK(tuinator::as_pane(split.get()) == nullptr);
}

TUINATOR_TEST(widget_roles_layout_and_input_queries) {
    tuinator::VBox box;
    tuinator::Label label("hello");
    tuinator::Button button("OK");
    tuinator::Checkbox checkbox("Flag", true);

    TUINATOR_CHECK(tuinator::as_layout_box(&box) != nullptr);
    TUINATOR_CHECK(tuinator::as_text_display(&label) != nullptr);
    TUINATOR_CHECK_EQ(tuinator::as_text_display(&label)->display_text(), "hello");
    TUINATOR_CHECK(tuinator::as_activatable(&button) != nullptr);
    TUINATOR_CHECK(tuinator::as_toggle_control(&checkbox) != nullptr);
    TUINATOR_CHECK(checkbox.is_checked());
}

TUINATOR_TEST(scroll_view_is_single_child_container) {
    auto content = std::make_unique<tuinator::Label>("scroll");
    tuinator::ScrollView view(std::move(content));

    TUINATOR_CHECK(tuinator::as_single_child_container(&view) != nullptr);
    TUINATOR_CHECK(view.content() != nullptr);

    view.set_content(std::make_unique<tuinator::Label>("replaced"));
    TUINATOR_CHECK_EQ(tuinator::as_text_display(view.content())->display_text(), "replaced");
}

TUINATOR_TEST(button_activate_invokes_callback) {
    bool clicked = false;
    tuinator::Button button("Go", [&clicked]() { clicked = true; });
    tuinator::as_activatable(&button)->activate();
    TUINATOR_CHECK(clicked);
}

TUINATOR_TEST(widget_capabilities_registry) {
    tuinator::Grid grid;
    tuinator::Slider slider(0, 10, 5);
    tuinator::TextArea area;
    tuinator::ComboBox combo;

    TUINATOR_CHECK(tuinator::widget_has_capability(grid, tuinator::WidgetCapability::LayoutBox));
    TUINATOR_CHECK(tuinator::widget_has_capability(slider, tuinator::WidgetCapability::ValueControl));
    TUINATOR_CHECK(tuinator::widget_has_capability(area, tuinator::WidgetCapability::MultiLineTextInput));
    TUINATOR_CHECK(tuinator::widget_has_capability(combo, tuinator::WidgetCapability::SelectableList));
}

TUINATOR_TEST(widget_pseudo_state_matching) {
    tuinator::Checkbox checkbox("Flag", true);
    TUINATOR_CHECK(tuinator::widget_matches_pseudo(checkbox, tuinator::WidgetPseudoState::Checked));

    checkbox.set_checked(false);
    TUINATOR_CHECK(!tuinator::widget_matches_pseudo(checkbox, tuinator::WidgetPseudoState::Checked));
}
