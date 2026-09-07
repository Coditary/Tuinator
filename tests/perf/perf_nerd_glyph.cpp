#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/render/nerd_glyph_catalog.hpp>
#include <tuinator/widgets/display/nerd_glyph_matrix_gallery.hpp>

#include <cstdio>

#include "bench_harness.hpp"
#include "render_helper.hpp"

#ifndef TUINATOR_DATA_DIR
#define TUINATOR_DATA_DIR "data"
#endif

namespace {

std::string data_root() { return TUINATOR_DATA_DIR; }

} // namespace

TUINATOR_PERF_TEST(nerd_glyph_catalog_load) {
    return tuinator::perf::bench("nerd_glyph_catalog_load", 600.0, 1, 5,
                                 [&]() { tuinator::load_nerd_glyph_catalog(data_root()); });
}

TUINATOR_PERF_TEST(nerd_glyph_matrix_gallery_render) {
    const auto sections = tuinator::load_nerd_glyph_catalog(data_root());
    tuinator::NerdGlyphMatrixGallery gallery(sections, 24);
    tuinator::MemoryTerminalBackend backend({100, 36});
    backend.init();

    return tuinator::perf::bench("nerd_glyph_matrix_gallery_render", 500.0, 2, 20,
                                 [&]() { tuinator::test::render_root(gallery, backend); });
}

TUINATOR_PERF_TEST(nerd_glyph_utf8_conversion) {
    const auto sections = tuinator::load_nerd_glyph_catalog(data_root());
    std::vector<char32_t> codepoints;
    for (const tuinator::NerdGlyphDomainSection& section : sections) {
        for (const tuinator::NerdGlyphEntry& entry : section.entries) {
            codepoints.push_back(entry.codepoint);
        }
    }

    return tuinator::perf::bench("nerd_glyph_utf8_conversion", 120.0, 2, 15, [&]() {
        for (char32_t codepoint : codepoints) {
            tuinator::nerd_glyph_utf8(codepoint);
        }
    });
}
