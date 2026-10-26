#include <tuinator/core/event.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/containers/tabs.hpp>
#include <tuinator/widgets/controls/button.hpp>
#include <tuinator/widgets/controls/combo_box.hpp>
#include <tuinator/widgets/controls/slider.hpp>
#include <tuinator/widgets/controls/toggle.hpp>
#include <tuinator/widgets/display/label.hpp>
#include <tuinator/widgets/display/spinner.hpp>
#include <tuinator/widgets/views/list_view.hpp>
#include <tuinator/widgets/views/table.hpp>

#include "negative_test.hpp"
#include "param_test.hpp"
#include "test_harness.hpp"

namespace {

const tuinator::Theme& theme() {
    static const tuinator::Theme k = tuinator::dark_theme();
    return k;
}

struct UnfocusedKeyCase {
    const char* label;
    tuinator::Key key;
};

constexpr UnfocusedKeyCase kListUnfocusedKeys[] = {
    {"down", tuinator::Key::Down},
    {"up", tuinator::Key::Up},
    {"home", tuinator::Key::Home},
    {"end", tuinator::Key::End},
};

} // namespace

TUINATOR_TEST(negative_list_view_unfocused_keys_do_not_change_selection) {
    TUINATOR_PARAM_TABLE(list_unfocused, UnfocusedKeyCase, kListUnfocusedKeys, {
        tuinator::ListView list(theme().label, theme().button_focused);
        list.set_items({"a", "b", "c"});
        list.set_selected_index(1);
        list.layout({0, 0, 20, 5});
        list.set_focused(false);
        TUINATOR_CHECK_FALSE(list.handle_event(tuinator::KeyPress{_tuinator_param_row.key}));
        TUINATOR_CHECK_EQ(list.selected_index(), 1);
    });
}

TUINATOR_TEST(negative_list_view_empty_rejects_navigation) {
    tuinator::ListView list(theme().label, theme().button_focused);
    list.layout({0, 0, 12, 4});
    list.set_focused(true);
    TUINATOR_CHECK_FALSE(list.handle_event(tuinator::KeyPress{tuinator::Key::Down}));
    TUINATOR_CHECK_FALSE(list.handle_event(tuinator::KeyPress{tuinator::Key::Enter}));
}

TUINATOR_TEST(negative_button_outside_click_does_not_invoke_callback) {
    int activations = 0;
    tuinator::Button button("Hit", [&] { ++activations; });
    button.layout({4, 2, 8, 1});

    tuinator::MouseEvent miss{};
    miss.position = {0, 0};
    miss.action = tuinator::MouseAction::Click;
    TUINATOR_CHECK_FALSE(button.handle_event(miss));
    TUINATOR_CHECK_EQ(activations, 0);
}

TUINATOR_TEST(negative_toggle_unfocused_space_ignored) {
    tuinator::Toggle toggle("opt", true);
    toggle.layout({0, 0, 12, 1});
    toggle.set_focused(false);
    TUINATOR_CHECK_FALSE(toggle.handle_event(tuinator::KeyPress{.character = ' '}));
    TUINATOR_CHECK(toggle.is_checked());
}

TUINATOR_TEST(negative_combo_empty_rejects_arrow_keys) {
    tuinator::ComboBox combo(theme().label, theme().button_focused);
    combo.set_items({});
    combo.layout({0, 0, 14, 4});
    combo.set_focused(true);
    TUINATOR_CHECK_FALSE(combo.handle_event(tuinator::KeyPress{tuinator::Key::Down}));
    TUINATOR_CHECK_FALSE(combo.handle_event(tuinator::KeyPress{tuinator::Key::Up}));
}

TUINATOR_TEST(negative_slider_unfocused_arrow_ignored) {
    tuinator::Slider slider(0, 10, 4);
    slider.layout({0, 0, 20, 1});
    slider.set_focused(false);
    TUINATOR_CHECK_FALSE(slider.handle_event(tuinator::KeyPress{tuinator::Key::Right}));
    TUINATOR_CHECK_EQ(slider.value(), 4);
}

TUINATOR_TEST(negative_spinner_unfocused_does_not_step) {
    tuinator::Spinner spinner(0, 10, 5);
    spinner.layout({0, 0, 14, 1});
    spinner.set_focused(false);
    TUINATOR_CHECK_FALSE(spinner.handle_event(tuinator::KeyPress{tuinator::Key::Right}));
    TUINATOR_CHECK_EQ(spinner.value(), 5);
}

TUINATOR_TEST(negative_table_unfocused_arrows_ignored) {
    tuinator::Table table;
    table.set_columns({{"n", 4}});
    table.set_rows({{"a"}, {"b"}});
    table.set_selected_row(0);
    table.layout({0, 0, 10, 6});
    table.set_focused(false);
    TUINATOR_CHECK_FALSE(table.handle_event(tuinator::KeyPress{tuinator::Key::Down}));
    TUINATOR_CHECK_EQ(table.selected_row(), 0);
}

TUINATOR_TEST(negative_tabs_unfocused_arrow_does_not_switch) {
    tuinator::Tabs tabs;
    tabs.add_tab("A", std::make_unique<tuinator::Label>("a"));
    tabs.add_tab("B", std::make_unique<tuinator::Label>("b"));
    tabs.layout({0, 0, 24, 6});
    tabs.set_focused(false);
    TUINATOR_CHECK_FALSE(tabs.handle_event(tuinator::KeyPress{tuinator::Key::Right}));
    TUINATOR_CHECK_EQ(tabs.selected_index(), 0);
}

TUINATOR_TEST(negative_clamp_does_not_equal_raw_out_of_range_index) {
    tuinator::ListView list(theme().label, theme().button_focused);
    list.set_items({"only"});
    list.set_selected_index(50);
    TUINATOR_CHECK_FALSE(list.selected_index() == 50);
    TUINATOR_CHECK_EQ(list.selected_index(), 0);
}

/// Proves the negative list test would fail if selection changed without focus (mutation guard).
TUINATOR_TEST(negative_meta_list_unfocused_must_not_move) {
    TUINATOR_EXPECT_CHECK_FAILS({
        tuinator::ListView list(theme().label, theme().button_focused);
        list.set_items({"a", "b"});
        list.set_selected_index(0);
        list.set_focused(false);
        list.set_selected_index(1);
        TUINATOR_CHECK_EQ(list.selected_index(), 0);
    });
}

/// Proves clamp meta: expecting raw 99 on a 1-item list must fail inside capture.
TUINATOR_TEST(negative_meta_clamp_must_not_equal_raw_index) {
    TUINATOR_EXPECT_CHECK_FAILS({
        tuinator::ListView list(theme().label, theme().button_focused);
        list.set_items({"x"});
        list.set_selected_index(99);
        TUINATOR_CHECK_EQ(list.selected_index(), 99);
    });
}
