#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/controls/combo_box.hpp>
#include <tuinator/widgets/controls/slider.hpp>
#include <tuinator/widgets/display/progress_bar.hpp>
#include <tuinator/widgets/display/spinner.hpp>
#include <tuinator/widgets/menu/menu_bar.hpp>
#include <tuinator/widgets/views/tree_view.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(progress_bar_plain_renders_arrow) {
    tuinator::MemoryTerminalBackend backend({20, 3});
    backend.init();

    tuinator::ProgressBar bar(0.5, tuinator::progress_bar_preset(tuinator::ProgressBarLayout::Plain,
                                                                 tuinator::Theme{}.accent, tuinator::Theme{}.muted));
    bar.layout({0, 0, 20, 1});

    backend.begin_frame();
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    bar.paint(ctx);
    backend.end_frame();

    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 0, 0), '=');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 8, 0), '=');
    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 9, 0), '>');
}

TUINATOR_TEST(progress_bar_filled_label_centers_text) {
    tuinator::MemoryTerminalBackend backend({30, 3});
    backend.init();

    auto options = tuinator::progress_bar_preset(tuinator::ProgressBarLayout::FilledLabel, tuinator::Theme{}.accent,
                                                 tuinator::Theme{}.muted);
    options.inside_label = "Working";
    options.min_width = 30;

    tuinator::ProgressBar bar(0.5, options);
    bar.layout({0, 0, 30, 1});

    backend.begin_frame();
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    bar.paint(ctx);
    backend.end_frame();

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "Working"));
}

TUINATOR_TEST(progress_bar_pulse_renders_placeholders) {
    tuinator::MemoryTerminalBackend backend({40, 3});
    backend.init();

    auto options = tuinator::progress_bar_preset(tuinator::ProgressBarLayout::Pulse, tuinator::Theme{}.accent,
                                                 tuinator::Theme{}.muted);
    options.animation_phase = 4;

    tuinator::ProgressBar bar(0.0, options);
    bar.layout({0, 0, 40, 1});

    backend.begin_frame();
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    bar.paint(ctx);
    backend.end_frame();

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "--%"));
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "--:--:--"));
}

TUINATOR_TEST(progress_bar_shimmer_renders_label) {
    tuinator::MemoryTerminalBackend backend({36, 3});
    backend.init();

    tuinator::ProgressBar bar(0.0, tuinator::progress_bar_preset(tuinator::ProgressBarLayout::Shimmer,
                                                                 tuinator::Theme{}.accent, tuinator::Theme{}.muted));
    bar.layout({0, 0, 36, 1});
    bar.set_animation_phase(2);

    backend.begin_frame();
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    bar.paint(ctx);
    backend.end_frame();

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "Loading..."));
}

TUINATOR_TEST(progress_bar_bounce_renders_brackets) {
    tuinator::MemoryTerminalBackend backend({16, 3});
    backend.init();

    tuinator::ProgressBar bar(0.0, tuinator::progress_bar_preset(tuinator::ProgressBarLayout::Bounce,
                                                                 tuinator::Theme{}.accent, tuinator::Theme{}.muted));
    bar.layout({0, 0, 16, 1});
    bar.set_animation_phase(1);

    backend.begin_frame();
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    bar.paint(ctx);
    backend.end_frame();

    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 0, 0), '[');
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "]"));
}

TUINATOR_TEST(progress_bar_task_row_renders_percent_and_eta) {
    tuinator::MemoryTerminalBackend backend({72, 3});
    backend.init();

    auto options = tuinator::progress_bar_preset(tuinator::ProgressBarLayout::TaskRow, tuinator::Theme{}.accent,
                                                 tuinator::Theme{}.muted);
    options.stats.rate = 10.0;

    tuinator::ProgressBar bar(0.5, options);
    bar.layout({0, 0, 72, 1});

    backend.begin_frame();
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    bar.paint(ctx);
    backend.end_frame();

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "50%"));
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "Processing..."));
}

TUINATOR_TEST(progress_bar_filled_label_indeterminate_animates) {
    tuinator::MemoryTerminalBackend backend({30, 3});
    backend.init();

    auto options = tuinator::progress_bar_filled_label(tuinator::Theme{}.accent, tuinator::Theme{}.muted, "Loading...",
                                                       tuinator::ProgressBarMode::Indeterminate);
    options.min_width = 30;

    tuinator::ProgressBar bar(0.0, options);
    bar.layout({0, 0, 30, 1});
    bar.set_animation_phase(0);

    backend.begin_frame();
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    bar.paint(ctx);
    backend.end_frame();

    const tuinator::Color start_bg = backend.cells()[0][0].style.background;

    bar.set_animation_phase(22);
    backend.begin_frame();
    bar.paint(ctx);
    backend.end_frame();

    const tuinator::Color later_bg = backend.cells()[0][0].style.background;
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "Loading..."));
    TUINATOR_CHECK(start_bg != later_bg);
}

TUINATOR_TEST(progress_bar_tqdm_contains_fraction) {
    // tqdm line uses multi-byte bar glyphs; 60 columns truncates the "42/100" suffix.
    tuinator::MemoryTerminalBackend backend({80, 3});
    backend.init();

    auto options = tuinator::progress_bar_preset(tuinator::ProgressBarLayout::Tqdm, tuinator::Theme{}.label,
                                                 tuinator::Theme{}.muted);
    options.stats.total = 100;
    options.stats.current = 42;
    options.stats.rate = 9.5;

    tuinator::ProgressBar bar(0.42, options);
    bar.layout({0, 0, 80, 1});

    backend.begin_frame();
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    bar.paint(ctx);
    backend.end_frame();

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "42/100"));
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "%"));
}

TUINATOR_TEST(progress_bar_gradient_varies_across_columns) {
    tuinator::MemoryTerminalBackend backend({24, 3});
    backend.init();

    auto options = tuinator::progress_bar_preset(tuinator::ProgressBarLayout::Blocks, tuinator::Theme{}.accent,
                                                 tuinator::Theme{}.muted);
    options.gradient_stops = tuinator::progress_bar_gradient({
        {0.0f, 0xFF0000},
        {1.0f, 0x0000FF},
    });
    options.show_percent = false;
    options.min_width = 24;

    tuinator::ProgressBar bar(1.0, options);
    bar.layout({0, 0, 24, 1});

    backend.begin_frame();
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    bar.paint(ctx);
    backend.end_frame();

    const auto& left = backend.cells()[0][2].style.foreground_rgb;
    const auto& right = backend.cells()[0][20].style.foreground_rgb;
    TUINATOR_CHECK(left.has_value());
    TUINATOR_CHECK(right.has_value());
    TUINATOR_CHECK(*left != *right);
}

TUINATOR_TEST(progress_bar_bracketed_renders_caps) {
    tuinator::MemoryTerminalBackend backend({24, 3});
    backend.init();

    tuinator::ProgressBar bar(0.5, tuinator::progress_bar_preset(tuinator::ProgressBarLayout::Bracketed,
                                                                 tuinator::Theme{}.accent, tuinator::Theme{}.muted));
    bar.layout({0, 0, 24, 1});

    backend.begin_frame();
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    bar.paint(ctx);
    backend.end_frame();

    TUINATOR_CHECK_EQ(tuinator::test::cell_at(backend, 0, 0), '[');
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "%"));
}

TUINATOR_TEST(slider_changes_with_arrow_keys) {
    tuinator::Slider slider(0, 10, 0, {}, tuinator::Theme{}.label, 12);
    slider.layout({0, 0, 12, 1});
    slider.set_focused(true);

    TUINATOR_CHECK(slider.handle_event(tuinator::KeyPress{tuinator::Key::Right}));
    TUINATOR_CHECK_EQ(slider.value(), 1);
}

TUINATOR_TEST(spinner_changes_with_up_down) {
    tuinator::Spinner spinner(0, 10, 5, 1, {}, tuinator::Theme{}.label);
    spinner.layout({0, 0, 12, 1});
    spinner.set_focused(true);

    TUINATOR_CHECK(spinner.handle_event(tuinator::KeyPress{tuinator::Key::Up}));
    TUINATOR_CHECK_EQ(spinner.value(), 6);
}

TUINATOR_TEST(combo_box_selects_item) {
    tuinator::ComboBox combo(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    combo.set_items({"A", "B", "C"});
    combo.layout({0, 0, 12, 7});
    combo.set_focused(true);

    TUINATOR_CHECK(combo.handle_event(tuinator::KeyPress{tuinator::Key::Down}));
    TUINATOR_CHECK_EQ(combo.selected_index(), 1);
}

TUINATOR_TEST(tree_view_expands_node) {
    tuinator::TreeView tree(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    tree.set_root({
        "root",
        {
            {"child", {{"leaf"}}},
        },
    });
    tree.layout({0, 0, 20, 5});
    tree.set_focused(true);

    const int before = tree.selected_index();
    TUINATOR_CHECK(tree.handle_event(tuinator::KeyPress{tuinator::Key::Down}));
    TUINATOR_CHECK(tree.selected_index() > before);
}

TUINATOR_TEST(menu_bar_has_items) {
    tuinator::MenuBar menu(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    menu.set_menus({{"File", {{"Open", []() {}, "Ctrl+O"}}}});
    const tuinator::Size size = menu.preferred_size();
    TUINATOR_CHECK(size.width >= 8);
    TUINATOR_CHECK_EQ(size.height, 1);
}
