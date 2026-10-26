#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/capabilities.hpp>
#include <tuinator/widgets/chrome/dialog.hpp>
#include <tuinator/widgets/controls/button.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

#include <vector>

TUINATOR_TEST(dialog_message_renders_text_and_ok) {
    tuinator::MemoryTerminalBackend backend({32, 8});
    backend.init();

    auto dialog = tuinator::dialog::make_message(tuinator::dark_theme(), "Saved successfully.");
    dialog->layout({0, 0, 32, 8});
    tuinator::test::render_root(*dialog, backend);

    bool found_message = false;
    bool found_ok = false;
    for (int y = 0; y < backend.terminal_size().height; ++y) {
        if (tuinator::test::row_contains(backend, y, "Saved successfully")) {
            found_message = true;
        }
        if (tuinator::test::row_contains(backend, y, "[OK]")) {
            found_ok = true;
        }
    }
    TUINATOR_CHECK(found_message);
    TUINATOR_CHECK(found_ok);
}

TUINATOR_TEST(dialog_message_ok_button_fires_callback) {
    bool ok = false;
    auto dialog = tuinator::dialog::make_message(tuinator::dark_theme(), "Hi", [&ok]() { ok = true; });
    dialog->layout({0, 0, 24, 6});

    tuinator::Widget* button = nullptr;
    dialog->for_each_descendant([&](tuinator::Widget* widget) {
        if (widget != nullptr && widget->widget_type_name() == "Button") {
            button = widget;
        }
    });
    TUINATOR_CHECK(button != nullptr);
    tuinator::as_activatable(button)->activate();
    TUINATOR_CHECK(ok);
}

TUINATOR_TEST(dialog_confirm_cancel_reports_false) {
    bool answered = false;
    bool confirmed = true;
    auto dialog = tuinator::dialog::make_confirm(tuinator::dark_theme(), "Delete file?",
                                                 [&](bool choice) {
                                                     answered = true;
                                                     confirmed = choice;
                                                 });
    dialog->layout({0, 0, 40, 10});

    std::vector<tuinator::Button*> buttons;
    dialog->for_each_descendant([&](tuinator::Widget* widget) {
        if (auto* button = dynamic_cast<tuinator::Button*>(widget)) {
            buttons.push_back(button);
        }
    });
    TUINATOR_CHECK(buttons.size() >= 2u);
    tuinator::as_activatable(buttons[1])->activate();
    TUINATOR_CHECK(answered);
    TUINATOR_CHECK(!confirmed);
}
