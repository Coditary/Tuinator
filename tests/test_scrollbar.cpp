#include "render_helper.hpp"
#include "test_harness.hpp"

#include <tuinator/layout/box.hpp>
#include <tuinator/render/scrollbar.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/containers/panel.hpp>
#include <tuinator/widgets/containers/scroll_view.hpp>
#include <tuinator/widgets/controls/text_area.hpp>
#include <tuinator/widgets/controls/toggle.hpp>
#include <tuinator/widgets/display/label.hpp>

#include <memory>
#include <string>

TUINATOR_TEST(scrollbar_style_presets_provide_distinct_glyphs) {
    const auto ascii = tuinator::ScrollbarStyles::ascii().glyphs();
    const auto thin = tuinator::ScrollbarStyles::thin().glyphs();
    const auto minimal = tuinator::ScrollbarStyles::minimal().glyphs();

    TUINATOR_CHECK(ascii.thumb == "#");
    TUINATOR_CHECK(thin.thumb == "┃");
    TUINATOR_CHECK(minimal.vertical_track == "·");
}

TUINATOR_TEST(scrollbar_behavior_differs_from_style) {
    const auto classic_behavior = tuinator::ScrollbarBehavior::classic();
    const auto minimal_behavior = tuinator::ScrollbarBehavior::minimal();

    TUINATOR_CHECK(classic_behavior.show_arrows);
    TUINATOR_CHECK(!minimal_behavior.show_arrows);
    TUINATOR_CHECK(classic_behavior.wheel_step == 3);
}

TUINATOR_TEST(scrollbar_options_compose_config_behavior_and_style) {
    tuinator::Theme theme = tuinator::dark_theme();

    tuinator::ScrollbarGlyphPatch glyph_patch;
    glyph_patch.thumb = "#";

    tuinator::ScrollbarConfig config;
    config.horizontal = false;

    auto options = tuinator::ScrollbarOptions::from_parts(
        config,
        tuinator::ScrollbarBehavior::classic().with_wheel_step(5),
        tuinator::ScrollbarStyles::themed(theme, tuinator::ScrollbarStyles::classic())
            .with_thumb_style(tuinator::style_fg(tuinator::Rgb::hex(0x00FF00)))
            .patch_glyphs(glyph_patch));

    TUINATOR_CHECK(options.config.vertical);
    TUINATOR_CHECK(!options.config.horizontal);
    TUINATOR_CHECK_EQ(options.behavior.wheel_step, 5);
    TUINATOR_CHECK(options.style.glyphs().thumb == "#");
    TUINATOR_CHECK(options.style.palette().thumb.foreground_rgb.has_value());
    TUINATOR_CHECK_EQ(options.style.palette().thumb.foreground_rgb->g, 255);
}

TUINATOR_TEST(scrollbar_paints_vertical_track) {
    tuinator::MemoryTerminalBackend backend({12, 6});
    backend.init();

    tuinator::ScrollbarOptions options =
        tuinator::scrollbar_options(tuinator::ScrollbarPreset::Ascii);

    tuinator::ScrollbarMetrics metrics;
    metrics.show_vertical = true;
    metrics.viewport_width = 11;
    metrics.viewport_height = 6;

    backend.begin_frame();
    tuinator::Canvas canvas(backend);
    tuinator::paint_scrollbars(canvas, options, 0, 5, 11, 20);
    backend.end_frame();

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "^"));
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 5, "v"));
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 2, "#"));
}

TUINATOR_TEST(canvas_draw_text_clips_horizontally_scrolled_content) {
    tuinator::MemoryTerminalBackend backend({20, 3});
    backend.init();

    backend.begin_frame();
    tuinator::Canvas canvas(backend);
    canvas.with_clip({{5, 0}, {10, 3}}, [&](tuinator::Canvas& clipped) {
        tuinator::Canvas scrolled = clipped.scrolled(-3, 0);
        scrolled.draw_text({0, 1}, "Hello horizontal");
    });
    backend.end_frame();

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 1, "lo hor"));
    TUINATOR_CHECK(!tuinator::test::row_contains(backend, 1, "Hello"));
}

TUINATOR_TEST(scroll_view_horizontal_scroll_keeps_text_visible) {
    tuinator::MemoryTerminalBackend backend({20, 4});
    backend.init();

    auto content = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    content->add_child(std::make_unique<tuinator::Label>("alpha beta gamma delta tail"));

    tuinator::ScrollViewOptions options;
    options.width = 12;
    options.height = 3;
    options.scrollbars = tuinator::scrollbar_options(tuinator::ScrollbarPreset::Ascii);

    tuinator::ScrollView scroll(std::move(content), options);
    scroll.layout({0, 0, 12, 3});
    scroll.scroll_to(6, 0);
    tuinator::test::render_root(scroll, backend);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "gamma"));
    TUINATOR_CHECK(!tuinator::test::row_contains(backend, 0, "alpha"));
}

TUINATOR_TEST(hbox_forwards_wheel_events_to_child_under_cursor) {
    tuinator::MemoryTerminalBackend backend({40, 8});
    backend.init();

    auto list = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    for (int i = 1; i <= 12; ++i) {
        auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});
        row->add_child(std::make_unique<tuinator::Label>("Item " + std::to_string(i)));
        row->add_child(std::make_unique<tuinator::Toggle>("Enable", false, [](bool) {}));
        list->add_child(std::move(row));
    }

    auto scroll = std::make_unique<tuinator::ScrollView>(
        std::move(list),
        tuinator::ScrollViewOptions{.width = 20, .height = 5});
    auto* scroll_ptr = scroll.get();

    auto panels = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});
    panels->add_child(std::move(scroll));
    panels->add_child(std::make_unique<tuinator::Label>("spacer", tuinator::Style{}));
    panels->layout({0, 0, 40, 8});
    scroll_ptr->layout({0, 0, 20, 5});

    TUINATOR_CHECK_EQ(scroll_ptr->scroll_y(), 0);

    tuinator::MouseEvent wheel{};
    wheel.action = tuinator::MouseAction::WheelDown;
    wheel.position = {5, 2};
    TUINATOR_CHECK(panels->handle_event(wheel));
    TUINATOR_CHECK(scroll_ptr->scroll_y() > 0);
}

TUINATOR_TEST(scroll_view_mouse_click_on_row_label_toggles) {
    tuinator::MemoryTerminalBackend backend({30, 8});
    backend.init();

    auto list = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    for (int i = 1; i <= 10; ++i) {
        auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});
        row->add_child(std::make_unique<tuinator::Label>("Item " + std::to_string(i)));
        row->add_child(std::make_unique<tuinator::Toggle>("Enable", false, [](bool) {}));
        list->add_child(std::move(row));
    }

    tuinator::ScrollView scroll(std::move(list), tuinator::ScrollViewOptions{.width = 24, .height = 5});
    scroll.layout({5, 3, 24, 5});

    tuinator::Widget* hit = scroll.hit_test_focusable({8, 5});
    TUINATOR_CHECK(hit != nullptr);

    tuinator::MouseEvent click{};
    click.action = tuinator::MouseAction::Click;
    click.position = {8, 5};
    TUINATOR_CHECK(scroll.handle_event(click));
}

TUINATOR_TEST(scroll_view_row_updates_cleanly_when_scrolling) {
    tuinator::MemoryTerminalBackend backend({30, 6});
    backend.init();

    auto list = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    for (int i = 1; i <= 40; ++i) {
        auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});
        row->add_child(std::make_unique<tuinator::Label>("Item " + std::to_string(i)));
        row->add_child(std::make_unique<tuinator::Toggle>("Enable", false, [](bool) {}));
        list->add_child(std::move(row));
    }

    tuinator::ScrollViewOptions options;
    options.width = 30;
    options.height = 6;
    options.scrollbars = tuinator::scrollbar_options(tuinator::ScrollbarPreset::Ascii);

    tuinator::ScrollView scroll(std::move(list), options);
    scroll.layout({0, 0, 30, 6});

    scroll.scroll_to(0, 0);
    tuinator::test::render_root(scroll, backend);
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 2, "Item 3"));
    TUINATOR_CHECK(!tuinator::test::row_contains(backend, 2, "Item 30"));

    scroll.scroll_to(0, 27);
    tuinator::test::render_root(scroll, backend);
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 2, "Item 30"));
    // "Item 3" is a prefix of "Item 30" — match the label boundary.
    TUINATOR_CHECK(!tuinator::test::row_contains(backend, 2, "Item 3 "));
}

TUINATOR_TEST(scroll_view_paints_scrollbars_for_tall_content) {
    tuinator::MemoryTerminalBackend backend({10, 5});
    backend.init();

    auto content = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    for (int i = 0; i < 8; ++i) {
        content->add_child(std::make_unique<tuinator::Label>("row " + std::to_string(i)));
    }

    tuinator::ScrollViewOptions options;
    options.width = 10;
    options.height = 5;
    options.scrollbars = tuinator::scrollbar_options(tuinator::ScrollbarPreset::Ascii);

    tuinator::ScrollView scroll(std::move(content), options);
    scroll.layout({0, 0, 10, 5});
    tuinator::test::render_root(scroll, backend);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "^"));
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 4, "v"));
}

TUINATOR_TEST(scroll_view_maps_content_dirty_to_screen) {
    tuinator::Rect reported{};
    auto content = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    for (int i = 0; i < 20; ++i) {
        content->add_child(std::make_unique<tuinator::Label>("row", tuinator::Style{}));
    }

    tuinator::ScrollViewOptions options;
    options.width = 20;
    options.height = 6;

    tuinator::ScrollView scroll(std::move(content), options);
    scroll.layout({5, 3, 20, 6});
    scroll.set_on_dirty([&](tuinator::Rect region) { reported = region; });

    scroll.scroll_to(0, 8);
    scroll.content()->children()[10]->mark_dirty();

    TUINATOR_CHECK_EQ(reported.x, 5);
    TUINATOR_CHECK_EQ(reported.y, 5);
}

TUINATOR_TEST(panel_forwards_dirty_callback_to_content) {
    tuinator::Rect reported{};
    tuinator::Panel panel("Weekly", tuinator::Style{});
    auto label = std::make_unique<tuinator::Label>("cell", tuinator::Style{});
    tuinator::Label* label_ptr = label.get();
    panel.set_content(std::move(label));
    panel.layout({0, 0, 24, 6});
    panel.set_on_dirty([&](tuinator::Rect region) { reported = region; });

    label_ptr->mark_dirty();

    TUINATOR_CHECK_EQ(reported.x, 1);
    TUINATOR_CHECK_EQ(reported.y, 2);
}

TUINATOR_TEST(text_area_enter_accepts_newline_character) {
    tuinator::TextArea area;
    area.set_focused(true);
    area.layout({0, 0, 40, 10});

    tuinator::KeyPress newline{};
    newline.key = tuinator::Key::Unknown;
    newline.character = '\n';
    area.handle_event(newline);

    TUINATOR_CHECK_EQ(area.value(), std::string("\n"));
}
