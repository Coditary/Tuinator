#include <tuinator/tuinator.hpp>

#include <memory>

int main() {
    tuinator::Application app;

    const auto sections = tuinator::load_nerd_glyph_catalog("data");

    tuinator::NerdGlyphMatrixGalleryStyle style;
    style.background = tuinator::style_fg_bg(tuinator::Rgb{0xc0, 0xca, 0xf5}, tuinator::Rgb{0x1a, 0x1b, 0x26});
    style.title = tuinator::style_fg_bg(tuinator::Rgb{0x7d, 0xcf, 0xff}, tuinator::Rgb{0x1a, 0x1b, 0x26});
    style.title.bold = true;
    style.domain_header = tuinator::style_fg_bg(tuinator::Rgb{0xbb, 0x9a, 0xf7}, tuinator::Rgb{0x1a, 0x1b, 0x26});
    style.domain_header.bold = true;
    style.category_header = tuinator::style_fg_bg(tuinator::Rgb{0x7a, 0x7c, 0x9e}, tuinator::Rgb{0x1a, 0x1b, 0x26});
    style.glyph = tuinator::style_fg_bg(tuinator::Rgb{0x9e, 0xce, 0x6a}, tuinator::Rgb{0x1a, 0x1b, 0x26});

    auto gallery = std::make_unique<tuinator::NerdGlyphMatrixGallery>(sections, 32, style);

    tuinator::ScrollViewOptions scroll_options;
    scroll_options.width = 100;
    scroll_options.height = 32;
    scroll_options.background = style.background;

    auto root = std::make_unique<tuinator::ScrollView>(std::move(gallery), scroll_options);
    app.set_root(std::move(root));
    return app.run();
}
