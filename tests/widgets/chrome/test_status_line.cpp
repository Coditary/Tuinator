#include <tuinator/widgets/chrome/status_line.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(status_line_renders_left_and_right_segments) {
    tuinator::MemoryTerminalBackend backend({120, 1});
    backend.init();

    tuinator::StatusLineStyle style;
    style.background = tuinator::style_fg_bg(tuinator::Rgb{0xc0, 0xca, 0xf5}, tuinator::Rgb{0x16, 0x16, 0x1e});
    style.accent_bar_rgb = tuinator::Rgb{0x7d, 0xcf, 0xff};

    tuinator::StatusLine status(style);
    status.set_left({
        {
            .kind = tuinator::StatusSegmentKind::Box,
            .text = " DIFFVIEWFILES ",
            .foreground_rgb = tuinator::Rgb{0xc0, 0xca, 0xf5},
            .background_rgb = tuinator::Rgb{0x16, 0x16, 0x1e},
        },
        {
            .kind = tuinator::StatusSegmentKind::Separator,
            .line_icon = tuinator::LineIcon::RightHalfCircleThin,
            .foreground_rgb = tuinator::Rgb{0x7d, 0xcf, 0xff},
        },
        {
            .text = "LUA",
            .foreground_rgb = tuinator::Rgb{0xc0, 0xca, 0xf5},
            .bold = true,
        },
        {
            .kind = tuinator::StatusSegmentKind::Pill,
            .icon = tuinator::FileIcon::Lua,
            .text = " file_history_panel.lua",
            .foreground_rgb = tuinator::Rgb{0x7d, 0xcf, 0xff},
        },
    });
    status.set_right({
        {
            .kind = tuinator::StatusSegmentKind::Pill,
            .text = " NORMAL ",
            .foreground_rgb = tuinator::Rgb{0x1a, 0x1b, 0x26},
            .background_rgb = tuinator::Rgb{0x7d, 0xcf, 0xff},
            .bold = true,
        },
        {
            .ui_icon = tuinator::UiIcon::DiffAdded,
            .text = "43",
            .foreground_rgb = tuinator::Rgb{0x9e, 0xce, 0x6a},
        },
        {
            .ui_icon = tuinator::UiIcon::GitBranch,
            .text = " feat/file-history",
            .foreground_rgb = tuinator::Rgb{0xbb, 0x9a, 0xf7},
        },
    });
    status.layout({0, 0, 120, 1});

    tuinator::test::render_root(status, backend);

    TUINATOR_CHECK(tuinator::test::row_has(backend, "DIFFVIEWFILES"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "LUA"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "43"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "feat/file-history"));
}
