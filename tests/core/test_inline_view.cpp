#include <tuinator/tuinator.hpp>

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>

#include "test_harness.hpp"

#if TUINATOR_PLATFORM_POSIX

namespace {

struct CaptureOutput {
    char* buffer = nullptr;
    size_t length = 0;
    FILE* file = nullptr;

    CaptureOutput() { file = open_memstream(&buffer, &length); }

    ~CaptureOutput() {
        if (file != nullptr) {
            std::fclose(file);
        }
        std::free(buffer);
    }

    std::string text() const { return std::string(buffer != nullptr ? buffer : "", length); }
};

} // namespace

TUINATOR_TEST(flush_cli_output_smoke) { tuinator::flush_cli_output(); }

TUINATOR_TEST(inline_view_region_size_matches_height) {
    CaptureOutput capture;
    tuinator::InlineView view({.height = 6, .output = capture.file});
    view.set_root(std::make_unique<tuinator::Label>("size"));
    view.start();
    TUINATOR_CHECK_EQ(view.region_size().height, 6);
}

TUINATOR_TEST(inline_view_start_renders_label) {
    CaptureOutput capture;
    TUINATOR_CHECK(capture.file != nullptr);

    tuinator::InlineView view({.height = 3, .clear_on_shutdown = false, .output = capture.file});
    view.set_root(std::make_unique<tuinator::Label>(
        "hello inline", tuinator::Style{.foreground = tuinator::Color::Green, .bold = true}));
    view.start();

    const std::string out = capture.text();
    TUINATOR_CHECK(out.find("hello inline") != std::string::npos);
    TUINATOR_CHECK(out.find("\033[32m") != std::string::npos);
}

TUINATOR_TEST(inline_view_finish_is_idempotent) {
    CaptureOutput capture;
    tuinator::InlineView view({.height = 2, .output = capture.file});
    view.set_root(std::make_unique<tuinator::Label>("bye"));
    view.start();
    view.finish();
    view.finish();
}

TUINATOR_TEST(inline_scene_load_from_string) {
    CaptureOutput capture;
    tuinator::InlineView view({.height = 6, .clear_on_shutdown = false, .output = capture.file});

    static constexpr char kScene[] = R"json({
        "version": 1,
        "root": {
            "type": "VBox",
            "gap": 0,
            "children": [
                {
                    "type": "Label",
                    "text": "From JSON",
                    "style": { "foreground": "Cyan", "bold": true }
                },
                {
                    "type": "Label",
                    "text": "inline band",
                    "style": "muted"
                }
            ]
        }
    })json";

    auto scene = tuinator::scene::load_and_build_from_string(kScene, view.app(), view.app().theme(), {});
    TUINATOR_CHECK(scene.root != nullptr);

    view.set_root(std::move(scene.root));
    view.start();

    const std::string out = capture.text();
    TUINATOR_CHECK(out.find("From JSON") != std::string::npos);
    TUINATOR_CHECK(out.find("inline band") != std::string::npos);
}

TUINATOR_TEST(inline_view_run_with_timer_updates_and_quits) {
    CaptureOutput capture;
    tuinator::InlineView view({.height = 3, .clear_on_shutdown = false, .output = capture.file});

    struct State {
        tuinator::Label* label = nullptr;
        int ticks = 0;
    } state;

    auto root = std::make_unique<tuinator::VBox>();
    auto label = std::make_unique<tuinator::Label>("tick 0");
    state.label = label.get();
    root->add_child(std::move(label));
    view.set_root(std::move(root));
    view.start();

    view.app().set_interval(10, [&]() {
        state.ticks += 1;
        state.label->set_text("tick " + std::to_string(state.ticks));
        if (state.ticks >= 3) {
            view.app().quit();
        }
    });

    const int code = view.run();
    view.finish();

    TUINATOR_CHECK_EQ(code, 0);
    TUINATOR_CHECK(state.ticks >= 3);
    TUINATOR_CHECK(capture.text().find("tick 3") != std::string::npos);
}

TUINATOR_TEST(inline_view_present_skips_unchanged_frame) {
    CaptureOutput capture;
    tuinator::InlineView view({.height = 2, .output = capture.file});
    view.set_root(std::make_unique<tuinator::Label>("once"));
    view.present();
    const std::size_t after_first = capture.length;
    view.present();
    TUINATOR_CHECK_EQ(capture.length, after_first);
    TUINATOR_CHECK(capture.text().find("once") != std::string::npos);
    view.finish();
}

TUINATOR_TEST(inline_scene_button_quit_handler) {
    CaptureOutput capture;
    tuinator::InlineView view({.height = 5, .keyboard_input = true, .output = capture.file});

    tuinator::scene::HandlerRegistry handlers;
    handlers.register_handler("quit", [](tuinator::Application& app) { app.quit(); });

    static constexpr char kScene[] = R"json({
        "version": 1,
        "root": {
            "type": "VBox",
            "children": [
                { "type": "Label", "text": "Press button" },
                { "type": "Button", "label": "Quit", "onClick": "quit" }
            ]
        }
    })json";

    tuinator::scene::LoadOptions options;
    options.handlers = &handlers;
    auto scene = tuinator::scene::load_and_build_from_string(kScene, view.app(), view.app().theme(), options);
    view.set_root(std::move(scene.root));
    view.start();
    view.finish();
}

#endif
