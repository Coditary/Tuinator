#include <tuinator/testing/widget_query.hpp>
#include <tuinator/widgets/containers/panel.hpp>
#include <tuinator/widgets/containers/scroll_view.hpp>
#include <tuinator/widgets/display/label.hpp>

#include "test_harness.hpp"

#include <memory>

TUINATOR_TEST(panel_finds_nested_test_id_in_scroll_content) {
    auto list = std::make_unique<tuinator::Label>("deep");
    list->set_test_id("leaf");
    auto scroll = std::make_unique<tuinator::ScrollView>(std::move(list));
    scroll->set_test_id("scroll");

    auto panel = std::make_unique<tuinator::Panel>("Box");
    panel->set_test_id("panel");
    panel->set_content(std::move(scroll));

    const tuinator::Widget* leaf = tuinator::testing::find_by_selector(*panel, "leaf");
    TUINATOR_CHECK(leaf != nullptr);
    TUINATOR_CHECK_EQ(leaf->test_id(), "leaf");
}

TUINATOR_TEST(panel_for_each_descendant_includes_content_tree) {
    auto panel = std::make_unique<tuinator::Panel>("T");
    auto inner = std::make_unique<tuinator::Label>("x");
    inner->set_test_id("inner");
    panel->set_content(std::move(inner));

    int count = 0;
    panel->for_each_descendant([&](tuinator::Widget*) { ++count; });
    TUINATOR_CHECK(count >= 2);
}
