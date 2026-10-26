#include <tuinator/layout/box.hpp>
#include <tuinator/testing/app_driver.hpp>
#include <tuinator/testing/json_protocol.hpp>
#include <tuinator/widgets/controls/button.hpp>
#include <tuinator/widgets/controls/checkbox.hpp>
#include <tuinator/widgets/controls/text_input.hpp>
#include <tuinator/widgets/display/label.hpp>
#include <tuinator/widgets/display/separator.hpp>

#include <memory>
#include <string>

namespace {

std::unique_ptr<tuinator::Widget> build_demo_ui() {
    auto form = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    form->set_test_id("root");

    form->add_child(std::make_unique<tuinator::Label>("Login"));

    auto username = std::make_unique<tuinator::TextInput>(
        tuinator::TextInputOptions{.min_width = 24, .placeholder = "guest"});
    username->set_test_id("username");
    auto* username_ptr = username.get();
    form->add_child(std::move(username));

    auto remember = std::make_unique<tuinator::Checkbox>("Remember me", false);
    remember->set_test_id("remember");
    form->add_child(std::move(remember));

    auto preview = std::make_unique<tuinator::Label>("Signed in as: guest");
    preview->set_test_id("preview");
    auto* preview_ptr = preview.get();
    form->add_child(std::move(preview));

    username_ptr->set_on_change([preview_ptr](const std::string& value) {
        preview_ptr->set_text("Signed in as: " + (value.empty() ? std::string("guest") : value));
    });

    form->add_child(std::make_unique<tuinator::Separator>());

    auto submit = std::make_unique<tuinator::Button>("Sign in", []() {});
    submit->set_test_id("submit");
    form->add_child(std::move(submit));

    return form;
}

} // namespace

int main() {
    tuinator::testing::AppDriver driver({60, 18}, build_demo_ui());
    return tuinator::testing::run_json_protocol_loop(driver);
}
