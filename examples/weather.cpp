#include <tuinator/tuinator.hpp>

#include <memory>

int main() {
    tuinator::Application app;

    tuinator::NerdIconCatalogGalleryStyle gallery_style;
    gallery_style.background = tuinator::style_fg_bg(
        tuinator::Rgb{0xc0, 0xca, 0xf5},
        tuinator::Rgb{0x1a, 0x1b, 0x26});
    gallery_style.title = tuinator::style_fg_bg(
        tuinator::Rgb{0x7d, 0xcf, 0xff},
        tuinator::Rgb{0x1a, 0x1b, 0x26});
    gallery_style.title.bold = true;
    gallery_style.domain_header = tuinator::style_fg_bg(
        tuinator::Rgb{0xbb, 0x9a, 0xf7},
        tuinator::Rgb{0x1a, 0x1b, 0x26});
    gallery_style.domain_header.bold = true;
    gallery_style.row_text = gallery_style.background;
    gallery_style.icon = tuinator::style_fg_bg(
        tuinator::Rgb{0x9e, 0xce, 0x6a},
        tuinator::Rgb{0x1a, 0x1b, 0x26});

    auto gallery = std::make_unique<tuinator::NerdIconCatalogGallery>(gallery_style);

    tuinator::ScrollViewOptions scroll_options;
    scroll_options.width = 84;
    scroll_options.height = 24;
    scroll_options.background = gallery_style.background;

    auto root = std::make_unique<tuinator::ScrollView>(std::move(gallery), scroll_options);
    app.set_root(std::move(root));
    return app.run();
}
