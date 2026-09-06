#include <tuinator/layout/box.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/chrome/terminal_frame.hpp>
#include <tuinator/widgets/display/label.hpp>
#include <tuinator/widgets/display/progress_bar.hpp>

#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(terminal_frame_renders_title_border_and_status_line) {
    auto content = std::make_unique<tuinator::VBox>();
    content->add_child(std::make_unique<tuinator::Label>("Hello from embedded terminal"));
    content->add_child(std::make_unique<tuinator::Label>("Second line inside the frame"));

    tuinator::TerminalFrameStyle style;
    style.border = tuinator::style_fg(tuinator::Rgb{0x7d, 0xcf, 0xff});
    style.title = tuinator::style_fg_bg(tuinator::Rgb{0xc0, 0xca, 0xf5}, tuinator::Rgb{0x1a, 0x1b, 0x26});
    style.title.bold = true;
    style.title_background = style.title;
    style.content_background = tuinator::style_fg_bg(tuinator::Rgb{0xc0, 0xca, 0xf5}, tuinator::Rgb{0x1a, 0x1b, 0x26});
    style.status_line.background =
        tuinator::style_fg_bg(tuinator::Rgb{0xa9, 0xb1, 0xd6}, tuinator::Rgb{0x16, 0x16, 0x1e});

    tuinator::TerminalFrameOptions options;
    options.style = style;

    tuinator::TerminalFrame frame("tuinator — bash", std::move(content), options);
    frame.set_status_left({
        {.text = "NORMAL", .foreground_rgb = tuinator::Rgb{0x9e, 0xce, 0x6a}, .bold = true},
        {.text = "UTF-8", .foreground_rgb = tuinator::Rgb{0x7d, 0xcf, 0xff}},
    });
    frame.set_status_right({
        {.text = "42:13", .foreground_rgb = tuinator::Rgb{0xc0, 0xca, 0xf5}},
    });

    const tuinator::Size preferred = frame.preferred_size();
    tuinator::MemoryTerminalBackend backend({preferred.width + 4, preferred.height + 4});
    backend.init();
    frame.layout({1, 1, preferred.width, preferred.height});
    tuinator::test::render_root(frame, backend);

    TUINATOR_CHECK(tuinator::test::row_has(backend, "tuinator"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "Hello from embedded terminal"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "NORMAL"));
    TUINATOR_CHECK(tuinator::test::row_has(backend, "42:13"));
}

TUINATOR_TEST(terminal_frame_surface_matches_content_area) {
    auto label = std::make_unique<tuinator::Label>("Surface test");
    tuinator::TerminalFrame frame("Surface", std::move(label));

    frame.layout({0, 0, 40, 12});
    frame.refresh_surface(tuinator::dark_theme());

    const tuinator::Size surface = frame.surface().terminal_size();
    TUINATOR_CHECK_EQ(surface.width, 38);
    TUINATOR_CHECK_EQ(surface.height, 9);
    TUINATOR_CHECK(tuinator::test::row_has(frame.surface(), "Surface test"));
}

TUINATOR_TEST(terminal_frame_without_status_line_is_shorter) {
    tuinator::TerminalFrameOptions options;
    options.show_status_line = false;

    tuinator::TerminalFrame frame("No status", std::make_unique<tuinator::Label>("content"), options);

    const tuinator::Size size = frame.preferred_size();
    TUINATOR_CHECK(size.height >= 5);
    TUINATOR_CHECK(size.width >= 16);
}
