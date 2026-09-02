#include "render_helper.hpp"
#include "test_harness.hpp"

#include <tuinator/layout/box.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/label.hpp>

TUINATOR_TEST(label_centers_in_vbox) {
    tuinator::MemoryTerminalBackend backend({80, 8});
    backend.init();

    const tuinator::Theme theme = tuinator::dark_theme();
    auto root = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0, .padding = 1});
    root->add_child(std::make_unique<tuinator::Label>("Hello", theme.label));

    tuinator::test::render_root(*root, backend);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 1, "Hello"));
}

TUINATOR_TEST(label_regression_no_control_chars) {
    tuinator::MemoryTerminalBackend backend({120, 10});
    backend.init();

    const tuinator::Theme theme = tuinator::dark_theme();
    auto root = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    root->add_child(std::make_unique<tuinator::Label>("Tuinator Data Explorer", theme.heading));
    root->add_child(std::make_unique<tuinator::Label>("Status line", theme.muted));

    tuinator::test::render_root(*root, backend);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 1, "Tuinator Data Explorer"));
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 3, "Status line"));

    for (int y = 1; y <= 3; y += 2) {
        for (int x = 0; x < backend.terminal_size().width; ++x) {
            const unsigned char ch =
                static_cast<unsigned char>(tuinator::test::cell_at(backend, x, y));
            TUINATOR_CHECK(ch == ' ' || (ch >= 32 && ch < 127));
        }
    }
}

TUINATOR_TEST(vbox_layout_assigns_bounds) {
    auto root = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    auto first = std::make_unique<tuinator::Label>("A", tuinator::Theme{}.label);
    auto second = std::make_unique<tuinator::Label>("B", tuinator::Theme{}.label);
    tuinator::Label* first_ptr = first.get();
    tuinator::Label* second_ptr = second.get();
    root->add_child(std::move(first));
    root->add_child(std::move(second));

    root->layout({0, 0, 40, 10});

    TUINATOR_CHECK_EQ(first_ptr->bounds().y, 1);
    TUINATOR_CHECK_EQ(second_ptr->bounds().y, 3);
    TUINATOR_CHECK_EQ(first_ptr->bounds().width, 38);
}
