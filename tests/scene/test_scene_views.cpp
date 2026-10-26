#include <tuinator/core/application.hpp>
#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/scene/scene.hpp>
#include <tuinator/testing/app_driver.hpp>
#include <tuinator/widgets/views/list_view.hpp>
#include <tuinator/widgets/views/table.hpp>

#include "component_expect.hpp"
#include "test_harness.hpp"

#ifndef TUINATOR_DATA_DIR
#define TUINATOR_DATA_DIR "data"
#endif

namespace {

tuinator::scene::BuildResult load_scene(const char* json) {
    auto backend = std::make_unique<tuinator::MemoryTerminalBackend>(tuinator::Size{80, 24});
    backend->init();
    tuinator::Application app(std::move(backend));
    return tuinator::scene::load_and_build_from_string(json, app, tuinator::dark_theme(), {});
}

} // namespace

TUINATOR_TEST(scene_builds_list_view_with_selection) {
    static constexpr char kScene[] = R"json({
        "version": 1,
        "root": {
            "type": "ListView",
            "id": "files",
            "items": ["main.cpp", "util.hpp"],
            "selectedIndex": 1
        }
    })json";

    tuinator::scene::BuildResult built = load_scene(kScene);
    TUINATOR_CHECK(built.root != nullptr);
    TUINATOR_CHECK(built.widgets.count("files") == 1);

    auto* list = dynamic_cast<tuinator::ListView*>(built.widgets.at("files").ptr);
    TUINATOR_CHECK(list != nullptr);
    TUINATOR_CHECK_EQ(list->selected_index(), 1);

    tuinator::testing::AppDriver driver({40, 8}, std::move(built.root));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "util.hpp"));
}

TUINATOR_TEST(scene_builds_table_with_columns) {
    static constexpr char kScene[] = R"json({
        "version": 1,
        "root": {
            "type": "Table",
            "id": "grid",
            "columns": [
                { "title": "Key", "width": 8 },
                { "title": "Value", "width": 6 }
            ],
            "rows": [
                ["host", "localhost"],
                ["port", "8080"]
            ]
        }
    })json";

    tuinator::scene::BuildResult built = load_scene(kScene);
    auto* table = dynamic_cast<tuinator::Table*>(built.widgets.at("grid").ptr);
    TUINATOR_CHECK(table != nullptr);
    TUINATOR_CHECK_EQ(table->columns().size(), 2u);
    TUINATOR_CHECK_EQ(table->rows().size(), 2u);

    tuinator::testing::AppDriver driver({48, 12}, std::move(built.root));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "host"));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "8080"));
}

TUINATOR_TEST(scene_builds_nerd_glyph_matrix_gallery) {
    const std::string json = std::string(R"({"version":1,"root":{"type":"NerdGlyphMatrixGallery","id":"glyphs","columns":16,"dataRoot":")") +
                                         std::string(TUINATOR_DATA_DIR) + R"("}})";

    tuinator::scene::BuildResult built = load_scene(json.c_str());
    TUINATOR_CHECK(built.root != nullptr);
    TUINATOR_CHECK(built.widgets.count("glyphs") == 1);

    tuinator::testing::AppDriver driver({64, 24}, std::move(built.root));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Nerd Glyph Matrix"));
}

TUINATOR_TEST(scene_builds_nerd_icon_catalog_gallery) {
    static constexpr char kScene[] = R"json({
        "version": 1,
        "root": {
            "type": "NerdIconCatalogGallery",
            "id": "icons"
        }
    })json";

    tuinator::scene::BuildResult built = load_scene(kScene);
    TUINATOR_CHECK(built.widgets.count("icons") == 1);

    tuinator::testing::AppDriver driver({80, 20}, std::move(built.root));
    TUINATOR_CHECK(tuinator::test::expect::snapshot_contains(driver, "Nerd Icon Catalog"));
}
