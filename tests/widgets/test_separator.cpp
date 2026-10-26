#include <tuinator/widgets/display/separator.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(separator_preferred_size_is_single_cell) {
    tuinator::Separator separator;
    const tuinator::Size size = separator.preferred_size();
    TUINATOR_CHECK_EQ(size.width, 1);
    TUINATOR_CHECK_EQ(size.height, 1);
}

TUINATOR_TEST(separator_paints_horizontal_rule_across_width) {
    tuinator::MemoryTerminalBackend backend({24, 3});
    backend.init();

    tuinator::Separator separator;
    separator.layout({0, 0, 24, 1});

    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    separator.paint(ctx);

    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 0, 0), '-');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 23, 0), '-');
}
