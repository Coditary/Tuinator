#include <tuinator/tuinator.hpp>

#include <cstdlib>
#include <memory>
#include <string>

namespace {

tuinator::Rgb lerp_rgb(tuinator::Rgb from, tuinator::Rgb to, float t) {
    const auto mix = [](int a, int b, float amount) {
        return static_cast<std::uint8_t>(static_cast<float>(a) + (static_cast<float>(b - a) * amount));
    };

    return {mix(from.r, to.r, t), mix(from.g, to.g, t), mix(from.b, to.b, t)};
}

tuinator::Rgb gradient_color(float t) {
    constexpr tuinator::Rgb kTeal{95, 184, 158};
    constexpr tuinator::Rgb kLavender{184, 152, 208};
    constexpr tuinator::Rgb kMagenta{232, 72, 160};

    if (t <= 0.5f) {
        return lerp_rgb(kTeal, kLavender, t / 0.5f);
    }

    return lerp_rgb(kLavender, kMagenta, (t - 0.5f) / 0.5f);
}

void add_gradient_column(tuinator::VBox& column, bool ansi_only) {
    constexpr int kGradientSteps = 32;
    constexpr char kBar[] = "████████████████";

    for (int i = 0; i < kGradientSteps; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(kGradientSteps - 1);
        const tuinator::ColorValue value = tuinator::ColorValue::from_rgb(gradient_color(t));
        const tuinator::Style style = ansi_only ? value.ansi_foreground_style() : value.foreground_style();

        column.add_child(std::make_unique<tuinator::Label>(kBar, style));
    }
}

} // namespace

int main() {
    tuinator::Application app;

    auto root = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0, .padding = 1});

    root->add_child(std::make_unique<tuinator::Label>(
        "Color Palette (8-color)", tuinator::Style{.foreground = tuinator::Color::White, .bold = true}));

    const tuinator::Color colors[] = {
        tuinator::Color::Red,     tuinator::Color::Green, tuinator::Color::Yellow, tuinator::Color::Blue,
        tuinator::Color::Magenta, tuinator::Color::Cyan,  tuinator::Color::White,
    };

    const char* names[] = {
        "Red", "Green", "Yellow", "Blue", "Magenta", "Cyan", "White",
    };

    for (std::size_t i = 0; i < sizeof(colors) / sizeof(colors[0]); ++i) {
        root->add_child(std::make_unique<tuinator::Label>(std::string("* ") + names[i],
                                                          tuinator::Style{.foreground = colors[i], .bold = true}));
    }

    root->add_child(std::make_unique<tuinator::Label>(""));
    {
        const char* colorterm = std::getenv("COLORTERM");
        const char* term = std::getenv("TERM");
        root->add_child(std::make_unique<tuinator::Label>(
            std::string("COLORTERM=") + (colorterm != nullptr ? colorterm : "(unset)") +
                "  TERM=" + (term != nullptr ? term : "(unset)"),
            tuinator::Style{.foreground = tuinator::Color::Yellow, .dim = true}));
    }
    root->add_child(std::make_unique<tuinator::Label>(
        "Gradient: true color vs ANSI fallback", tuinator::Style{.foreground = tuinator::Color::White, .bold = true}));

    auto headers = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 4});
    headers->add_child(std::make_unique<tuinator::Label>("True color (24-bit RGB)",
                                                         tuinator::Style{.foreground = tuinator::Color::Cyan}));
    headers->add_child(std::make_unique<tuinator::Label>("ANSI fallback (nearest)",
                                                         tuinator::Style{.foreground = tuinator::Color::Yellow}));
    root->add_child(std::move(headers));

    auto gradients = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 4});
    auto true_color = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    auto ansi_color = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    add_gradient_column(*true_color, false);
    add_gradient_column(*ansi_color, true);
    gradients->add_child(std::move(true_color));
    gradients->add_child(std::move(ansi_color));
    root->add_child(std::move(gradients));

    root->add_child(std::make_unique<tuinator::Label>(""));
    root->add_child(std::make_unique<tuinator::Label>(
        "bold text", tuinator::Style{.foreground = tuinator::Color::Cyan, .bold = true}));
    root->add_child(std::make_unique<tuinator::Label>(
        "dim text", tuinator::Style{.foreground = tuinator::Color::Yellow, .dim = true}));
    root->add_child(std::make_unique<tuinator::Label>("reverse text", tuinator::Style{.reverse = true}));

    root->add_child(std::make_unique<tuinator::Label>(
        "Press q to quit", tuinator::Style{.foreground = tuinator::Color::Yellow, .dim = true}));

    app.set_root(std::move(root));
    return app.run();
}
