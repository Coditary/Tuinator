#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/render/nerd_glyph_catalog.hpp>
#include <tuinator/widgets/display/nerd_glyph_matrix_gallery.hpp>

#include <cstdio>

#include "render_helper.hpp"
#include "test_harness.hpp"

#ifndef TUINATOR_DATA_DIR
#define TUINATOR_DATA_DIR "data"
#endif

namespace {

std::string data_root() { return TUINATOR_DATA_DIR; }

} // namespace

TUINATOR_TEST(nerd_glyph_catalog_loads_all_exported_icons) {
    const auto sections = tuinator::load_nerd_glyph_catalog(data_root());
    TUINATOR_CHECK(!sections.empty());
    const std::size_t total = tuinator::nerd_glyph_catalog_total(sections);
    TUINATOR_CHECK(total >= 10386u);
    TUINATOR_CHECK(total <= 12000u);
}

TUINATOR_TEST(nerd_glyph_matrix_gallery_renders_domain_headers_and_glyph_rows) {
    const auto sections = tuinator::load_nerd_glyph_catalog(data_root());
    const std::size_t total = tuinator::nerd_glyph_catalog_total(sections);
    TUINATOR_CHECK(total >= 10386u);

    tuinator::NerdGlyphMatrixGallery gallery(sections, 24);
    const tuinator::Size preferred = gallery.preferred_size();
    TUINATOR_CHECK(preferred.width >= 40);
    TUINATOR_CHECK(preferred.height > 1000);

    tuinator::MemoryTerminalBackend backend({preferred.width, preferred.height});
    backend.init();
    gallery.layout({0, 0, preferred.width, preferred.height});
    tuinator::test::render_root(gallery, backend);

    char title[96];
    std::snprintf(title, sizeof(title), "Nerd Glyph Matrix (%zu glyphs, 24 cols)", total);
    TUINATOR_CHECK(tuinator::test::row_has(backend, title));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "[[ Action ]]"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "[[ MdEditor ]]"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "[[ FileLanguage ]]"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "[[ Weather ]]"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "-- Day --"));

    int glyph_rows = 0;
    for (int y = 0; y < backend.terminal_size().height; ++y) {
        int glyphs_in_row = 0;
        for (int x = 0; x < backend.terminal_size().width; ++x) {
            const char ch = tuinator::test::cell_at(backend, x, y);
            if (ch != ' ' && ch != '-' && ch != '[' && ch != ']' && ch != '(' && ch != ')') {
                ++glyphs_in_row;
            }
        }
        if (glyphs_in_row >= 8) {
            ++glyph_rows;
        }
    }
    TUINATOR_CHECK(glyph_rows >= 100);
}

TUINATOR_TEST(nerd_glyph_matrix_columns_pack_multiple_icons_per_row) {
    tuinator::NerdGlyphMatrixGallery gallery(tuinator::load_nerd_glyph_catalog(data_root()), 16);
    const tuinator::Size size = gallery.preferred_size();

    tuinator::MemoryTerminalBackend backend({size.width, 80});
    backend.init();
    gallery.layout({0, 0, size.width, 80});
    tuinator::test::render_root(gallery, backend);

    bool found_dense_row = false;
    for (int y = 0; y < backend.terminal_size().height; ++y) {
        int non_space = 0;
        for (int x = 0; x < backend.terminal_size().width; ++x) {
            if (tuinator::test::cell_at(backend, x, y) != ' ') {
                ++non_space;
            }
        }
        if (non_space >= 12) {
            found_dense_row = true;
            break;
        }
    }
    TUINATOR_CHECK(found_dense_row);
}
