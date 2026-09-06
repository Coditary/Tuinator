#include <tuinator/tuinator.hpp>

#include <memory>
#include <string>
#include <vector>

namespace {

class ControlsRoot : public tuinator::VBox {
  public:
    explicit ControlsRoot(tuinator::BoxOptions options) : tuinator::VBox(options) {}
    bool wants_full_screen() const override { return true; }
};

void add_progress_row(tuinator::VBox& content, const tuinator::Theme& theme, const char* caption,
                      tuinator::ProgressBarLayout layout, std::vector<tuinator::ProgressBar*>& bars,
                      double value = 0.35, const tuinator::ProgressBarOptions* custom = nullptr) {
    content.add_child(std::make_unique<tuinator::Label>(caption, theme.muted));

    tuinator::ProgressBarOptions options =
        custom != nullptr ? *custom : tuinator::progress_bar_preset(layout, theme.accent, theme.muted);
    if (layout == tuinator::ProgressBarLayout::Labeled) {
        options.label = "Progress";
    }

    auto bar = std::make_unique<tuinator::ProgressBar>(value, options);
    bars.push_back(bar.get());
    content.add_child(std::move(bar));
}

tuinator::ProgressBarOptions task_row_options(const tuinator::Theme& theme, const std::string& label, double value,
                                              bool completed = false) {
    auto options = tuinator::progress_bar_preset(tuinator::ProgressBarLayout::TaskRow, theme.accent, theme.muted);
    options.label = label;
    options.completed = completed;
    options.stats.total = 100;
    options.stats.rate = value > 0.0 && value < 1.0 ? 12.0 : 0.0;
    options.min_width = 72;
    return options;
}

void add_indeterminate_row(tuinator::VBox& content, const tuinator::Theme& theme, const char* caption,
                           tuinator::ProgressBarLayout layout, std::vector<tuinator::ProgressBar*>& animated_bars) {
    content.add_child(std::make_unique<tuinator::Label>(caption, theme.muted));

    auto options = tuinator::progress_bar_preset(layout, theme.accent, theme.muted);
    auto bar = std::make_unique<tuinator::ProgressBar>(0.0, options);
    animated_bars.push_back(bar.get());
    content.add_child(std::move(bar));
}

tuinator::ProgressBarOptions braille_metric_options(const tuinator::Theme& theme, const std::string& label,
                                                    const std::string& metric, tuinator::Color fill_color) {
    auto options = tuinator::progress_bar_preset(tuinator::ProgressBarLayout::BrailleMetric, theme.accent, theme.muted);
    options.label = label;
    options.metric_value = metric;
    options.fill_style.foreground = fill_color;
    options.fill_style.bold = true;
    options.track_style.dim = true;
    options.head_style.foreground = tuinator::Color::Red;
    options.head_style.bold = true;
    return options;
}

} // namespace

int main() {
    tuinator::Application app;
    const tuinator::Theme theme = app.theme();

    auto status_message = std::make_shared<std::string>("Ready");
    auto status = std::make_unique<tuinator::StatusBar>(*status_message, theme.muted);
    tuinator::StatusBar* status_ptr = status.get();

    auto desktop = std::make_unique<tuinator::Desktop>();
    tuinator::Desktop* desktop_ptr = desktop.get();

    auto actions = std::make_shared<tuinator::ActionRegistry>();
    {
        tuinator::RegisteredAction action;
        action.id = "file.new";
        action.label = "&New File";
        action.category = "File";
        action.shortcut = "Ctrl+N";
        action.hint = "Create a new document";
        action.icon = "*";
        action.callback = [status_ptr, status_message]() {
            *status_message = "Action: New File";
            status_ptr->set_text(*status_message);
        };
        actions->register_action(std::move(action));
    }
    {
        tuinator::RegisteredAction action;
        action.id = "view.palette";
        action.label = "Command Palette";
        action.category = "View";
        action.shortcut = "Ctrl+P";
        action.hint = "Search commands";
        action.callback = [desktop_ptr]() { desktop_ptr->show_command_palette(); };
        actions->register_action(std::move(action));
    }
    desktop_ptr->set_action_registry(actions);
    desktop_ptr->set_context_menu_items({
        [] {
            tuinator::MenuItem item{"Copy", []() {}};
            item.icon = "C";
            item.hint = "Copy selection";
            return item;
        }(),
        [] {
            tuinator::MenuItem item{"Paste", []() {}};
            item.icon = "P";
            item.hint = "Paste from clipboard";
            return item;
        }(),
        tuinator::MenuItem::separator(),
        tuinator::MenuItem::submenu("More",
                                    {
                                        {"Refresh", []() {}},
                                        {"Inspect", []() {}},
                                    }),
    });
    std::vector<tuinator::CommandPaletteEntry> palette_entries;
    for (const tuinator::RegisteredAction* action : actions->all()) {
        palette_entries.push_back({
            action->id,
            tuinator::parse_menu_mnemonic(action->label).text,
            action->category,
            action->shortcut,
            action->callback,
        });
    }
    palette_entries.push_back({
        "demo.quit",
        "Quit",
        "App",
        "Ctrl+Q",
        [&app]() { app.quit(); },
    });
    desktop_ptr->set_command_palette_entries(std::move(palette_entries));

    auto menu = std::make_unique<tuinator::MenuBar>(theme.label, theme.button_focused);
    if (const tuinator::MenuBarLook* look = tuinator::menu_bar_look_named("mac")) {
        menu->apply_look(*look);
    } else {
        menu->set_border_glyphs(theme.glyphs);
    }
    menu->set_menus({
        {"&File",
         {
             [] {
                 tuinator::MenuItem item{"&New", []() {}, "Ctrl+N"};
                 item.icon = "*";
                 item.hint = "Create a new document";
                 return item;
             }(),
             tuinator::MenuItem::separator(),
             [] {
                 tuinator::MenuItem item{"&Open...", []() {}, "Ctrl+O"};
                 item.icon = ">";
                 item.hint = "Open an existing file";
                 return item;
             }(),
             tuinator::MenuItem::submenu("&Export",
                                         {
                                             [] {
                                                 tuinator::MenuItem item{"&PDF", []() {}};
                                                 item.icon = "P";
                                                 item.hint = "Export as PDF";
                                                 return item;
                                             }(),
                                             [] {
                                                 tuinator::MenuItem item{"&PNG", []() {}};
                                                 item.icon = "I";
                                                 item.hint = "Export as PNG";
                                                 return item;
                                             }(),
                                         },
                                         {}, ">"),
             tuinator::MenuItem::separator(),
             [&app] {
                 tuinator::MenuItem item{"&Quit", [&app]() { app.quit(); }, "Ctrl+Q"};
                 item.icon = "X";
                 item.hint = "Exit the application";
                 return item;
             }(),
         }},
        {"&View",
         {
             tuinator::MenuItem::checkbox("&Toolbar", true),
             tuinator::MenuItem::checkbox("&Status Bar", true),
             tuinator::MenuItem::separator(),
             [] {
                 tuinator::MenuItem item{"&Zoom In", []() {}, "Ctrl++"};
                 item.icon = "+";
                 item.hint = "Increase zoom level";
                 return item;
             }(),
             [] {
                 tuinator::MenuItem item{"Zoom Out", []() {}, "Ctrl+-", false};
                 item.icon = "-";
                 item.hint = "Decrease zoom level";
                 return item;
             }(),
         }},
    });
    menu->set_on_action([status_ptr, status_message](const std::string& menu_name, const std::string& item) {
        *status_message = menu_name + " -> " + item;
        status_ptr->set_text(*status_message);
    });
    menu->set_on_hint([status_ptr, status_message](const std::string& hint) {
        if (!hint.empty()) {
            *status_message = hint;
            status_ptr->set_text(*status_message);
        }
    });

    std::vector<tuinator::ProgressBar*> progress_bars;
    std::vector<tuinator::ProgressBar*> animated_bars;
    tuinator::ProgressBar* tqdm_bar = nullptr;
    tuinator::ProgressBar* wave_bar = nullptr;
    int elapsed_ms = 0;

    auto slider = std::make_unique<tuinator::Slider>(
        0, 100, 35,
        [status_ptr, status_message, &progress_bars](int value) {
            const double progress = value / 100.0;
            for (tuinator::ProgressBar* bar : progress_bars) {
                bar->set_value(progress);
            }
            *status_message = "Slider: " + std::to_string(value);
            status_ptr->set_text(*status_message);
        },
        theme.label, 28);

    auto spinner = std::make_unique<tuinator::Spinner>(
        0, 10, 3, 1,
        [status_ptr, status_message](int value) {
            *status_message = "Spinner: " + std::to_string(value);
            status_ptr->set_text(*status_message);
        },
        theme.label);

    auto combo = std::make_unique<tuinator::ComboBox>(theme.label, theme.button_focused);
    combo->set_items({"C++", "Rust", "Go", "Java", "Kotlin"});
    combo->set_on_select([status_ptr, status_message](int, const std::string& item) {
        *status_message = "Language: " + item;
        status_ptr->set_text(*status_message);
    });

    tuinator::TreeNode tree_root{
        "Project",
        {
            {"src",
             {
                 {"main.cpp"},
                 {"widgets",
                  {
                      {"button.cpp"},
                      {"tree_view.cpp"},
                  }},
             }},
            {"docs", {{"API.md"}}},
        },
    };

    auto tree = std::make_unique<tuinator::TreeView>(theme.label, theme.button_focused);
    tree->set_root(std::move(tree_root));
    tree->set_on_select([status_ptr, status_message](const std::string& path) {
        *status_message = "Tree: " + path;
        status_ptr->set_text(*status_message);
    });

    auto dialog_button = std::make_unique<tuinator::Button>(
        "Show confirm dialog",
        [desktop_ptr, status_ptr, status_message, &theme]() {
            desktop_ptr->show_modal(
                "Confirm", {20, 6, 42, 9},
                tuinator::dialog::make_confirm(
                    theme, "Save changes before closing?", [desktop_ptr, status_ptr, status_message](bool confirmed) {
                        desktop_ptr->close_window(desktop_ptr->active_window());
                        *status_message = confirmed ? "Dialog: confirmed" : "Dialog: cancelled";
                        status_ptr->set_text(*status_message);
                    }));
        },
        theme.button);

    auto content = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    content->add_child(std::make_unique<tuinator::Label>("JavaFX-style controls", theme.heading));
    content->add_child(std::move(menu));

    auto progress_panel = std::make_unique<tuinator::Panel>("Progress", theme.border, theme.heading, theme.glyphs);
    auto panel_body = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    panel_body->add_child(
        std::make_unique<tuinator::Label>("Determinate — known progress (slider below)", theme.muted));
    {
        auto total = std::make_unique<tuinator::ProgressBar>(
            0.82, tuinator::progress_bar_filled_label(theme.accent, theme.muted, "Total progress",
                                                      tuinator::ProgressBarMode::Determinate));
        progress_bars.push_back(total.get());
        panel_body->add_child(std::move(total));
    }
    panel_body->add_child(
        std::make_unique<tuinator::Label>("Indeterminate — unknown progress (pendulum)", theme.muted));
    {
        auto loading = std::make_unique<tuinator::ProgressBar>(
            0.0, tuinator::progress_bar_filled_label(theme.accent, theme.muted, "Loading...",
                                                     tuinator::ProgressBarMode::Indeterminate));
        animated_bars.push_back(loading.get());
        panel_body->add_child(std::move(loading));
    }
    panel_body->add_child(std::make_unique<tuinator::Label>(
        "Press Esc to quit", tuinator::Style{.foreground = tuinator::Color::White, .dim = true}));
    progress_panel->set_content(std::move(panel_body));
    content->add_child(std::move(progress_panel));

    content->add_child(std::make_unique<tuinator::Label>("Indeterminate loaders", theme.heading));
    add_indeterminate_row(*content, theme, "Pulse line + placeholders", tuinator::ProgressBarLayout::Pulse,
                          animated_bars);
    add_indeterminate_row(*content, theme, "Rich shimmer (Loading...)", tuinator::ProgressBarLayout::Shimmer,
                          animated_bars);
    add_indeterminate_row(*content, theme, "Bounce [=   ]", tuinator::ProgressBarLayout::Bounce, animated_bars);
    add_indeterminate_row(*content, theme, "Sliding blocks", tuinator::ProgressBarLayout::SlideBlock, animated_bars);
    add_indeterminate_row(*content, theme, "Moving dot", tuinator::ProgressBarLayout::MovingDot, animated_bars);

    content->add_child(std::make_unique<tuinator::Label>("Task rows", theme.heading));
    {
        auto row = std::make_unique<tuinator::ProgressBar>(0.25, task_row_options(theme, "Processing...", 0.25));
        progress_bars.push_back(row.get());
        content->add_child(std::move(row));
    }
    {
        auto row = std::make_unique<tuinator::ProgressBar>(0.50, task_row_options(theme, "Processing...", 0.50));
        progress_bars.push_back(row.get());
        content->add_child(std::move(row));
    }
    {
        auto row = std::make_unique<tuinator::ProgressBar>(0.75, task_row_options(theme, "Processing...", 0.75));
        progress_bars.push_back(row.get());
        content->add_child(std::move(row));
    }
    {
        auto row = std::make_unique<tuinator::ProgressBar>(1.0, task_row_options(theme, "Complete Task", 1.0, true));
        content->add_child(std::move(row));
    }

    content->add_child(std::make_unique<tuinator::Label>("tqdm-style", theme.muted));
    {
        auto options = tuinator::progress_bar_preset(tuinator::ProgressBarLayout::Tqdm, theme.label, theme.muted);
        options.stats.total = 100;
        options.stats.rate = 9.6;
        auto bar = std::make_unique<tuinator::ProgressBar>(0.42, options);
        tqdm_bar = bar.get();
        progress_bars.push_back(bar.get());
        content->add_child(std::move(bar));
    }

    content->add_child(std::make_unique<tuinator::Label>("Braille / metric bars", theme.muted));
    {
        auto cpu = std::make_unique<tuinator::ProgressBar>(
            0.72, braille_metric_options(theme, "CPU", "72%", tuinator::Color::Blue));
        progress_bars.push_back(cpu.get());
        content->add_child(std::move(cpu));
    }
    {
        auto memory = std::make_unique<tuinator::ProgressBar>(
            0.58, braille_metric_options(theme, "Memory", "472 MB", tuinator::Color::Green));
        progress_bars.push_back(memory.get());
        content->add_child(std::move(memory));
    }
    {
        auto traffic_options = braille_metric_options(theme, "Traffic", "1452 req/m", tuinator::Color::Blue);
        traffic_options.head_style = traffic_options.fill_style;
        auto traffic = std::make_unique<tuinator::ProgressBar>(0.81, traffic_options);
        progress_bars.push_back(traffic.get());
        content->add_child(std::move(traffic));
    }
    {
        auto wave = std::make_unique<tuinator::ProgressBar>(
            0.0, tuinator::progress_bar_preset(tuinator::ProgressBarLayout::BrailleWave, theme.accent, theme.muted));
        wave_bar = wave.get();
        content->add_child(std::move(wave));
    }

    content->add_child(std::make_unique<tuinator::Label>("Classic layouts", theme.heading));
    add_progress_row(*content, theme, "Plain arrow", tuinator::ProgressBarLayout::Plain, progress_bars);
    add_progress_row(*content, theme, "Bracketed + percent", tuinator::ProgressBarLayout::Bracketed, progress_bars);
    add_progress_row(*content, theme, "Unicode blocks", tuinator::ProgressBarLayout::Blocks, progress_bars);
    add_progress_row(*content, theme, "Dots", tuinator::ProgressBarLayout::Dots, progress_bars);
    add_progress_row(*content, theme, "Pill", tuinator::ProgressBarLayout::Pill, progress_bars);
    add_progress_row(*content, theme, "Labeled", tuinator::ProgressBarLayout::Labeled, progress_bars);
    content->add_child(std::make_unique<tuinator::Label>("Gradient fill (true color)", theme.muted));
    {
        auto options = tuinator::progress_bar_preset(tuinator::ProgressBarLayout::Blocks, theme.accent, theme.muted);
        options.gradient_stops = tuinator::progress_bar_gradient({
            {0.0f, 0x5FB89E},
            {0.5f, 0xB898D0},
            {1.0f, 0xE848A0},
        });
        options.min_width = 40;
        auto bar = std::make_unique<tuinator::ProgressBar>(0.72, options);
        progress_bars.push_back(bar.get());
        content->add_child(std::move(bar));
    }
    content->add_child(std::make_unique<tuinator::Label>("Slider drives all bars above", theme.muted));
    content->add_child(std::move(slider));
    content->add_child(std::make_unique<tuinator::Label>("Spinner (< > / Up Down)", theme.muted));
    content->add_child(std::move(spinner));
    content->add_child(std::make_unique<tuinator::Label>("ComboBox (Enter to open)", theme.muted));
    content->add_child(std::move(combo));
    content->add_child(std::make_unique<tuinator::Label>("TreeView (Left/Right expand)", theme.muted));
    tree->set_flex(1);
    content->add_child(std::move(tree));
    content->add_child(std::move(dialog_button));
    content->add_child(std::make_unique<tuinator::Label>(
        "Tab = focus | Alt+F menus | Ctrl+P palette | Right-click = context menu", theme.muted));

    auto scroll = std::make_unique<tuinator::ScrollView>(std::move(content),
                                                         tuinator::ScrollViewOptions{.width = 80, .height = 20});
    scroll->set_flex(1);

    auto root = std::make_unique<ControlsRoot>(tuinator::BoxOptions{.gap = 1, .padding = 0});
    root->add_child(std::move(scroll));
    root->add_child(std::move(status));

    desktop->set_background(std::move(root));
    app.set_root(std::move(desktop));

    app.set_interval(80, [&progress_bars, &animated_bars, tqdm_bar, wave_bar, &elapsed_ms]() {
        static double value = 0.35;
        value += 0.02;
        if (value > 1.0) {
            value = 0.0;
        }
        elapsed_ms += 80;
        const int phase = elapsed_ms / 80;

        for (tuinator::ProgressBar* bar : progress_bars) {
            bar->set_value(value);
        }

        for (std::size_t i = 0; i < animated_bars.size(); ++i) {
            animated_bars[i]->set_animation_phase(phase + static_cast<int>(i * 8));
        }

        if (tqdm_bar != nullptr) {
            tuinator::ProgressBarStats stats = tqdm_bar->options().stats;
            stats.current = static_cast<int>(value * stats.total + 0.5);
            stats.elapsed_ms = elapsed_ms;
            stats.rate = 8.0 + value * 4.0;
            tqdm_bar->set_stats(stats);
            tqdm_bar->set_value(value);
        }

        if (wave_bar != nullptr) {
            wave_bar->set_animation_phase(phase);
        }
    });

    return app.run();
}
