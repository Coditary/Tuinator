#include <tuinator/widgets/chrome/status_bar.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(status_bar_renders_text) {
    tuinator::MemoryTerminalBackend backend({40, 2});
    backend.init();

    tuinator::StatusBar bar("Ready — all systems nominal");
    bar.layout({0, 0, 40, 1});

    tuinator::test::render_root(bar, backend);
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "Ready"));
}

TUINATOR_TEST(status_bar_set_text_updates_content) {
    tuinator::StatusBar bar("old");
    bar.set_text("new status");
    TUINATOR_CHECK_EQ(bar.text(), "new status");
}
