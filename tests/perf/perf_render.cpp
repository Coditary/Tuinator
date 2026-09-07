#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/layout/box.hpp>
#include <tuinator/render/dirty_region.hpp>
#include <tuinator/render/paint_context.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/containers/panel.hpp>
#include <tuinator/widgets/display/label.hpp>

#include <memory>
#include <string>

#include "bench_harness.hpp"
#include "render_helper.hpp"

namespace {

std::unique_ptr<tuinator::Widget> make_dashboard_tree() {
    auto root = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    const tuinator::Theme theme = tuinator::dark_theme();

    root->add_child(std::make_unique<tuinator::Label>("Tuinator Dashboard", theme.heading));
    root->add_child(std::make_unique<tuinator::Label>("Performance harness — mixed labels and panels", theme.muted));

    auto row = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 1});
    for (int panel = 0; panel < 3; ++panel) {
        auto panel_widget = std::make_unique<tuinator::Panel>("Panel " + std::to_string(panel + 1));
        auto content = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
        for (int line = 0; line < 6; ++line) {
            content->add_child(std::make_unique<tuinator::Label>(
                "Row " + std::to_string(line + 1) + " — UTF-8 ☀️ metrics αβγ 数据", theme.label));
        }
        panel_widget->set_content(std::move(content));
        row->add_child(std::move(panel_widget));
    }
    root->add_child(std::move(row));

    auto footer = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 2});
    for (int i = 0; i < 4; ++i) {
        footer->add_child(std::make_unique<tuinator::Label>("Status " + std::to_string(i), theme.muted));
    }
    root->add_child(std::move(footer));

    return root;
}

void render_tree(tuinator::Widget& root, tuinator::MemoryTerminalBackend& backend, bool partial, int dirty_y) {
    const tuinator::Size term = backend.terminal_size();
    root.layout({0, 0, term.width, term.height});

    tuinator::BeginFrameOptions frame;
    frame.full_redraw = !partial;
    if (partial) {
        frame.dirty_region = {0, dirty_y, term.width, 4};
    }

    backend.begin_frame(frame);
    tuinator::Canvas canvas(backend);
    const tuinator::Theme theme = tuinator::dark_theme();
    canvas.set_glyphs(theme.glyphs);
    tuinator::PaintContext ctx{canvas, theme};
    root.paint(ctx);
    backend.end_frame();
}

} // namespace

TUINATOR_PERF_TEST(render_dashboard_full_frame) {
    tuinator::MemoryTerminalBackend backend({120, 40});
    backend.init();
    auto root = make_dashboard_tree();

    return tuinator::perf::bench("render_dashboard_full_frame", 250.0, 2, 30,
                                 [&]() { render_tree(*root, backend, false, 0); });
}

TUINATOR_PERF_TEST(render_dashboard_partial_frame) {
    tuinator::MemoryTerminalBackend backend({120, 40});
    backend.init();
    auto root = make_dashboard_tree();
    render_tree(*root, backend, false, 0);

    int dirty_y = 0;
    return tuinator::perf::bench("render_dashboard_partial_frame", 180.0, 2, 40, [&]() {
        render_tree(*root, backend, true, dirty_y);
        dirty_y = (dirty_y + 2) % 20;
    });
}

TUINATOR_PERF_TEST(dirty_region_many_unions) {
    tuinator::DirtyRegion region;
    return tuinator::perf::bench("dirty_region_many_unions", 20.0, 2, 20, [&]() {
        region.mark_full();
        region.clear();
        for (int i = 0; i < 500; ++i) {
            region.mark_rect({i % 40, i % 20, 8, 4});
        }
        region.bounds();
    });
}
