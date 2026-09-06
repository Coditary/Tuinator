#include <tuinator/tuinator.hpp>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace {

bool try_load_file(const std::string& path, tuinator::TerminalImage& out) {
    if (auto loaded = tuinator::TerminalImage::load_png(path)) {
        out = std::move(*loaded);
        return true;
    }
    if (auto loaded = tuinator::TerminalImage::load_ppm(path)) {
        out = std::move(*loaded);
        return true;
    }
    return false;
}

tuinator::TerminalImage load_default_image() {
    static const char* kCandidates[] = {
#ifdef TUINATOR_LENNA_PATH
        TUINATOR_LENNA_PATH,
#endif
        "examples/assets/lenna.png",
        "../examples/assets/lenna.png",
    };

    for (const char* path : kCandidates) {
        tuinator::TerminalImage image;
        if (!try_load_file(path, image)) {
            continue;
        }

        constexpr int kMaxHeight = 240;
        if (image.height() > kMaxHeight) {
            const int width = std::max(1, image.width() * kMaxHeight / image.height());
            return image.resized(width, kMaxHeight);
        }
        return image;
    }

    return tuinator::TerminalImage::gradient(160, 80);
}

tuinator::Size display_cells_for(const tuinator::TerminalImage& image) {
    constexpr int kWidth = 40;
    int height = image.width() > 0 ? image.height() * kWidth / image.width() : 12;
    height = std::clamp(height, 8, 22);
    return {kWidth, height};
}

} // namespace

int main(int argc, char** argv) {
    tuinator::Application app;
    const tuinator::Theme theme = app.theme();
    const tuinator::GraphicsProtocol protocol = tuinator::active_graphics_protocol();

    tuinator::TerminalImage image = load_default_image();
    if (argc > 1) {
        tuinator::TerminalImage loaded;
        if (try_load_file(argv[1], loaded)) {
            image = std::move(loaded);
        }
    }

    const tuinator::Size display_cells = display_cells_for(image);

    auto root = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    root->add_child(std::make_unique<tuinator::Label>("Tuinator image test", theme.heading));
    root->add_child(std::make_unique<tuinator::Label>("Protocol: " + tuinator::graphics_protocol_name(protocol) +
                                                          "  |  TUINATOR_GRAPHICS=kitty|iterm2|sixel|auto",
                                                      theme.muted));
    root->add_child(std::make_unique<tuinator::ImageView>(image, display_cells));
    root->add_child(std::make_unique<tuinator::Label>("Lenna test image (" + std::to_string(image.width()) + "x" +
                                                          std::to_string(image.height()) +
                                                          " px). Override: argv[1] path.",
                                                      theme.muted));
    root->add_child(std::make_unique<tuinator::Label>("Press q to quit", theme.muted));

    app.set_root(std::move(root));
    return app.run();
}
