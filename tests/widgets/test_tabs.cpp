#include <tuinator/core/event.hpp>
#include <tuinator/widgets/capabilities.hpp>
#include <tuinator/widgets/containers/tabs.hpp>
#include <tuinator/widgets/display/label.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

#include <memory>

namespace {

std::unique_ptr<tuinator::Widget> tab_label(std::string text) {
    return std::make_unique<tuinator::Label>(std::move(text));
}

void setup_two_tabs(tuinator::Tabs& tabs) {
    tabs.add_tab("One", tab_label("alpha"));
    tabs.add_tab("Two", tab_label("beta"));
}

} // namespace

TUINATOR_TEST(tabs_starts_on_first_tab) {
    tuinator::Tabs tabs;
    setup_two_tabs(tabs);
    tabs.layout({0, 0, 40, 8});
    TUINATOR_CHECK_EQ(tabs.selected_index(), 0);
    TUINATOR_CHECK(tuinator::as_tab_host(&tabs) != nullptr);
    TUINATOR_CHECK_EQ(tuinator::as_tab_host(&tabs)->tab_count(), 2);
}

TUINATOR_TEST(tabs_left_right_keys_switch_when_focused) {
    tuinator::Tabs tabs;
    setup_two_tabs(tabs);
    tabs.layout({0, 0, 40, 8});
    tabs.set_focused(true);

    TUINATOR_CHECK(tabs.handle_event(tuinator::KeyPress{tuinator::Key::Right}));
    TUINATOR_CHECK_EQ(tabs.selected_index(), 1);

    TUINATOR_CHECK(tabs.handle_event(tuinator::KeyPress{tuinator::Key::Left}));
    TUINATOR_CHECK_EQ(tabs.selected_index(), 0);
}

TUINATOR_TEST(tabs_number_keys_activate_tab) {
    tuinator::Tabs tabs;
    setup_two_tabs(tabs);
    tabs.layout({0, 0, 40, 8});

    TUINATOR_CHECK(tabs.handle_event(tuinator::KeyPress{.character = '2'}));
    TUINATOR_CHECK_EQ(tabs.selected_index(), 1);
}

TUINATOR_TEST(tabs_click_on_tab_bar_switches_tab) {
    tuinator::Tabs tabs;
    setup_two_tabs(tabs);
    tabs.layout({0, 0, 40, 8});

    tuinator::MouseEvent click{};
    click.position = {tabs.bounds().x + 8, tabs.bounds().y};
    click.action = tuinator::MouseAction::Click;

    TUINATOR_CHECK(tabs.handle_event(click));
    TUINATOR_CHECK_EQ(tabs.selected_index(), 1);
}

TUINATOR_TEST(tabs_active_content_reflects_selected_tab) {
    tuinator::Tabs tabs;
    setup_two_tabs(tabs);
    tabs.layout({0, 0, 32, 6});

    tabs.set_selected_index(0);
    const tuinator::TextDisplay* first = tuinator::as_text_display(tabs.active_content());
    TUINATOR_CHECK(first != nullptr);
    TUINATOR_CHECK_EQ(first->display_text(), "alpha");

    tabs.set_selected_index(1);
    const tuinator::TextDisplay* second = tuinator::as_text_display(tabs.active_content());
    TUINATOR_CHECK(second != nullptr);
    TUINATOR_CHECK_EQ(second->display_text(), "beta");
}

TUINATOR_TEST(tabs_active_content_laid_out_below_tab_row) {
    tuinator::Tabs tabs;
    setup_two_tabs(tabs);
    tabs.layout({0, 0, 30, 10});

    const tuinator::Widget* content = tabs.active_content();
    TUINATOR_CHECK(content != nullptr);
    TUINATOR_CHECK_EQ(content->bounds().y, tabs.bounds().y + 1);
    TUINATOR_CHECK(content->bounds().height > 0);
}
