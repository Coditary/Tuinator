#include "render_helper.hpp"
#include "test_harness.hpp"

#include <tuinator/widgets/display/big_text.hpp>

TUINATOR_TEST(big_text_block_has_area) {
    tuinator::BigText text("A", tuinator::BigTextKind::Block);
    const tuinator::Size size = text.preferred_size();
    TUINATOR_CHECK(size.width >= 7);
    TUINATOR_CHECK(size.height >= 7);
}

TUINATOR_TEST(big_text_named_quadrant) {
    TUINATOR_CHECK(tuinator::big_text_kind_named("quad") == tuinator::BigTextKind::Quadrant);
    TUINATOR_CHECK(tuinator::big_text_style_named("slant") != nullptr);
}

TUINATOR_TEST(big_text_paints_block_glyph) {
    tuinator::MemoryTerminalBackend backend({20, 10});
    backend.init();

    tuinator::BigText text("I", tuinator::BigTextKind::Block);
    tuinator::test::render_root(text, backend);

    bool found_block = false;
    for (int y = 0; y < backend.terminal_size().height; ++y) {
        for (int x = 0; x < backend.terminal_size().width; ++x) {
            if (static_cast<unsigned char>(tuinator::test::cell_at(backend, x, y)) != ' ') {
                found_block = true;
            }
        }
    }
    TUINATOR_CHECK(found_block);
}

TUINATOR_TEST(big_text_slant_contains_ascii_art) {
    tuinator::MemoryTerminalBackend backend({40, 10});
    backend.init();

    tuinator::BigText text("A", tuinator::BigTextKind::Slant);
    tuinator::test::render_root(text, backend);

    bool found_slash = false;
    for (int y = 0; y < backend.terminal_size().height; ++y) {
        for (int x = 0; x < backend.terminal_size().width; ++x) {
            if (tuinator::test::cell_at(backend, x, y) == '/') {
                found_slash = true;
            }
        }
    }
    TUINATOR_CHECK(found_slash);
}

TUINATOR_TEST(big_text_catalog_is_populated) {
    TUINATOR_CHECK(tuinator::all_big_text_styles().size() >= 16);
}

TUINATOR_TEST(big_text_shadow_uses_separate_style) {
    tuinator::MemoryTerminalBackend backend({20, 12});
    backend.init();

    tuinator::Style fill;
    fill.foreground = tuinator::Color::Magenta;
    tuinator::Style shadow;
    shadow.foreground = tuinator::Color::Cyan;

    tuinator::BigTextOptions options;
    options.kind = tuinator::BigTextKind::Stacked;
    options.shadow_style = shadow;

    tuinator::BigText text("A", options, fill);
    tuinator::test::render_root(text, backend);

    bool found_fill = false;
    bool found_shadow = false;
    for (const auto& row : backend.cells()) {
        for (const auto& cell : row) {
            if (cell.ch == ' ') {
                continue;
            }
            if (cell.style.foreground == tuinator::Color::Magenta && !cell.style.dim) {
                found_fill = true;
            }
            if (cell.style.foreground == tuinator::Color::Cyan) {
                found_shadow = true;
            }
        }
    }
    TUINATOR_CHECK(found_fill);
    TUINATOR_CHECK(found_shadow);
}

TUINATOR_TEST(big_text_vertical_gradient_varies_by_row) {
    tuinator::MemoryTerminalBackend backend({20, 16});
    backend.init();

    tuinator::BigTextOptions options;
    options.kind = tuinator::BigTextKind::Block;
    options.gradient_stops = tuinator::big_text_gradient({
        {0.0f, 0xFF0000},
        {1.0f, 0x0000FF},
    });

    tuinator::BigText text("A", options);
    tuinator::test::render_root(text, backend);

    std::optional<tuinator::Rgb> top_rgb;
    std::optional<tuinator::Rgb> bottom_rgb;
    for (int y = 0; y < backend.terminal_size().height; ++y) {
        for (int x = 0; x < backend.terminal_size().width; ++x) {
            const auto& cell = backend.cells()[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
            if (cell.ch == ' ' || !cell.style.foreground_rgb) {
                continue;
            }
            if (!top_rgb) {
                top_rgb = *cell.style.foreground_rgb;
            }
            bottom_rgb = *cell.style.foreground_rgb;
        }
    }

    TUINATOR_CHECK(top_rgb.has_value());
    TUINATOR_CHECK(bottom_rgb.has_value());
    TUINATOR_CHECK(top_rgb->r > bottom_rgb->r);
    TUINATOR_CHECK(top_rgb->b < bottom_rgb->b);
}

TUINATOR_TEST(big_text_look_catalog_has_omarchy) {
    TUINATOR_CHECK(tuinator::big_text_look_named("omarchy") != nullptr);
    TUINATOR_CHECK(tuinator::all_big_text_looks().size() >= 5);
}
