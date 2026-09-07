#include <tuinator/layout/box.hpp>
#include <tuinator/widgets/chrome/dialog.hpp>
#include <tuinator/widgets/controls/button.hpp>
#include <tuinator/widgets/display/label.hpp>

#include <utility>

namespace tuinator::dialog {

std::unique_ptr<Widget> make_message(const Theme& theme, std::string message, std::function<void()> on_ok) {
    auto box = std::make_unique<VBox>(BoxOptions{1, 1});
    box->add_child(std::make_unique<Label>(std::move(message), theme.label));
    box->add_child(std::make_unique<Button>(
        "OK",
        [callback = std::move(on_ok)]() {
            if (callback) {
                callback();
            }
        },
        theme.button));
    return box;
}

std::unique_ptr<Widget> make_confirm(const Theme& theme, std::string message,
                                     std::function<void(bool confirmed)> on_result) {
    auto box = std::make_unique<VBox>(BoxOptions{1, 1});
    box->add_child(std::make_unique<Label>(std::move(message), theme.label));

    auto callback = std::move(on_result);

    auto actions = std::make_unique<HBox>(BoxOptions{2, 0});
    actions->add_child(std::make_unique<Button>(
        "OK",
        [callback]() {
            if (callback) {
                callback(true);
            }
        },
        theme.success));
    actions->add_child(std::make_unique<Button>(
        "Cancel",
        [callback]() {
            if (callback) {
                callback(false);
            }
        },
        theme.danger));
    box->add_child(std::move(actions));
    return box;
}

} // namespace tuinator::dialog
