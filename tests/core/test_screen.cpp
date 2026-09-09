#include <tuinator/core/screen.hpp>
#include <tuinator/widgets/display/label.hpp>

#include "test_harness.hpp"

TUINATOR_TEST(screen_wants_full_screen) {
    tuinator::Screen screen;
    TUINATOR_CHECK(screen.wants_full_screen());
}

TUINATOR_TEST(make_screen_is_vbox_with_children) {
    auto root = tuinator::make_screen({.gap = 2, .padding = 1});
    root->add_child(std::make_unique<tuinator::Label>("child"));
    TUINATOR_CHECK_EQ(root->children().size(), 1U);
    TUINATOR_CHECK_EQ(root->gap(), 2);
    TUINATOR_CHECK_EQ(root->padding(), 1);
}
