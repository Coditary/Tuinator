#include <tuinator/core/event.hpp>
#include <tuinator/layout/box.hpp>
#include <tuinator/widgets/containers/panel.hpp>
#include <tuinator/widgets/containers/scroll_view.hpp>
#include <tuinator/widgets/containers/split_pane.hpp>
#include <tuinator/widgets/containers/tabs.hpp>
#include <tuinator/widgets/display/label.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

#include <memory>

namespace {

std::unique_ptr<tuinator::Widget> label(std::string text) {
    return std::make_unique<tuinator::Label>(std::move(text));
}

} // namespace

TUINATOR_TEST(edge_panel_without_content_has_positive_preferred_size) {
    tuinator::Panel panel("title");
    const tuinator::Size size = panel.preferred_size();
    TUINATOR_CHECK(size.width > 0);
    TUINATOR_CHECK(size.height > 0);
}

TUINATOR_TEST(edge_panel_tiny_bounds_content_clipped_not_crash) {
    tuinator::MemoryTerminalBackend backend({6, 4});
    backend.init();
    tuinator::Panel panel("T");
    panel.set_content(label("inner-body"));
    panel.layout({0, 0, 4, 3});
    tuinator::test::render_root(panel, backend);
    TUINATOR_CHECK(tuinator::test::row_has(backend, "T"));
}

TUINATOR_TEST(edge_scroll_view_scroll_past_end_clamps) {
    auto content = std::make_unique<tuinator::Label>("only-line");
    tuinator::ScrollView scroll(std::move(content), tuinator::ScrollViewOptions{.height = 4});
    scroll.layout({0, 0, 12, 4});
    scroll.scroll_to(0, 999);
    scroll.scroll_to(999, 999);

    tuinator::MemoryTerminalBackend backend({12, 4});
    backend.init();
    tuinator::test::render_root(scroll, backend);
    TUINATOR_CHECK(tuinator::test::row_has(backend, "only-line"));
}

TUINATOR_TEST(edge_scroll_view_content_smaller_than_viewport) {
    auto content = std::make_unique<tuinator::Label>("short");
    tuinator::ScrollView scroll(std::move(content), tuinator::ScrollViewOptions{.width = 20, .height = 8});
    scroll.layout({0, 0, 20, 8});
    scroll.scroll_to(0, 5);

    tuinator::MemoryTerminalBackend backend({20, 8});
    backend.init();
    tuinator::test::render_root(scroll, backend);
    TUINATOR_CHECK(tuinator::test::row_has(backend, "short"));
}

TUINATOR_TEST(edge_scroll_view_home_key_resets_offset) {
    auto list = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    for (int i = 0; i < 10; ++i) {
        list->add_child(label("L" + std::to_string(i)));
    }
    tuinator::ScrollView scroll(std::move(list), tuinator::ScrollViewOptions{.height = 3});
    scroll.layout({0, 0, 10, 3});
    scroll.scroll_to(0, 7);
    TUINATOR_CHECK(scroll.handle_event(tuinator::KeyPress{tuinator::Key::Home}));

    tuinator::MemoryTerminalBackend backend({10, 3});
    backend.init();
    tuinator::test::render_root(scroll, backend);
    TUINATOR_CHECK(tuinator::test::row_has(backend, "L0"));
}

TUINATOR_TEST(edge_split_pane_first_size_larger_than_width_clamps) {
    auto split = std::make_unique<tuinator::SplitPane>(
        label("L"), label("R"),
        tuinator::SplitPaneOptions{.orientation = tuinator::SplitOrientation::Horizontal, .first_size = 500});
    split->layout({0, 0, 20, 5});
    const int left_width = split->first_child() ? split->first_child()->bounds().width : -1;
    TUINATOR_CHECK(left_width >= 0);
    TUINATOR_CHECK(left_width < 20);
}

TUINATOR_TEST(edge_split_pane_zero_height_children_safe) {
    auto split = std::make_unique<tuinator::SplitPane>(label("a"), label("b"));
    split->layout({0, 0, 10, 0});
    tuinator::MemoryTerminalBackend backend({10, 1});
    backend.init();
    tuinator::test::render_root(*split, backend);
    TUINATOR_CHECK(true);
}

TUINATOR_TEST(edge_tabs_empty_add_tab_safe) {
    tuinator::Tabs tabs;
    tabs.layout({0, 0, 20, 6});
    TUINATOR_CHECK_EQ(tabs.tab_count(), 0);
    TUINATOR_CHECK(tabs.active_content() == nullptr);
}

TUINATOR_TEST(edge_tabs_selected_index_clamps) {
    tuinator::Tabs tabs;
    tabs.add_tab("A", label("one"));
    tabs.add_tab("B", label("two"));
    tabs.set_selected_index(9);
    TUINATOR_CHECK_EQ(tabs.selected_index(), 1);
    tabs.set_selected_index(-3);
    TUINATOR_CHECK_EQ(tabs.selected_index(), 0);
}

TUINATOR_TEST(edge_tabs_left_at_first_tab_stays) {
    tuinator::Tabs tabs;
    tabs.add_tab("A", label("a"));
    tabs.add_tab("B", label("b"));
    tabs.layout({0, 0, 24, 6});
    tabs.set_focused(true);
    TUINATOR_CHECK(tabs.handle_event(tuinator::KeyPress{tuinator::Key::Left}));
    TUINATOR_CHECK_EQ(tabs.selected_index(), 0);
}

TUINATOR_TEST(edge_tabs_number_key_out_of_range_ignored) {
    tuinator::Tabs tabs;
    tabs.add_tab("A", label("a"));
    tabs.layout({0, 0, 16, 5});
    TUINATOR_CHECK(!tabs.handle_event(tuinator::KeyPress{.character = '9'}));
    TUINATOR_CHECK_EQ(tabs.selected_index(), 0);
}
