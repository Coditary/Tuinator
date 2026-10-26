#include <tuinator/layout/box.hpp>
#include <tuinator/testing/app_driver.hpp>
#include <tuinator/testing/json_protocol.hpp>
#include <tuinator/widgets/controls/text_input.hpp>
#include <tuinator/widgets/display/label.hpp>

#include "test_harness.hpp"

#include <memory>

TUINATOR_TEST(json_protocol_hover_and_styles) {
    auto root = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    auto label = std::make_unique<tuinator::Label>("hover me");
    label->set_test_id("target");
    root->add_child(std::move(label));

    tuinator::testing::AppDriver driver({32, 6}, std::move(root));
    const std::string hover = tuinator::testing::handle_json_command(driver, R"({"cmd":"hover","id":"target"})");
    TUINATOR_CHECK(hover.find("\"ok\":true") != std::string::npos);

    const std::string styles = tuinator::testing::handle_json_command(driver, R"({"cmd":"styles","id":"target"})");
    TUINATOR_CHECK(styles.find("\"ok\":true") != std::string::npos);
    TUINATOR_CHECK(styles.find("uniform_background") != std::string::npos);
}

TUINATOR_TEST(json_protocol_type_and_missing_id_errors) {
    auto field = std::make_unique<tuinator::TextInput>();
    field->set_test_id("name");
    tuinator::testing::AppDriver driver({30, 5}, std::move(field));

    const std::string typed =
        tuinator::testing::handle_json_command(driver, R"({"cmd":"type","id":"name","text":"ai"})");
    TUINATOR_CHECK(typed.find("\"ok\":true") != std::string::npos);
    TUINATOR_CHECK(driver.snapshot_text().find("ai") != std::string::npos);

    const std::string missing = tuinator::testing::handle_json_command(driver, R"({"cmd":"click","id":"nope"})");
    TUINATOR_CHECK(missing.find("\"ok\":false") != std::string::npos);
}

TUINATOR_TEST(json_protocol_snapshot_and_step) {
    auto label = std::make_unique<tuinator::Label>("snap");
    tuinator::testing::AppDriver driver({20, 4}, std::move(label));

    const std::string snap = tuinator::testing::handle_json_command(driver, R"({"cmd":"snapshot"})");
    TUINATOR_CHECK(snap.find("snap") != std::string::npos);

    const std::string step = tuinator::testing::handle_json_command(driver, R"({"cmd":"step"})");
    TUINATOR_CHECK(step.find("\"ok\":true") != std::string::npos);
}
