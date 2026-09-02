#include "render_helper.hpp"
#include "test_harness.hpp"

#include <tuinator/render/graphics_encode.hpp>
#include <tuinator/render/graphics_protocol.hpp>
#include <tuinator/render/terminal_image.hpp>
#include <tuinator/widgets/display/image_view.hpp>

#include <string>
#include <vector>

TUINATOR_TEST(image_view_records_draw_request) {
    tuinator::MemoryTerminalBackend backend({60, 20});
    backend.init();

    tuinator::ImageView view(tuinator::TerminalImage::gradient(32, 16), {20, 8});
    view.layout({5, 3, 20, 8});

    backend.begin_frame();
    tuinator::Canvas canvas(backend);
    tuinator::PaintContext ctx = tuinator::test::make_paint_context(canvas);
    view.paint(ctx);
    backend.end_frame();

    TUINATOR_CHECK_EQ(backend.image_draws().size(), 1U);
    TUINATOR_CHECK_EQ(backend.image_draws()[0].x, 0);
    TUINATOR_CHECK_EQ(backend.image_draws()[0].y, 0);
    TUINATOR_CHECK_EQ(backend.image_draws()[0].cell_size.width, 20);
    TUINATOR_CHECK_EQ(backend.image_draws()[0].image.width(), 32);
}

TUINATOR_TEST(kitty_encoder_emits_graphics_sequence) {
    const tuinator::TerminalImage image = tuinator::TerminalImage::gradient(8, 4);
    const std::string encoded = tuinator::encode_terminal_image(
        tuinator::GraphicsProtocol::Kitty,
        image,
        2,
        1,
        10,
        4);

    TUINATOR_CHECK(encoded.find("\033_G") != std::string::npos);
    TUINATOR_CHECK(encoded.find("a=t,f=100") != std::string::npos);
    TUINATOR_CHECK(encoded.find("m=0;\033\\") != std::string::npos);

    const std::string placed = tuinator::encode_kitty_place(10, 4);
    TUINATOR_CHECK(placed.find("a=p,i=1,c=10,r=4") != std::string::npos);
}

TUINATOR_TEST(png_loader_reads_lenna_asset) {
    const std::vector<std::string> candidates = {
        "examples/assets/lenna.png",
        "../examples/assets/lenna.png",
    };

    bool loaded = false;
    for (const std::string& path : candidates) {
        if (auto image = tuinator::TerminalImage::load_png(path)) {
            TUINATOR_CHECK(image->width() > 0);
            TUINATOR_CHECK(image->height() > 0);
            TUINATOR_CHECK_EQ(image->rgba().size(), static_cast<std::size_t>(image->width() * image->height() * 4));
            loaded = true;
            break;
        }
    }

    TUINATOR_CHECK(loaded);
}

TUINATOR_TEST(png_encoder_produces_signature) {
    const tuinator::TerminalImage image = tuinator::TerminalImage::gradient(4, 4);
    const std::vector<std::uint8_t> png = tuinator::rgba_to_png(image);
    TUINATOR_CHECK(png.size() >= 8);
    TUINATOR_CHECK_EQ(png[0], 0x89);
    TUINATOR_CHECK_EQ(png[1], 'P');
    TUINATOR_CHECK_EQ(png[2], 'N');
    TUINATOR_CHECK_EQ(png[3], 'G');
}
