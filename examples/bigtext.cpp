#include <tuinator/tuinator.hpp>

#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace {

class BigTextRoot : public tuinator::VBox {
  public:
    explicit BigTextRoot(tuinator::BoxOptions options) : tuinator::VBox(options) {}

    bool wants_full_screen() const override { return true; }
};

const char* sample_for(tuinator::BigTextKind kind) {
    switch (kind) {
    case tuinator::BigTextKind::Letter: return "ITSFOSS";
    case tuinator::BigTextKind::Stacked: return "NEXIS";
    case tuinator::BigTextKind::Slant: return "awesome";
    case tuinator::BigTextKind::Doom: return "DOOM";
    default: return "Hello";
    }
}

const char* showcase_text(const tuinator::BigTextLook& look) {
    if (std::strcmp(look.id, "omarchy") == 0) {
        return "OMARCHY";
    }
    if (std::strcmp(look.id, "latchdark") == 0) {
        return "LATCHDARK";
    }
    if (std::strcmp(look.id, "sunset") == 0) {
        return "SUNSET";
    }
    if (std::strcmp(look.id, "ocean") == 0) {
        return "DEEP";
    }
    if (std::strcmp(look.id, "neon") == 0) {
        return "NEON";
    }
    if (std::strcmp(look.id, "ice") == 0) {
        return "FROST";
    }
    if (std::strcmp(look.id, "doom-fire") == 0) {
        return "DOOM";
    }
    return "COOL";
}

void add_look_showcase(tuinator::VBox& content, const tuinator::Theme& theme, const tuinator::BigTextLook& look) {
    auto block = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    block->add_child(std::make_unique<tuinator::Label>(look.title, theme.muted));

    tuinator::BigTextOptions options;
    tuinator::apply_big_text_look(options, look);

    auto text = std::make_unique<tuinator::BigText>(showcase_text(look), options, theme.heading);
    if (std::strcmp(look.id, "latchdark") == 0) {
        tuinator::Style border = theme.border;
        border.foreground_rgb = tuinator::Rgb::hex(0x707070);
        auto panel =
            std::make_unique<tuinator::Panel>("", border, theme.heading, tuinator::unicode_rounded_border_glyphs());
        panel->set_content(std::move(text));
        block->add_child(std::move(panel));
    } else {
        block->add_child(std::move(text));
    }

    content.add_child(std::move(block));
}

} // namespace

int main() {
    tuinator::Application app;
    const tuinator::Theme& theme = app.theme();

    auto content = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    content->add_child(
        std::make_unique<tuinator::Label>("Big text  |  q quit  |  wheel/PgUp/PgDn scroll", theme.muted));

    content->add_child(std::make_unique<tuinator::Label>("Color themes", theme.heading));
    for (const tuinator::BigTextLook& look : tuinator::all_big_text_looks()) {
        add_look_showcase(*content, theme, look);
    }

    content->add_child(std::make_unique<tuinator::Separator>());
    content->add_child(std::make_unique<tuinator::Label>("All styles", theme.heading));

    auto hello = std::make_unique<tuinator::BigText>("Hello\nWorld",
                                                     tuinator::BigTextOptions{
                                                         .kind = tuinator::BigTextKind::Block,
                                                         .underline = "▀▄",
                                                     },
                                                     theme.heading);
    content->add_child(std::move(hello));

    for (const tuinator::BigTextPreset& preset : tuinator::all_big_text_styles()) {
        auto block = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
        block->add_child(std::make_unique<tuinator::Label>(preset.title, theme.muted));

        tuinator::BigTextOptions options;
        options.kind = preset.kind;
        options.rainbow = preset.kind == tuinator::BigTextKind::Letter || preset.kind == tuinator::BigTextKind::Banner;
        if (preset.kind == tuinator::BigTextKind::Block) {
            options.underline = "▀▄";
        }
        if (preset.kind == tuinator::BigTextKind::Stacked) {
            tuinator::Style shadow;
            shadow.foreground = tuinator::Color::Cyan;
            options.shadow_style = shadow;
        }

        auto text = std::make_unique<tuinator::BigText>(sample_for(preset.kind), options, theme.accent);
        block->add_child(std::move(text));
        content->add_child(std::move(block));
    }

    auto scroll = std::make_unique<tuinator::ScrollView>(std::move(content),
                                                         tuinator::ScrollViewOptions{.width = 100, .height = 24});
    scroll->set_flex(1);

    auto root = std::make_unique<BigTextRoot>(tuinator::BoxOptions{.gap = 0, .padding = 0});
    root->add_child(std::move(scroll));
    app.set_root(std::move(root));
    return app.run();
}
