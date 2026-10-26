#include <tuinator/layout/box.hpp>
#include <tuinator/testing/app_driver.hpp>
#include <tuinator/testing/cell_styles.hpp>
#include <tuinator/testing/json_protocol.hpp>
#include <tuinator/testing/widget_query.hpp>
#include <tuinator/widgets/controls/button.hpp>
#include <tuinator/widgets/controls/checkbox.hpp>
#include <tuinator/widgets/controls/text_input.hpp>
#include <tuinator/widgets/display/label.hpp>

#include "test_harness.hpp"

#include <filesystem>
#include <memory>

TUINATOR_TEST(find_by_test_id_prefers_test_id) {
    auto root = std::make_unique<tuinator::Label>("Hi");
    root->set_widget_id("stylesheet-id");
    root->set_test_id("login-title");

    TUINATOR_CHECK(tuinator::testing::find_by_selector(*root, "login-title") == root.get());
    TUINATOR_CHECK(tuinator::testing::find_by_selector(*root, "stylesheet-id") == root.get());
    TUINATOR_CHECK(tuinator::testing::find_by_selector(*root, "missing") == nullptr);
}

TUINATOR_TEST(app_driver_types_and_snapshots) {
    auto form = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    auto input = std::make_unique<tuinator::TextInput>();
    input->set_test_id("name");
    form->add_child(std::move(input));

    auto toggle = std::make_unique<tuinator::Checkbox>("Opt", false);
    toggle->set_test_id("opt");
    form->add_child(std::move(toggle));

    auto button = std::make_unique<tuinator::Button>("Go", []() {});
    button->set_test_id("go");
    form->add_child(std::move(button));

    tuinator::testing::AppDriver driver({40, 10}, std::move(form));
    driver.focus("name");
    driver.type_into("name", "Leo");
    driver.focus("opt");
    driver.press_key(tuinator::Key::None, ' ');
    driver.present();

    const std::string snapshot = driver.snapshot_text();
    TUINATOR_CHECK(snapshot.find("Leo") != std::string::npos);

    const std::string tree = driver.ui_tree_json();
    TUINATOR_CHECK(tree.find("\"id\":\"name\"") != std::string::npos);
    TUINATOR_CHECK(tree.find("\"checked\":true") != std::string::npos);

    const auto path = std::filesystem::temp_directory_path() / "tuinator_test_driver.png";
    TUINATOR_CHECK(driver.write_snapshot_png(path));
    TUINATOR_CHECK(std::filesystem::exists(path));
}

TUINATOR_TEST(region_style_detects_mismatched_row_background) {
    tuinator::MemoryTerminalBackend backend({12, 1});
    backend.init();
    backend.begin_frame();
    backend.draw_text(0, 0, "name", tuinator::style_fg_bg(tuinator::Rgb{10, 10, 10}, tuinator::Rgb{100, 100, 100}));
    backend.draw_text(8, 0, "27d", tuinator::style_fg_bg(tuinator::Rgb{10, 10, 10}, tuinator::Rgb{20, 20, 20}));
    backend.end_frame();

    const tuinator::testing::RegionStyleAnalysis analysis =
        tuinator::testing::analyze_region(backend, {0, 0, 12, 1});
    TUINATOR_CHECK(!analysis.uniform_background);
    TUINATOR_CHECK(analysis.distinct_backgrounds.size() >= 2);
    TUINATOR_CHECK(analysis.left_background.has_value());
    TUINATOR_CHECK(analysis.right_background.has_value());
    TUINATOR_CHECK(*analysis.left_background != *analysis.right_background);
}

TUINATOR_TEST(json_protocol_commands) {
    auto root = std::make_unique<tuinator::Button>("OK", []() {});
    root->set_test_id("ok");
    tuinator::testing::AppDriver driver({20, 5}, std::move(root));

    const std::string tree_response = tuinator::testing::handle_json_command(driver, R"({"cmd":"tree"})");
    TUINATOR_CHECK(tree_response.find("\"ok\":true") != std::string::npos);
    TUINATOR_CHECK(tree_response.find("\"id\":\"ok\"") != std::string::npos);

    const std::string click_response = tuinator::testing::handle_json_command(driver, R"({"cmd":"activate","id":"ok"})");
    TUINATOR_CHECK(click_response.find("\"ok\":true") != std::string::npos);
}
