#include <tuinator/render/animated_image.hpp>
#include <tuinator/tuinator.hpp>

#include <algorithm>
#include <memory>
#include <string>

namespace {

bool try_load_image(const std::string& path, tuinator::TerminalImage& out) {
    if (auto loaded = tuinator::TerminalImage::load_png(path)) {
        out = std::move(*loaded);
        return true;
    }
    if (auto loaded = tuinator::TerminalImage::load_ppm(path)) {
        out = std::move(*loaded);
        return true;
    }
    if (auto gif = tuinator::AnimatedImage::load_gif(path)) {
        if (!gif->empty()) {
            out = gif->frames().front().image;
            return true;
        }
    }
    return false;
}

tuinator::TerminalImage scale_if_needed(tuinator::TerminalImage image) {
    constexpr int kMaxHeight = 240;
    if (image.height() > kMaxHeight) {
        const int width = std::max(1, image.width() * kMaxHeight / image.height());
        return image.resized(width, kMaxHeight);
    }
    return image;
}

tuinator::TerminalImage load_first_existing(const char* const* paths, std::size_t count,
                                            tuinator::TerminalImage fallback) {
    for (std::size_t i = 0; i < count; ++i) {
        tuinator::TerminalImage image;
        if (try_load_image(paths[i], image)) {
            return scale_if_needed(std::move(image));
        }
    }
    return std::move(fallback);
}

tuinator::Size display_cells_for(const tuinator::TerminalImage& image, int cell_width = 34) {
    int height = image.width() > 0 ? image.height() * cell_width / image.width() : 12;
    height = std::clamp(height, 8, 18);
    return {cell_width, height};
}

std::unique_ptr<tuinator::VBox> make_image_column(const std::string& title, const tuinator::TerminalImage& image,
                                                  const tuinator::Theme& theme) {
    auto col = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 0});
    col->set_flex(1);
    col->add_child(std::make_unique<tuinator::Label>(title, theme.heading));
    col->add_child(std::make_unique<tuinator::ImageView>(image, display_cells_for(image)));
    col->add_child(std::make_unique<tuinator::Label>(std::to_string(image.width()) + "x" +
                                                           std::to_string(image.height()) + " px",
                                                       theme.muted));
    return col;
}

} // namespace

int main() {
    tuinator::Application app;
    const tuinator::Theme theme = app.theme();
    const tuinator::GraphicsProtocol protocol = tuinator::active_graphics_protocol();

    static const char* kLennaPaths[] = {
#ifdef TUINATOR_LENNA_PATH
        TUINATOR_LENNA_PATH,
#endif
        "examples/assets/lenna.png",
        "../examples/assets/lenna.png",
    };

    static const char* kSpinnerPaths[] = {
#ifdef TUINATOR_SPINNER_PATH
        TUINATOR_SPINNER_PATH,
#endif
        "examples/assets/spinner.gif",
        "../examples/assets/spinner.gif",
    };

    const tuinator::TerminalImage lenna =
        load_first_existing(kLennaPaths, std::size(kLennaPaths), tuinator::TerminalImage::gradient(160, 120));
    const tuinator::TerminalImage spinner =
        load_first_existing(kSpinnerPaths, std::size(kSpinnerPaths), tuinator::TerminalImage::gradient(120, 120));

    auto root = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    root->add_child(std::make_unique<tuinator::Label>("Zwei Bilder gleichzeitig", theme.heading));
    root->add_child(std::make_unique<tuinator::Label>("Protocol: " + tuinator::graphics_protocol_name(protocol) +
                                                          "  |  TUINATOR_GRAPHICS=kitty|iterm2|sixel|auto",
                                                      theme.muted));

    auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2, .padding = 0});
    row->add_child(make_image_column("Lenna (PNG)", lenna, theme));
    row->add_child(make_image_column("Spinner (GIF, 1. Frame)", spinner, theme));
    root->add_child(std::move(row));

    root->add_child(std::make_unique<tuinator::Label>("Beide ImageViews werden in einer HBox nebeneinander gerendert.",
                                                      theme.muted));
    root->add_child(std::make_unique<tuinator::Label>("Drücke q zum Beenden", theme.muted));

    app.set_root(std::move(root));
    return app.run();
}
