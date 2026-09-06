#include <tuinator/widgets/display/throbber.hpp>

#include <string>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(throbber_ascii_advances_frames) {
    tuinator::Throbber throbber("ascii");
    const std::string first = throbber.frame();
    throbber.tick();
    TUINATOR_CHECK(throbber.frame() != first);
    TUINATOR_CHECK(std::string(throbber.name()) == "line");
}

TUINATOR_TEST(throbber_named_dots_exists) {
    const tuinator::ThrobberSet* dots = tuinator::throbber_set_named("dots");
    TUINATOR_CHECK(dots != nullptr);
    TUINATOR_CHECK(std::string(dots->id) == "braille");
    TUINATOR_CHECK(std::string(dots->title) == "Braille");
    TUINATOR_CHECK(dots->frames.size() >= 4);
}

TUINATOR_TEST(throbber_unknown_name_falls_back) {
    tuinator::Throbber throbber("not-a-real-set");
    TUINATOR_CHECK(!throbber.frame().empty());
}

TUINATOR_TEST(throbber_catalog_is_populated) { TUINATOR_CHECK(tuinator::all_throbber_sets().size() > 20); }

TUINATOR_TEST(throbber_short_frame_clears_previous_cells) {
    tuinator::ThrobberSet set;
    set.id = "shrink";
    set.title = "Shrink";
    set.interval_ms = 80;
    set.frames = {"xxxx", "y"};

    tuinator::Throbber throbber(set);
    throbber.layout({0, 0, 4, 1});

    tuinator::MemoryTerminalBackend backend({8, 1});
    backend.init();
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    throbber.set_frame(0);
    throbber.paint(ctx);
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 0, 0), 'x');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 3, 0), 'x');

    throbber.set_frame(1);
    throbber.paint(ctx);
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 0, 0), 'y');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 1, 0), ' ');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 3, 0), ' ');
}
