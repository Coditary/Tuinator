#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/render/graphics_encode.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/display/asciidoc_view.hpp>

#include <filesystem>
#include <fstream>
#include <string>

#include "render_helper.hpp"
#include "test_harness.hpp"

namespace {

void paint_widget(tuinator::Widget& widget, tuinator::MemoryTerminalBackend& backend) {
    backend.begin_frame({.full_redraw = true, .clear_buffer = true});
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    widget.paint(ctx);
    backend.end_frame();
}

const tuinator::MemoryTerminalBackend::Cell* find_cell(const tuinator::MemoryTerminalBackend& backend, char ch) {
    for (const auto& row : backend.cells()) {
        for (const auto& cell : row) {
            if (cell.ch == ch) {
                return &cell;
            }
        }
    }
    return nullptr;
}

} // namespace

TUINATOR_TEST(asciidoc_view_renders_heading_bold) {
    tuinator::AsciiDocView view("= Hello");
    view.layout({0, 0, 40, 5});

    tuinator::MemoryTerminalBackend backend({40, 5});
    backend.init();
    paint_widget(view, backend);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "Hello"));
    const auto* cell = find_cell(backend, 'H');
    TUINATOR_CHECK(cell != nullptr);
    TUINATOR_CHECK(cell->style.bold);
}

TUINATOR_TEST(asciidoc_view_renders_constrained_bold) {
    tuinator::AsciiDocView view("a *b* c");
    view.layout({0, 0, 40, 3});

    tuinator::MemoryTerminalBackend backend({40, 3});
    backend.init();
    paint_widget(view, backend);

    const auto* bold = find_cell(backend, 'b');
    TUINATOR_CHECK(bold != nullptr);
    TUINATOR_CHECK(bold->style.bold);
}

TUINATOR_TEST(asciidoc_view_source_block_fills_row) {
    tuinator::AsciiDocView view("[source,cpp]\n----\nint x;\n----\n");
    view.layout({0, 0, 20, 4});

    tuinator::MemoryTerminalBackend backend({20, 4});
    backend.init();
    paint_widget(view, backend);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "int x;"));
    const auto& row = backend.cells()[0];
    TUINATOR_CHECK(row.back().style.background_rgb.has_value() ||
                   row.back().style.background != tuinator::Color::Default);
}

TUINATOR_TEST(asciidoc_view_streaming_matches_full_parse) {
    const std::string full =
        "= Title\n\nSome *bold* and _italic_ with `code`.\n\n"
        "[source,cpp]\n----\nint x = 1;\n----\n\n"
        "* one\n* two\n** nested\n\nNOTE: read this\n\n'''\n\n"
        "A https://example.com[link] and image:pic.png[alt].\n";

    tuinator::AsciiDocView whole(full);
    whole.layout({0, 0, 50, 40});

    tuinator::AsciiDocView streamed;
    streamed.layout({0, 0, 50, 40});
    for (const char c : full) {
        streamed.append(std::string(1, c));
    }

    tuinator::MemoryTerminalBackend backend_a({50, 40});
    backend_a.init();
    paint_widget(whole, backend_a);

    tuinator::MemoryTerminalBackend backend_b({50, 40});
    backend_b.init();
    paint_widget(streamed, backend_b);

    TUINATOR_CHECK_EQ(backend_a.snapshot(), backend_b.snapshot());
}

TUINATOR_TEST(asciidoc_view_broken_input_never_crashes) {
    const char* nasty[] = {
        "*",      "**",    "_",     "`",    "#",      "[line-through]#",  "----",
        "....",   "____",  "****",  "''",   "=",      "==x",              ".",
        "[source", "[source,", "link:", "image:", "https://", "\\", "> ",  "\xC3",
    };

    for (const char* text : nasty) {
        tuinator::AsciiDocView view;
        view.layout({0, 0, 24, 6});
        view.append(text);

        tuinator::MemoryTerminalBackend backend({24, 6});
        backend.init();
        paint_widget(view, backend);
    }
    TUINATOR_CHECK(true);
}

TUINATOR_TEST(asciidoc_view_renders_block_image) {
    const auto png = tuinator::rgba_to_png(tuinator::TerminalImage::gradient(64, 32));
    const auto path = std::filesystem::temp_directory_path() / "tuinator_adocview_test.png";
    {
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        out.write(reinterpret_cast<const char*>(png.data()), static_cast<std::streamsize>(png.size()));
    }

    tuinator::AsciiDocView view("image::" + path.string() + "[Alt]");
    view.layout({0, 0, 40, 10});

    tuinator::MemoryTerminalBackend backend({40, 10});
    backend.init();
    paint_widget(view, backend);

    TUINATOR_CHECK_EQ(backend.image_draws().size(), 1U);
    TUINATOR_CHECK_EQ(backend.image_draws()[0].cell_size.height, 2);
}
