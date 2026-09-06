#include <tuinator/widgets/chrome/source_control_panel.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(source_control_panel_renders_header_and_entry) {
    tuinator::MemoryTerminalBackend backend({56, 8});
    backend.init();

    tuinator::SourceControlPanelStyle style;
    style.panel_bg = tuinator::Rgb{0x16, 0x16, 0x1e};
    style.text_fg = tuinator::Rgb{0xc0, 0xca, 0xf5};
    style.muted_fg = tuinator::Rgb{0x56, 0x5f, 0x89};
    style.background = tuinator::style_fg_bg(style.text_fg, style.panel_bg);
    style.header = style.background;
    style.section_title = style.background;
    style.entry_text = style.background;
    style.muted = tuinator::style_fg_bg(style.muted_fg, style.panel_bg);
    style.footer = style.muted;

    tuinator::SourceControlPanel panel(style);
    panel.set_header("Source Control", "../Documents/git/diffview.nvim");
    panel.set_sections({
        {
            "Changes (1)",
            {
                {
                    .path = "file_history_panel.lua",
                    .status = tuinator::GitChangeStatus::Modified,
                    .additions = "55",
                    .deletions = "29",
                    .selected = true,
                },
            },
        },
    });
    panel.layout({0, 0, 56, 8});

    tuinator::test::render_root(panel, backend);

    TUINATOR_CHECK(tuinator::test::row_has(backend, "Source Control"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "Changes (1)"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "file_history_panel.lua"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "+55"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "-29"));
}

TUINATOR_TEST(source_control_panel_selection_callback) {
    tuinator::SourceControlPanel panel;
    panel.set_sections({
        {"Changes (2)",
         {
             {.path = "a.lua", .status = tuinator::GitChangeStatus::Modified},
             {.path = "b.lua", .status = tuinator::GitChangeStatus::Modified},
         }},
    });

    int calls = 0;
    std::string selected_path;
    panel.set_on_select([&](int, int, const tuinator::SourceControlEntry& entry) {
        ++calls;
        selected_path = entry.path;
    });

    panel.set_selected(0, 0);
    panel.set_focused(true);

    panel.handle_event(tuinator::KeyPress{tuinator::Key::Down});
    TUINATOR_CHECK_EQ(calls, 1);
    TUINATOR_CHECK_EQ(selected_path, "b.lua");
}
