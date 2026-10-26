#include <tuinator/core/event.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/controls/button.hpp>
#include <tuinator/widgets/controls/checkbox.hpp>
#include <tuinator/widgets/controls/combo_box.hpp>
#include <tuinator/widgets/controls/slider.hpp>
#include <tuinator/widgets/controls/text_area.hpp>
#include <tuinator/widgets/controls/text_input.hpp>
#include <tuinator/widgets/controls/toggle.hpp>
#include <tuinator/widgets/display/spinner.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

namespace {

const tuinator::Theme& theme() {
    static const tuinator::Theme k = tuinator::dark_theme();
    return k;
}

} // namespace

TUINATOR_TEST(edge_button_click_without_callback_succeeds) {
    tuinator::Button button("OK", nullptr);
    button.layout({0, 0, 8, 1});
    tuinator::MouseEvent click{};
    click.position = {1, 0};
    click.action = tuinator::MouseAction::Click;
    TUINATOR_CHECK(button.handle_event(click));
}

TUINATOR_TEST(edge_button_enter_ignored_when_not_focused) {
    int count = 0;
    tuinator::Button button("Go", [&] { ++count; });
    button.layout({0, 0, 8, 1});
    button.set_focused(false);
    TUINATOR_CHECK(!button.handle_event(tuinator::KeyPress{tuinator::Key::Enter}));
    TUINATOR_CHECK_EQ(count, 0);
}

TUINATOR_TEST(edge_button_zero_bounds_skips_paint) {
    tuinator::MemoryTerminalBackend backend({4, 2});
    backend.init();
    tuinator::Button button("X", nullptr);
    button.layout({0, 0, 0, 0});
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    button.paint(ctx);
    TUINATOR_CHECK(true);
}

TUINATOR_TEST(edge_toggle_space_ignored_without_focus) {
    tuinator::Toggle toggle("opt", false);
    toggle.layout({0, 0, 12, 1});
    TUINATOR_CHECK(!toggle.handle_event(tuinator::KeyPress{.character = ' '}));
    TUINATOR_CHECK(!toggle.is_checked());
}

TUINATOR_TEST(edge_checkbox_double_space_returns_to_unchecked) {
    tuinator::Checkbox box("flag", false);
    box.layout({0, 0, 14, 1});
    box.set_focused(true);
    TUINATOR_CHECK(box.handle_event(tuinator::KeyPress{.character = ' '}));
    TUINATOR_CHECK(box.checked());
    TUINATOR_CHECK(box.handle_event(tuinator::KeyPress{.character = ' '}));
    TUINATOR_CHECK(!box.checked());
}

TUINATOR_TEST(edge_combo_empty_items_accepts_index_reset) {
    tuinator::ComboBox combo(theme().label, theme().button_focused);
    combo.set_items({"a"});
    combo.set_selected_index(0);
    combo.set_items({});
    combo.layout({0, 0, 12, 4});
    combo.set_focused(true);
    TUINATOR_CHECK(!combo.handle_event(tuinator::KeyPress{tuinator::Key::Down}));
}

TUINATOR_TEST(edge_combo_escape_ignored_when_closed) {
    tuinator::ComboBox combo(theme().label, theme().button_focused);
    combo.set_items({"one", "two"});
    combo.layout({0, 0, 16, 6});
    combo.set_focused(true);
    TUINATOR_CHECK(!combo.handle_event(tuinator::KeyPress{tuinator::Key::Escape}));
}

TUINATOR_TEST(edge_slider_clamps_initial_value_to_range) {
    tuinator::Slider slider(0, 10, 50);
    TUINATOR_CHECK_EQ(slider.value(), 10);
    tuinator::Slider low(-5, 5, -99);
    TUINATOR_CHECK_EQ(low.value(), -5);
}

TUINATOR_TEST(edge_slider_arrow_at_max_stays) {
    tuinator::Slider slider(0, 3, 3);
    slider.layout({0, 0, 20, 1});
    slider.set_focused(true);
    TUINATOR_CHECK(slider.handle_event(tuinator::KeyPress{tuinator::Key::Right}));
    TUINATOR_CHECK_EQ(slider.value(), 3);
}

TUINATOR_TEST(edge_slider_min_equals_max_keeps_value) {
    tuinator::Slider slider(5, 5, 3);
    TUINATOR_CHECK_EQ(slider.value(), 5);
    slider.layout({0, 0, 12, 1});
    slider.set_focused(true);
    TUINATOR_CHECK(slider.handle_event(tuinator::KeyPress{tuinator::Key::Right}));
    TUINATOR_CHECK_EQ(slider.value(), 5);
}

TUINATOR_TEST(edge_spinner_clamps_at_min_max) {
    tuinator::Spinner spinner(0, 2, 1);
    spinner.layout({0, 0, 12, 1});
    spinner.set_focused(true);
    for (int i = 0; i < 10; ++i) {
        spinner.handle_event(tuinator::KeyPress{tuinator::Key::Right});
    }
    TUINATOR_CHECK_EQ(spinner.value(), 2);
    for (int i = 0; i < 10; ++i) {
        spinner.handle_event(tuinator::KeyPress{tuinator::Key::Left});
    }
    TUINATOR_CHECK_EQ(spinner.value(), 0);
}

TUINATOR_TEST(edge_spinner_click_left_half_decrements) {
    tuinator::Spinner spinner(0, 10, 5);
    spinner.layout({0, 0, 14, 1});
    tuinator::MouseEvent click{};
    click.position = {spinner.bounds().x + 1, spinner.bounds().y};
    click.action = tuinator::MouseAction::Click;
    TUINATOR_CHECK(spinner.handle_event(click));
    TUINATOR_CHECK_EQ(spinner.value(), 4);
}

TUINATOR_TEST(edge_text_input_rejects_control_only_character) {
    tuinator::TextInput input;
    input.layout({0, 0, 12, 1});
    input.set_focused(true);
    tuinator::KeyPress press;
    press.character = '\x01';
    press.text = "\x01";
    input.handle_event(press);
    TUINATOR_CHECK(input.value().empty());
}

TUINATOR_TEST(edge_text_input_password_masks_paint) {
    tuinator::MemoryTerminalBackend backend({14, 2});
    backend.init();
    tuinator::TextInput input(tuinator::TextInputOptions{.min_width = 8, .password = true});
    input.set_value("secret");
    input.layout({0, 0, 14, 1});
    tuinator::test::render_root(input, backend);
    TUINATOR_CHECK(!tuinator::test::row_has(backend, "secret"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "*"));
}

TUINATOR_TEST(edge_text_area_empty_preferred_size_positive) {
    tuinator::TextArea area;
    const tuinator::Size size = area.preferred_size();
    TUINATOR_CHECK(size.width > 0);
    TUINATOR_CHECK(size.height > 0);
}

TUINATOR_TEST(edge_text_area_layout_zero_height_paint_safe) {
    tuinator::MemoryTerminalBackend backend({8, 2});
    backend.init();
    tuinator::TextArea area;
    area.set_value("line");
    area.layout({0, 0, 0, 0});
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    area.paint(ctx);
    TUINATOR_CHECK(true);
}
