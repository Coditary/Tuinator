#include "backend/ansi_backend.hpp"

#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/render/graphics_encode.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/display/markdown_view.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <thread>

#include "render_helper.hpp"
#include "test_harness.hpp"

namespace {

std::string write_test_png_file(const char* filename, int width, int height) {
    const auto png = tuinator::rgba_to_png(tuinator::TerminalImage::gradient(width, height));
    const auto path = std::filesystem::temp_directory_path() / filename;
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char*>(png.data()), static_cast<std::streamsize>(png.size()));
    return path.string();
}

std::string write_test_png() { return write_test_png_file("tuinator_mdview_test.png", 64, 32); }

/// Minimal 2x2 two-frame GIF (9-bit literal LZW codes).
std::string write_test_gif(int delay_cs = 1) {
    std::vector<std::uint8_t> gif = {'G', 'I', 'F', '8', '9', 'a', 2, 0, 2, 0, 0xF7, 0, 0};
    for (int i = 0; i < 256; ++i) {
        gif.push_back(static_cast<std::uint8_t>(i));
        gif.push_back(static_cast<std::uint8_t>(i));
        gif.push_back(static_cast<std::uint8_t>(i));
    }
    const std::vector<std::vector<int>> frame_pixels = {{1, 2, 3, 4}, {4, 3, 2, 1}};
    for (const auto& pixels : frame_pixels) {
        gif.insert(gif.end(), {0x21, 0xF9, 0x04, 0x00, static_cast<std::uint8_t>(delay_cs), 0x00, 0x00, 0x00});
        gif.insert(gif.end(), {0x2C, 0, 0, 0, 0, 2, 0, 2, 0, 0x00, 0x08});
        std::vector<std::uint8_t> bytes;
        std::uint32_t acc = 0;
        int nbits = 0;
        std::vector<int> codes = {256};
        codes.insert(codes.end(), pixels.begin(), pixels.end());
        codes.push_back(257);
        for (const int code : codes) {
            acc |= static_cast<std::uint32_t>(code) << nbits;
            nbits += 9;
            while (nbits >= 8) {
                bytes.push_back(static_cast<std::uint8_t>(acc & 0xFF));
                acc >>= 8;
                nbits -= 8;
            }
        }
        if (nbits > 0) {
            bytes.push_back(static_cast<std::uint8_t>(acc & 0xFF));
        }
        gif.push_back(static_cast<std::uint8_t>(bytes.size()));
        gif.insert(gif.end(), bytes.begin(), bytes.end());
        gif.push_back(0x00);
    }
    gif.push_back(0x3B);

    const auto path = std::filesystem::temp_directory_path() / "tuinator_mdview_anim.gif";
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char*>(gif.data()), static_cast<std::streamsize>(gif.size()));
    return path.string();
}

void paint_widget(tuinator::Widget& widget, tuinator::MemoryTerminalBackend& backend) {
    backend.begin_frame({.full_redraw = true, .clear_buffer = true});
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    widget.paint(ctx);
    backend.end_frame();
}

void paint_widget(tuinator::Widget& widget, tuinator::detail::AnsiBackend& backend) {
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

std::string row_text(const tuinator::MemoryTerminalBackend& backend, int y) {
    std::string row;
    for (const auto& cell : backend.cells()[static_cast<std::size_t>(y)]) {
        row.push_back(cell.ch ? cell.ch : ' ');
    }
    return row;
}

} // namespace

TUINATOR_TEST(markdown_view_renders_heading_bold) {
    tuinator::MarkdownView view("# Hello");
    view.layout({0, 0, 40, 5});

    tuinator::MemoryTerminalBackend backend({40, 5});
    backend.init();
    paint_widget(view, backend);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "Hello"));
    const auto* cell = find_cell(backend, 'H');
    TUINATOR_CHECK(cell != nullptr);
    TUINATOR_CHECK(cell->style.bold);
}

TUINATOR_TEST(markdown_view_renders_emphasis_styles) {
    tuinator::MarkdownView view("a **b** *i* ~~s~~");
    view.layout({0, 0, 40, 3});

    tuinator::MemoryTerminalBackend backend({40, 3});
    backend.init();
    paint_widget(view, backend);

    const auto* bold = find_cell(backend, 'b');
    TUINATOR_CHECK(bold != nullptr);
    TUINATOR_CHECK(bold->style.bold);

    const auto* italic = find_cell(backend, 'i');
    TUINATOR_CHECK(italic != nullptr);
    TUINATOR_CHECK(italic->style.italic);

    const auto* strike = find_cell(backend, 's');
    TUINATOR_CHECK(strike != nullptr);
    TUINATOR_CHECK(strike->style.strikethrough);
}

TUINATOR_TEST(markdown_view_link_is_underlined) {
    tuinator::MarkdownView view("see [docs](https://example.com)");
    view.layout({0, 0, 40, 3});

    tuinator::MemoryTerminalBackend backend({40, 3});
    backend.init();
    paint_widget(view, backend);

    const auto* link = find_cell(backend, 'd');
    TUINATOR_CHECK(link != nullptr);
    TUINATOR_CHECK(link->style.underline);
}

TUINATOR_TEST(markdown_view_code_block_fills_row) {
    tuinator::MarkdownView view("```\nint x;\n```\n");
    view.layout({0, 0, 20, 4});

    tuinator::MemoryTerminalBackend backend({20, 4});
    backend.init();
    paint_widget(view, backend);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "int x;"));
    // Full-width background fill: last cell of the row carries the code fill.
    const auto& row = backend.cells()[0];
    const auto& fill = row.back();
    TUINATOR_CHECK(fill.style.background_rgb.has_value() || fill.style.background != tuinator::Color::Default);
}

TUINATOR_TEST(markdown_view_unclosed_fence_renders_as_code) {
    tuinator::MarkdownView view("```\npartial code");
    view.layout({0, 0, 30, 4});

    tuinator::MemoryTerminalBackend backend({30, 4});
    backend.init();
    paint_widget(view, backend);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "partial code"));
}

TUINATOR_TEST(markdown_view_unclosed_bold_renders_literally) {
    tuinator::MarkdownView view("wait **bo");
    view.layout({0, 0, 30, 3});

    tuinator::MemoryTerminalBackend backend({30, 3});
    backend.init();
    paint_widget(view, backend);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "wait **bo"));
}

TUINATOR_TEST(markdown_view_wraps_long_paragraph) {
    tuinator::MarkdownView view("one two three four five six");
    view.layout({0, 0, 10, 6});

    tuinator::MemoryTerminalBackend backend({10, 6});
    backend.init();
    paint_widget(view, backend);

    TUINATOR_CHECK(view.content_height() > 1);
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "one two"));
}

TUINATOR_TEST(markdown_view_rewraps_on_width_change) {
    tuinator::MarkdownView view("one two three four five six seven eight");
    view.layout({0, 0, 40, 6});
    const int wide_height = view.content_height();

    view.layout({0, 0, 10, 6});
    TUINATOR_CHECK(view.content_height() > wide_height);
}

TUINATOR_TEST(markdown_view_follow_tail_scrolls_to_end) {
    tuinator::MarkdownView view;
    view.layout({0, 0, 20, 3});
    for (int i = 0; i < 10; ++i) {
        view.append("line " + std::to_string(i) + "\n\n");
    }

    TUINATOR_CHECK(view.scroll_y() == view.max_scroll_y());

    tuinator::MemoryTerminalBackend backend({20, 3});
    backend.init();
    paint_widget(view, backend);
    TUINATOR_CHECK(tuinator::test::row_has(backend, "line 9"));
}

TUINATOR_TEST(markdown_view_scroll_by_clamps) {
    tuinator::MarkdownView view("a\n\nb\n\nc\n\nd\n\ne\n");
    view.layout({0, 0, 20, 2});

    view.scroll_by(0, 100);
    TUINATOR_CHECK(view.scroll_y() == view.max_scroll_y());

    view.scroll_by(0, -100);
    TUINATOR_CHECK_EQ(view.scroll_y(), 0);
}

TUINATOR_TEST(markdown_view_marks_dirty_on_append) {
    tuinator::MarkdownView view;
    view.layout({3, 2, 40, 5});

    tuinator::Rect reported{};
    view.set_on_dirty([&](tuinator::Rect region) { reported = region; });
    view.append("hello");

    TUINATOR_CHECK_EQ(reported.width, 40);
    TUINATOR_CHECK_EQ(reported.height, 5);
    TUINATOR_CHECK_EQ(reported.x, 3);
    TUINATOR_CHECK_EQ(reported.y, 2);
}

TUINATOR_TEST(markdown_view_list_markers) {
    tuinator::MarkdownView view("- bullet\n\n1. first\n");
    view.layout({0, 0, 30, 6});

    tuinator::MemoryTerminalBackend backend({30, 6});
    backend.init();
    paint_widget(view, backend);

    TUINATOR_CHECK(tuinator::test::row_has(backend, "bullet"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "1."));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "first"));
}

TUINATOR_TEST(markdown_view_quote_renders_bar) {
    tuinator::MarkdownView view("> quoted text\n");
    view.layout({0, 0, 30, 3});

    tuinator::MemoryTerminalBackend backend({30, 3});
    backend.init();
    paint_widget(view, backend);

    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "quoted text"));
    const std::string row = row_text(backend, 0);
    TUINATOR_CHECK(row[0] == '|' || row[0] == '\xE2'); // "|" ascii or "▎" utf-8 lead byte
}

TUINATOR_TEST(markdown_view_thematic_break_draws_line) {
    tuinator::MarkdownView view("above\n\n---\n\nbelow\n");
    view.layout({0, 0, 20, 6});

    tuinator::MemoryTerminalBackend backend({20, 6});
    backend.init();
    paint_widget(view, backend);

    TUINATOR_CHECK(tuinator::test::row_has(backend, "above"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "below"));
    bool found_rule = false;
    for (int y = 0; y < 6; ++y) {
        const std::string row = row_text(backend, y);
        if (row.find("----") != std::string::npos ||
            row.find("\xE2\x94\x80\xE2\x94\x80") != std::string::npos) {
            found_rule = true;
        }
    }
    TUINATOR_CHECK(found_rule);
}

TUINATOR_TEST(markdown_view_clear_empties_view) {
    tuinator::MarkdownView view("# Title\n\ntext");
    view.layout({0, 0, 30, 5});
    view.clear();

    TUINATOR_CHECK_EQ(view.content_height(), 0);
    TUINATOR_CHECK(view.source().empty());
}

TUINATOR_TEST(markdown_view_streaming_matches_full_parse) {
    const std::string full =
        "# Title\n\nSome **bold** and *italic* text with `code`.\n\n"
        "```cpp\nint x = 1;\nreturn x;\n```\n\n"
        "- one\n- two\n\n> a quote\n\n---\n\n"
        "A [link](https://example.com) and https://bare.url/ok.\n";

    tuinator::MarkdownView whole(full);
    whole.layout({0, 0, 50, 40});

    tuinator::MarkdownView streamed;
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

TUINATOR_TEST(markdown_view_broken_input_never_crashes) {
    const char* nasty[] = {
        "***",      "``",       "`",        "[]",   "](",   "![",  "**",
        "__",       "~~",       "\\",       "#",    "#x",   ">",   ">>",
        "-",        "1.",       "~~~",      "```",  "* *",  "_ _", "[a](",
        "[a](b",    "![a](b",   "a**b",     "\xC3", "\xE2\x94", "*a **b* c**",
    };

    for (const char* text : nasty) {
        tuinator::MarkdownView view;
        view.layout({0, 0, 24, 6});
        view.append(text);

        tuinator::MemoryTerminalBackend backend({24, 6});
        backend.init();
        paint_widget(view, backend);
    }
    TUINATOR_CHECK(true);
}

TUINATOR_TEST(markdown_view_renders_block_image) {
    const std::string path = write_test_png(); // 64x32 px
    tuinator::MarkdownView view("![Alt text](" + path + ")");
    view.layout({0, 0, 40, 10});

    tuinator::MemoryTerminalBackend backend({40, 10});
    backend.init();
    paint_widget(view, backend);

    // 64px wide at ~8px/col -> 8 cols; 32px at 2:1 cell aspect -> 2 rows.
    TUINATOR_CHECK_EQ(backend.image_draws().size(), 1U);
    TUINATOR_CHECK_EQ(backend.image_draws()[0].cell_size.width, 8);
    TUINATOR_CHECK_EQ(backend.image_draws()[0].cell_size.height, 2);
    TUINATOR_CHECK_EQ(backend.image_draws()[0].image.width(), 64);
}

TUINATOR_TEST(markdown_view_renders_two_block_images_simultaneously) {
    const std::string path_a = write_test_png();
    const std::string path_b = write_test_png_file("tuinator_mdview_test_b.png", 32, 16);

    const std::string markdown = "# Two images\n\n![First](" + path_a + ")\n\n![Second](" + path_b + ")\n";
    tuinator::MarkdownView view(markdown);
    view.layout({0, 0, 40, 12});

    tuinator::MemoryTerminalBackend backend({40, 12});
    backend.init();
    paint_widget(view, backend);

    TUINATOR_CHECK_EQ(backend.image_draws().size(), 2U);
    TUINATOR_CHECK(backend.image_draws()[0].y < backend.image_draws()[1].y);
    TUINATOR_CHECK_EQ(backend.image_draws()[0].image.width(), 64);
    TUINATOR_CHECK_EQ(backend.image_draws()[1].image.width(), 32);
    TUINATOR_CHECK(tuinator::test::row_has(backend, "Two images"));
}

TUINATOR_TEST(markdown_view_renders_same_image_twice_at_different_rows) {
    const std::string path = write_test_png();
    const std::string markdown = "![First](" + path + ")\n\n![Second](" + path + ")\n";
    tuinator::MarkdownView view(markdown);
    view.layout({0, 0, 40, 10});

    tuinator::MemoryTerminalBackend backend({40, 10});
    backend.init();
    paint_widget(view, backend);

    TUINATOR_CHECK_EQ(backend.image_draws().size(), 2U);
    TUINATOR_CHECK(backend.image_draws()[0].y != backend.image_draws()[1].y);
}

TUINATOR_TEST(markdown_view_kitty_places_same_file_twice) {
    const std::string path = write_test_png();
    tuinator::MarkdownView view("![A](" + path + ")\n\n![B](" + path + ")");
    view.layout({0, 0, 40, 10});

    tuinator::detail::AnsiBackend backend;
    constexpr std::size_t capacity = 1 << 20;
    char* buf = static_cast<char*>(std::malloc(capacity));
    FILE* mem = fmemopen(buf, capacity, "w");
    TUINATOR_CHECK(mem != nullptr);
    backend.set_output_file_for_testing(mem);

    paint_widget(view, backend);
    std::fflush(mem);
    const long length = std::ftell(mem);
    std::fseek(mem, 0, SEEK_SET);
    const std::string output(buf, static_cast<std::size_t>(length));

    const std::size_t place_a = output.find("a=p,i=2");
    const std::size_t place_b = output.find("a=p,i=4");
    TUINATOR_CHECK(place_a != std::string::npos);
    TUINATOR_CHECK(place_b != std::string::npos);
    TUINATOR_CHECK(place_a < place_b);

    std::fclose(mem);
    std::free(buf);
}

TUINATOR_TEST(markdown_view_kitty_places_two_block_images) {
    const std::string path_a = write_test_png();
    const std::string path_b = write_test_png_file("tuinator_mdview_kitty_b.png", 32, 16);

    tuinator::MarkdownView view("![A](" + path_a + ")\n\n![B](" + path_b + ")");
    view.layout({0, 0, 40, 12});

    tuinator::detail::AnsiBackend backend;
    constexpr std::size_t capacity = 1 << 20;
    char* buf = static_cast<char*>(std::malloc(capacity));
    FILE* mem = fmemopen(buf, capacity, "w");
    TUINATOR_CHECK(mem != nullptr);
    backend.set_output_file_for_testing(mem);

    paint_widget(view, backend);
    std::fflush(mem);
    const long length = std::ftell(mem);
    std::fseek(mem, 0, SEEK_SET);
    const std::string output(buf, static_cast<std::size_t>(length));

    const std::size_t place_a = output.find("a=p,i=2");
    const std::size_t place_b = output.find("a=p,i=4");
    TUINATOR_CHECK(place_a != std::string::npos);
    TUINATOR_CHECK(place_b != std::string::npos);
    TUINATOR_CHECK(place_a < place_b);
    TUINATOR_CHECK(output.find("a=d,d=i,i=2") == std::string::npos);

    std::fclose(mem);
    std::free(buf);
}

TUINATOR_TEST(markdown_view_image_reserves_visual_rows) {
    const std::string path = write_test_png();
    tuinator::MarkdownView view("![Alt](" + path + ")");
    view.layout({0, 0, 40, 10});
    TUINATOR_CHECK_EQ(view.content_height(), 2);
}

TUINATOR_TEST(markdown_view_inline_image_stays_text) {
    const std::string path = write_test_png();
    tuinator::MarkdownView view("see ![Alt](" + path + ") here");
    view.layout({0, 0, 40, 5});

    tuinator::MemoryTerminalBackend backend({40, 5});
    backend.init();
    paint_widget(view, backend);

    TUINATOR_CHECK(backend.image_draws().empty());
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "Alt"));
}

TUINATOR_TEST(markdown_view_missing_image_shows_alt_text) {
    tuinator::MarkdownView view("![Fallback](/nonexistent/tuinator_nope.png)");
    view.layout({0, 0, 40, 5});

    tuinator::MemoryTerminalBackend backend({40, 5});
    backend.init();
    paint_widget(view, backend);

    TUINATOR_CHECK(backend.image_draws().empty());
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "Fallback"));
}

TUINATOR_TEST(markdown_view_partially_visible_image_not_drawn) {
    const std::string path = write_test_png();
    tuinator::MarkdownView view("intro line\n\n![Alt](" + path + ")");
    view.layout({0, 0, 40, 3}); // image rows 2..3, only row 2 visible

    tuinator::MemoryTerminalBackend backend({40, 3});
    backend.init();
    paint_widget(view, backend);

    TUINATOR_CHECK(backend.image_draws().empty());
}

TUINATOR_TEST(markdown_view_render_images_disabled_shows_alt) {
    const std::string path = write_test_png();
    tuinator::MarkdownViewOptions options;
    options.render_images = false;
    tuinator::MarkdownView view("![Alt](" + path + ")", options);
    view.layout({0, 0, 40, 10});

    tuinator::MemoryTerminalBackend backend({40, 10});
    backend.init();
    paint_widget(view, backend);

    TUINATOR_CHECK(backend.image_draws().empty());
    TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "Alt"));
}

TUINATOR_TEST(markdown_view_renders_animated_gif) {
    const std::string path = write_test_gif();
    tuinator::MarkdownView view("![Anim](" + path + ")");
    view.layout({0, 0, 40, 10});

    tuinator::MemoryTerminalBackend backend({40, 10});
    backend.init();
    paint_widget(view, backend);

    TUINATOR_CHECK_EQ(backend.image_draws().size(), 1U);
    TUINATOR_CHECK_EQ(backend.image_draws()[0].image.width(), 2);
}

TUINATOR_TEST(markdown_view_animated_gif_needs_periodic_idle) {
    tuinator::MarkdownView animated("![Anim](" + write_test_gif() + ")");
    animated.layout({0, 0, 40, 10});
    TUINATOR_CHECK(animated.needs_periodic_idle());

    tuinator::MarkdownView still("![Still](" + write_test_png() + ")");
    still.layout({0, 0, 40, 10});
    TUINATOR_CHECK(!still.needs_periodic_idle());
}

TUINATOR_TEST(markdown_view_on_idle_marks_only_image_rows_dirty) {
    tuinator::MarkdownView view("intro\n\n![Anim](" + write_test_gif(1) + ")"); // 10ms frames
    view.layout({0, 0, 40, 10});

    tuinator::MemoryTerminalBackend backend({40, 10});
    backend.init();
    paint_widget(view, backend); // establishes anim start + drawn frame

    std::optional<tuinator::Rect> dirty;
    view.set_on_dirty([&](tuinator::Rect region) { dirty = region; });

    view.on_idle(); // same frame: nothing dirty yet
    TUINATOR_CHECK(!dirty.has_value());

    std::this_thread::sleep_for(std::chrono::milliseconds(30)); // cross a frame boundary
    view.on_idle();
    TUINATOR_CHECK(dirty.has_value());
    // image anchor is line index 2 (intro, blank, image): 2x2 px -> 4 cols x 2 rows
    TUINATOR_CHECK_EQ(dirty->y, 2);
    TUINATOR_CHECK_EQ(dirty->height, 2);
    TUINATOR_CHECK_EQ(dirty->width, 40);
}
