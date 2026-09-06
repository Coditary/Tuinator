#include <tuinator/tuinator.hpp>

#include <cmath>
#include <memory>
#include <string>
#include <vector>

namespace {

struct ChartState {
    tuinator::PieChart* chart = nullptr;
    std::vector<double> base_values;
    std::vector<double> phase;
};

class PieChartRoot : public tuinator::VBox {
  public:
    PieChartRoot(tuinator::BoxOptions options, tuinator::Application* app, tuinator::Label* mode_label,
                 tuinator::StatusBar* status)
        : tuinator::VBox(options), app_(app), mode_label_(mode_label), status_(status) {}

    bool wants_full_screen() const override { return true; }

    void set_charts(std::vector<ChartState*> charts) { charts_ = std::move(charts); }

    void set_style_cycle(std::vector<tuinator::PieChartStyle> styles) {
        styles_ = std::move(styles);
        style_index_ = 0;
    }

    bool handle_event(const tuinator::Event& event) override {
        const auto* key = std::get_if<tuinator::KeyPress>(&event);
        if (key != nullptr) {
            if (key->key == tuinator::Key::Enter || key->character == ' ' || key->character == 'h' ||
                key->character == 'H') {
                cycle_style();
                return true;
            }

            if (key->character >= '1' && key->character <= '4') {
                const int index = key->character - '1';
                if (index >= 0 && index < static_cast<int>(charts_.size()) &&
                    charts_[static_cast<std::size_t>(index)] != nullptr &&
                    charts_[static_cast<std::size_t>(index)]->chart != nullptr) {
                    charts_[static_cast<std::size_t>(index)]->chart->set_focused(true);
                    if (app_ != nullptr) {
                        app_->refresh_focus();
                    }
                    if (status_ != nullptr) {
                        status_->set_text("Focused chart " + std::to_string(index + 1) + " • Left/Right adjust slice");
                    }
                }
                return true;
            }
        }

        return tuinator::VBox::handle_event(event);
    }

  private:
    void cycle_style() {
        if (styles_.empty()) {
            return;
        }

        style_index_ = (style_index_ + 1) % styles_.size();
        const tuinator::PieChartStyle style = styles_[style_index_];

        for (ChartState* state : charts_) {
            if (state == nullptr || state->chart == nullptr) {
                continue;
            }
            tuinator::PieChartOptions options = state->chart->options();
            options.style = style;
            options.diameter = style == tuinator::PieChartStyle::Braille ? 16 : 14;
            state->chart->set_options(options);
        }

        if (mode_label_ != nullptr) {
            mode_label_->set_text("Current style: " + std::string(style_title(style)));
        }
        if (status_ != nullptr) {
            status_->set_text("Style: " + std::string(style_title(style)) + " • drag slice edges to resize");
        }
    }

    static const char* style_title(tuinator::PieChartStyle style) {
        for (const tuinator::PieChartStyleInfo& info : tuinator::all_pie_chart_styles()) {
            if (info.style == style) {
                return info.title;
            }
        }
        return "Custom";
    }

    tuinator::Application* app_ = nullptr;
    tuinator::Label* mode_label_ = nullptr;
    tuinator::StatusBar* status_ = nullptr;
    std::vector<ChartState*> charts_;
    std::vector<tuinator::PieChartStyle> styles_;
    std::size_t style_index_ = 0;
};

tuinator::PieChartOptions chart_options(const tuinator::Theme& theme, tuinator::PieChartStyle style) {
    tuinator::PieChartOptions options{};
    options.title_style = theme.heading;
    options.legend_style = theme.label;
    options.style = style;
    options.interactive = true;
    options.diameter = style == tuinator::PieChartStyle::Braille ? 16 : 14;
    options.show_legend = true;
    options.show_percent = true;
    return options;
}

std::vector<tuinator::PieChartSlice> make_slices(const std::vector<std::pair<std::string, double>>& items,
                                                 const std::vector<tuinator::Style>& colors) {
    std::vector<tuinator::PieChartSlice> slices;
    slices.reserve(items.size());
    for (std::size_t i = 0; i < items.size(); ++i) {
        slices.push_back({
            items[i].first,
            items[i].second,
            colors[i % colors.size()],
        });
    }
    return slices;
}

std::unique_ptr<tuinator::PieChart> make_chart(const tuinator::Theme& theme, const std::string& title,
                                               tuinator::PieChartStyle style,
                                               const std::vector<std::pair<std::string, double>>& items,
                                               const std::vector<tuinator::Style>& colors, ChartState* state) {
    auto options = chart_options(theme, style);
    options.title = title;
    auto chart = std::make_unique<tuinator::PieChart>(make_slices(items, colors), options);

    state->chart = chart.get();
    state->base_values.clear();
    state->phase.clear();
    for (const auto& item : items) {
        state->base_values.push_back(item.second);
        state->phase.push_back(static_cast<double>(state->base_values.size()) * 0.7);
    }

    return chart;
}

void animate_charts(const std::vector<ChartState*>& charts, double tick) {
    for (ChartState* state : charts) {
        if (state == nullptr || state->chart == nullptr) {
            continue;
        }

        auto slices = state->chart->slices();
        for (std::size_t i = 0; i < slices.size(); ++i) {
            const double base = state->base_values[i];
            const double wave = std::sin(tick * 0.04 + state->phase[i]);
            slices[i].value = std::max(1.0, base + wave * (base * 0.18));
        }
        state->chart->set_slices(std::move(slices));
    }
}

} // namespace

int main() {
    tuinator::Application app;
    const tuinator::Theme theme = tuinator::dark_theme();

    auto desktop = std::make_unique<tuinator::Desktop>();
    auto status = std::make_unique<tuinator::StatusBar>(
        "Drag slice edges to adjust • Space/H = cycle style • 1-4 = focus chart", theme.muted);
    auto* status_ptr = status.get();

    auto mode_label = std::make_unique<tuinator::Label>("Current style: Thick dots", theme.accent);
    auto* mode_label_ptr = mode_label.get();

    auto root =
        std::make_unique<PieChartRoot>(tuinator::BoxOptions{.gap = 1, .padding = 1}, &app, mode_label_ptr, status_ptr);

    root->add_child(std::make_unique<tuinator::Label>("Pie Chart Demo (Animated + Interactive)", theme.heading));
    root->add_child(std::make_unique<tuinator::Label>(
        "Thick/fine dots, stars, blocks, or braille. Drag a slice edge to resize values. "
        "Space/Enter/H = cycle style • 1-4 = focus chart • q = quit",
        theme.muted));
    root->add_child(std::move(mode_label));

    const std::vector<tuinator::Style> palette = {
        tuinator::Style{.foreground = tuinator::Color::Red, .bold = true},
        tuinator::Style{.foreground = tuinator::Color::Blue, .bold = true},
        tuinator::Style{.foreground = tuinator::Color::Green, .bold = true},
        tuinator::Style{.foreground = tuinator::Color::Yellow, .bold = true},
        tuinator::Style{.foreground = tuinator::Color::Magenta, .bold = true},
        tuinator::Style{.foreground = tuinator::Color::Cyan, .bold = true},
        tuinator::Style{.foreground = tuinator::Color::White, .dim = true},
    };

    ChartState languages{};
    ChartState market{};
    ChartState time{};
    ChartState budget{};

    auto grid = std::make_unique<tuinator::Grid>(tuinator::GridOptions{.columns = 2, .gap = 2});
    grid->add_child(make_chart(theme, "Languages", tuinator::PieChartStyle::Dots,
                               {{"Rust", 44.0}, {"Go", 34.0}, {"Python", 22.0}}, palette, &languages));
    grid->add_child(make_chart(theme, "Market Share", tuinator::PieChartStyle::Dots,
                               {{"Product A", 35.0}, {"Product B", 16.0}, {"Product C", 49.0}},
                               {palette[4], palette[3], palette[5]}, &market));
    grid->add_child(make_chart(theme, "Time Allocation", tuinator::PieChartStyle::Dots,
                               {{"Work", 52.0}, {"Sleep", 20.0}, {"Leisure", 28.0}},
                               {palette[1], palette[4], palette[2]}, &time));
    grid->add_child(make_chart(theme, "Budget", tuinator::PieChartStyle::Dots,
                               {{"Housing", 38.0}, {"Food", 18.0}, {"Transport", 12.0}, {"Other", 32.0}},
                               {palette[0], palette[3], palette[5], palette[6]}, &budget));

    grid->set_flex(1);
    root->add_child(std::move(grid));
    root->add_child(std::move(status));

    const std::vector<ChartState*> charts = {&languages, &market, &time, &budget};
    root->set_charts(charts);
    root->set_style_cycle({
        tuinator::PieChartStyle::Dots,
        tuinator::PieChartStyle::FineDots,
        tuinator::PieChartStyle::SmallDots,
        tuinator::PieChartStyle::Stars,
        tuinator::PieChartStyle::Hash,
        tuinator::PieChartStyle::Plus,
        tuinator::PieChartStyle::Blocks,
        tuinator::PieChartStyle::Braille,
    });

    desktop->set_background(std::move(root));
    app.set_root(std::move(desktop));

    double tick = 0.0;
    app.set_interval(80, [&]() {
        tick += 1.0;
        animate_charts(charts, tick);
    });

    return app.run();
}
