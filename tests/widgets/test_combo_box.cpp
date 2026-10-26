#include <tuinator/core/event.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/controls/combo_box.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

namespace {

void setup_combo(tuinator::ComboBox& combo) {
    combo.set_items({"Alpha", "Beta", "Gamma"});
    combo.layout({0, 0, 16, 8});
    combo.set_focused(true);
}

} // namespace

TUINATOR_TEST(combo_box_closed_up_down_changes_selection) {
    tuinator::ComboBox combo(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    setup_combo(combo);
    TUINATOR_CHECK_EQ(combo.selected_index(), 0);

    TUINATOR_CHECK(combo.handle_event(tuinator::KeyPress{tuinator::Key::Down}));
    TUINATOR_CHECK_EQ(combo.selected_index(), 1);

    TUINATOR_CHECK(combo.handle_event(tuinator::KeyPress{tuinator::Key::Up}));
    TUINATOR_CHECK_EQ(combo.selected_index(), 0);
}

TUINATOR_TEST(combo_box_space_toggles_dropdown) {
    tuinator::ComboBox combo(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    setup_combo(combo);

    TUINATOR_CHECK(!combo.is_dropdown_open());
    TUINATOR_CHECK(combo.handle_event(tuinator::KeyPress{.character = ' '}));
    TUINATOR_CHECK(combo.is_dropdown_open());

    TUINATOR_CHECK(combo.handle_event(tuinator::KeyPress{.character = ' '}));
    TUINATOR_CHECK(!combo.is_dropdown_open());
}

TUINATOR_TEST(combo_box_escape_closes_open_dropdown) {
    tuinator::ComboBox combo(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    setup_combo(combo);

    TUINATOR_CHECK(combo.handle_event(tuinator::KeyPress{.character = ' '}));
    TUINATOR_CHECK(combo.is_dropdown_open());

    tuinator::KeyPress escape{};
    escape.key = tuinator::Key::Escape;
    TUINATOR_CHECK(combo.handle_event(escape));
    TUINATOR_CHECK(!combo.is_dropdown_open());
}

TUINATOR_TEST(combo_box_open_home_end_navigate_items) {
    tuinator::ComboBox combo(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    setup_combo(combo);
    TUINATOR_CHECK(combo.handle_event(tuinator::KeyPress{.character = ' '}));

    TUINATOR_CHECK(combo.handle_event(tuinator::KeyPress{tuinator::Key::End}));
    TUINATOR_CHECK_EQ(combo.selected_index(), 2);

    TUINATOR_CHECK(combo.handle_event(tuinator::KeyPress{tuinator::Key::Home}));
    TUINATOR_CHECK_EQ(combo.selected_index(), 0);
}

TUINATOR_TEST(combo_box_mouse_click_item_selects_and_closes) {
    tuinator::ComboBox combo(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    setup_combo(combo);
    TUINATOR_CHECK(combo.handle_event(tuinator::KeyPress{.character = ' '}));

    tuinator::MouseEvent click{};
    click.position = {combo.bounds().x + 2, combo.bounds().y + 3};
    click.action = tuinator::MouseAction::Click;

    TUINATOR_CHECK(combo.handle_event(click));
    TUINATOR_CHECK_EQ(combo.selected_index(), 2);
    TUINATOR_CHECK(!combo.is_dropdown_open());
}

TUINATOR_TEST(combo_box_on_select_fires_when_selection_changes) {
    tuinator::ComboBox combo(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    setup_combo(combo);

    int index = -1;
    std::string label;
    combo.set_on_select([&](int i, const std::string& item) {
        index = i;
        label = item;
    });

    TUINATOR_CHECK(combo.handle_event(tuinator::KeyPress{tuinator::Key::Down}));
    TUINATOR_CHECK_EQ(index, 1);
    TUINATOR_CHECK_EQ(label, "Beta");
}

TUINATOR_TEST(combo_box_renders_selected_item_on_closed_row) {
    tuinator::MemoryTerminalBackend backend({20, 3});
    backend.init();

    tuinator::ComboBox combo(tuinator::Theme{}.label, tuinator::Theme{}.button_focused);
    combo.set_items({"Save", "Quit"});
    combo.set_selected_index(1);
    combo.layout({0, 0, 20, 3});

    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    combo.paint(ctx);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "Quit"));
}
