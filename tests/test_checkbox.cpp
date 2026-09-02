#include "render_helper.hpp"
#include "test_harness.hpp"

#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/checkbox.hpp>
#include <tuinator/widgets/panel.hpp>

TUINATOR_TEST(checkbox_named_style_exists) {
    TUINATOR_CHECK(tuinator::checkbox_style_named("ascii") != nullptr);
    TUINATOR_CHECK(tuinator::checkbox_style_named("checkmark") != nullptr);
    TUINATOR_CHECK(tuinator::all_checkbox_styles().size() >= 8);
}

TUINATOR_TEST(checkbox_renders_custom_glyphs) {
    tuinator::MemoryTerminalBackend backend({24, 3});
    backend.init();

    tuinator::CheckboxOptions options;
    options.glyphs = {"[ ]", "[X]", " "};
    options.label_style.foreground = tuinator::Color::White;

    tuinator::Checkbox checkbox("Test", true, options);
    tuinator::test::render_root(checkbox, backend);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "[X]"));
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "Test"));
}

TUINATOR_TEST(checkbox_toggles_with_space) {
    tuinator::MemoryTerminalBackend backend({24, 3});
    backend.init();

    bool changed = false;
    bool value = false;
    tuinator::Checkbox checkbox(
        "Toggle me",
        false,
        tuinator::CheckboxOptions{},
        [&](bool checked) {
            changed = true;
            value = checked;
        });
    checkbox.layout({0, 0, 24, 1});
    checkbox.set_focused(true);

    tuinator::KeyPress press;
    press.key = tuinator::Key::None;
    press.character = ' ';
    checkbox.handle_event(press);

    TUINATOR_CHECK(changed);
    TUINATOR_CHECK(value);
    TUINATOR_CHECK(checkbox.checked());
}

TUINATOR_TEST(checkbox_style_sets_label_color) {
    tuinator::MemoryTerminalBackend backend({24, 3});
    backend.init();

    const tuinator::Theme theme = tuinator::dark_theme();
    tuinator::Checkbox checkbox("Info", false, "info", {}, theme);
    tuinator::test::render_root(checkbox, backend);

    bool found_cyan = false;
    for (const auto& cell : backend.cells()[0]) {
        if (cell.ch != ' ' && cell.style.foreground == tuinator::Color::Cyan) {
            found_cyan = true;
        }
    }
    TUINATOR_CHECK(found_cyan);
}

TUINATOR_TEST(checkbox_apply_style_copies_glyphs) {
    tuinator::CheckboxOptions options = tuinator::checkbox_options_default(tuinator::dark_theme());
    const tuinator::CheckboxStyle* style = tuinator::checkbox_style_named("diamond");
    TUINATOR_CHECK(style != nullptr);
    tuinator::apply_checkbox_style(options, *style, tuinator::dark_theme());
    TUINATOR_CHECK(options.glyphs.checked == "◆");
    TUINATOR_CHECK(options.glyphs.unchecked == "◇");
}

TUINATOR_TEST(panel_hit_test_focusable_finds_nested_checkbox) {
    const tuinator::Theme theme = tuinator::dark_theme();
    auto checkbox = std::make_unique<tuinator::Checkbox>(
        "Inside",
        false,
        "basic",
        std::function<void(bool)>{},
        theme);
    tuinator::Checkbox* checkbox_ptr = checkbox.get();

    auto panel = std::make_unique<tuinator::Panel>("Box", theme.border, theme.muted);
    panel->set_content(std::move(checkbox));
    panel->layout({0, 0, 30, 5});
    checkbox_ptr->layout({1, 2, 20, 1});

    tuinator::Widget* hit = panel->hit_test_focusable({2, 2});
    TUINATOR_CHECK(hit == checkbox_ptr);
}

TUINATOR_TEST(checkbox_focus_highlight_matches_text_width) {
    tuinator::MemoryTerminalBackend backend({30, 3});
    backend.init();

    tuinator::Checkbox checkbox("Short", true, "ascii");
    checkbox.layout({0, 0, 30, 1});
    checkbox.set_focused(true);

    tuinator::test::render_root(checkbox, backend);

    int last_highlight = -1;
    for (int x = 0; x < backend.terminal_size().width; ++x) {
        const auto& cell = backend.cells()[0][static_cast<std::size_t>(x)];
        if (cell.style.background_rgb.has_value()) {
            last_highlight = x;
        }
    }

    TUINATOR_CHECK(last_highlight >= 0);
    TUINATOR_CHECK(last_highlight < 12);
}
