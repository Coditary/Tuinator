#include <tuinator/layout/box.hpp>
#include <tuinator/testing/widget_query.hpp>
#include <tuinator/widgets/containers/scroll_view.hpp>
#include <tuinator/widgets/display/label.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

#include <memory>

TUINATOR_TEST(scroll_view_finds_descendant_test_id) {
    auto content = std::make_unique<tuinator::Label>("row");
    content->set_test_id("row-id");
    tuinator::ScrollView scroll(std::move(content));
    scroll.set_test_id("scroll");
    scroll.layout({0, 0, 20, 6});

    const tuinator::Widget* row = tuinator::testing::find_by_selector(scroll, "row-id");
    TUINATOR_CHECK(row != nullptr);
}

TUINATOR_TEST(scroll_view_scroll_to_hides_top_rows) {
    tuinator::MemoryTerminalBackend backend({20, 6});
    backend.init();

    auto list = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    for (int i = 0; i < 12; ++i) {
        list->add_child(std::make_unique<tuinator::Label>("line-" + std::to_string(i)));
    }

    tuinator::ScrollView scroll(std::move(list), tuinator::ScrollViewOptions{.height = 4});
    scroll.layout({0, 0, 20, 4});
    scroll.scroll_to(0, 8);

    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    scroll.paint(ctx);

    TUINATOR_CHECK(!tuinator::test::row_contains(backend, 0, "line-0"));
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "line-8"));
}
