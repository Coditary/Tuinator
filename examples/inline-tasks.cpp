#include <tuinator/tuinator.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace {

struct TaskState {
    std::vector<std::string> completed;
    std::string current;
    int progress = 0;
    bool done = false;
};

struct DemoUi {
    tuinator::ListView* list = nullptr;
    tuinator::StatusLine* status = nullptr;
    tuinator::Throbber* status_throbber = nullptr;
    std::vector<tuinator::Throbber*> showcase_throbbers;
};

void add_showcase_throbber(
    tuinator::HBox& row,
    std::string_view name,
    std::string_view label,
    DemoUi& ui) {
    const tuinator::ThrobberSet* set = tuinator::throbber_set_named(name);
    if (set == nullptr) {
        return;
    }

    auto throbber = std::make_unique<tuinator::Throbber>(
        *set,
        tuinator::Style{.foreground_rgb = tuinator::Rgb::hex(0x89B4FA), .bold = true});
    ui.showcase_throbbers.push_back(throbber.get());
    row.add_child(std::move(throbber));
    row.add_child(std::make_unique<tuinator::Label>(
        std::string(label),
        tuinator::Style{.foreground_rgb = tuinator::Rgb::hex(0x6C7086), .dim = true}));
}

std::unique_ptr<tuinator::Widget> build_ui(TaskState& state, DemoUi& ui) {
    auto root = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0});
    root->set_flex(1);

    auto list = std::make_unique<tuinator::ListView>(
        tuinator::Style{.foreground = tuinator::Color::Default},
        tuinator::Style{.foreground = tuinator::Color::Green, .bold = true});
    list->set_items(state.completed);
    list->set_flex(1);
    ui.list = list.get();

    auto scroll = std::make_unique<tuinator::ScrollView>(
        std::move(list),
        tuinator::ScrollViewOptions{
            .width = 80,
            .height = 8,
        });
    scroll->set_flex(1);
    root->add_child(std::move(scroll));

    auto spinner_row = std::make_unique<tuinator::HBox>(
        tuinator::BoxOptions{.gap = 2, .padding = 1});
    spinner_row->add_child(std::make_unique<tuinator::Label>(
        "Spinners",
        tuinator::Style{.foreground_rgb = tuinator::Rgb::hex(0x45475A), .dim = true}));
    add_showcase_throbber(*spinner_row, "dots", "braille", ui);
    add_showcase_throbber(*spinner_row, "arrow", "arrows", ui);
    add_showcase_throbber(*spinner_row, "ascii", "line", ui);
    root->add_child(std::move(spinner_row));

    ui.status_throbber = ui.showcase_throbbers.empty() ? nullptr : ui.showcase_throbbers.front();

    auto status = std::make_unique<tuinator::StatusLine>();
    status->set_left({
        {.kind = tuinator::StatusSegmentKind::Text,
         .text = ui.status_throbber != nullptr ? ui.status_throbber->frame() : " ",
         .foreground_rgb = tuinator::Rgb::hex(0xF9E2AF),
         .bold = true},
        {.kind = tuinator::StatusSegmentKind::Text,
         .text = "|",
         .foreground_rgb = tuinator::Rgb::hex(0x585B70),
         .dim = true},
        {.kind = tuinator::StatusSegmentKind::Text, .text = "Tuinator", .bold = true},
        {.kind = tuinator::StatusSegmentKind::Text,
         .text = "|",
         .foreground_rgb = tuinator::Rgb::hex(0x585B70),
         .dim = true},
        {.kind = tuinator::StatusSegmentKind::Text, .text = "inline", .dim = true},
    });
    status->set_center({
        {.kind = tuinator::StatusSegmentKind::Text,
         .text = state.current,
         .foreground_rgb = tuinator::Rgb::hex(0xCDD6F4)},
    });
    status->set_right({
        {.kind = tuinator::StatusSegmentKind::Pill,
         .text = std::to_string(state.progress) + "%",
         .foreground_rgb = tuinator::Rgb::hex(0x1E1E2E),
         .background_rgb = tuinator::Rgb::hex(0xA6E3A1)},
    });
    ui.status = status.get();
    root->add_child(std::move(status));

    return root;
}

void refresh_ui(TaskState& state, DemoUi& ui) {
    ui.list->set_items(state.completed);

    const std::string spinner =
        ui.status_throbber != nullptr ? ui.status_throbber->frame() : " ";
    ui.status->set_left({
        {.kind = tuinator::StatusSegmentKind::Text,
         .text = spinner,
         .foreground_rgb = tuinator::Rgb::hex(0xF9E2AF),
         .bold = true},
        {.kind = tuinator::StatusSegmentKind::Text,
         .text = "|",
         .foreground_rgb = tuinator::Rgb::hex(0x585B70),
         .dim = true},
        {.kind = tuinator::StatusSegmentKind::Text, .text = "Tuinator", .bold = true},
        {.kind = tuinator::StatusSegmentKind::Text,
         .text = "|",
         .foreground_rgb = tuinator::Rgb::hex(0x585B70),
         .dim = true},
        {.kind = tuinator::StatusSegmentKind::Text, .text = "inline", .dim = true},
    });
    ui.status->set_center({
        {.kind = tuinator::StatusSegmentKind::Text,
         .text = state.current,
         .foreground_rgb = tuinator::Rgb::hex(0xCDD6F4)},
    });
    ui.status->set_right({
        {.kind = tuinator::StatusSegmentKind::Pill,
         .text = state.done ? "done" : std::to_string(state.progress) + "%",
         .foreground_rgb = tuinator::Rgb::hex(0x1E1E2E),
         .background_rgb = state.done ? tuinator::Rgb::hex(0x89B4FA) : tuinator::Rgb::hex(0xA6E3A1)},
    });
}

void tick_throbbers(DemoUi& ui, int dt_ms) {
    for (tuinator::Throbber* throbber : ui.showcase_throbbers) {
        throbber->tick(dt_ms);
    }
}

} // namespace

int main() {
    std::cout << "Tuinator inline demo — simulating ESLint-style task progress\n";
    std::cout << "Normal CLI output stays above the live TUI band.\n\n";
    std::cout.flush();

    const std::vector<std::string> files = {
        "src/core/application.cpp",
        "src/backend/curses_backend.cpp",
        "src/widgets/controls/button.cpp",
        "src/render/canvas.cpp",
        "include/tuinator/tuinator.hpp",
        "examples/counter.cpp",
        "tests/core/test_memory_backend.cpp",
    };

    TaskState state;
    state.current = "Linting " + files.front() + "…";
    DemoUi ui;

    tuinator::InlineView view({.height = 10, .clear_on_shutdown = false});
    view.set_root(build_ui(state, ui));
    view.present();

    std::size_t file_index = 0;
    int progress_ticks = 0;

    view.app().set_interval(50, [&]() {
        tick_throbbers(ui, 50);

        if (!state.done) {
            refresh_ui(state, ui);
        }

        if (state.done) {
            if (++progress_ticks >= 20) {
                view.app().quit();
            }
            return;
        }

        if (++progress_ticks % 3 != 0) {
            return;
        }

        if (state.progress < 100) {
            state.progress += 8;
            if (state.progress > 100) {
                state.progress = 100;
            }
            return;
        }

        if (file_index < files.size()) {
            state.completed.push_back("✓  " + files[file_index]);
            ++file_index;
            state.current = file_index < files.size() ? "Linting " + files[file_index] + "…"
                                                      : "Finishing…";
            state.progress = 0;
            return;
        }

        state.current = "All checks passed";
        state.done = true;
    });

    const int code = view.run();
    view.finish();

    std::cout << "\nFinished with exit code " << code << ".\n";
    return code;
}
