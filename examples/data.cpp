#include <tuinator/tuinator.hpp>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct Dataset {
    std::string name;
    std::vector<std::vector<std::string>> rows;
};

std::vector<Dataset> sample_data() {
    return {
        {"Alpha",
         {
             {"Alpha-1", "42", "ok"},
             {"Alpha-2", "17", "warn"},
             {"Alpha-3", "99", "ok"},
         }},
        {"Beta",
         {
             {"Beta-1", "3", "fail"},
             {"Beta-2", "8", "ok"},
         }},
        {"Gamma",
         {
             {"Gamma-1", "55", "ok"},
             {"Gamma-2", "12", "ok"},
             {"Gamma-3", "7", "warn"},
             {"Gamma-4", "31", "ok"},
         }},
        {"Delta",
         {
             {"Delta-1", "0", "idle"},
         }},
    };
}

class DataRoot : public tuinator::VBox {
  public:
    explicit DataRoot(tuinator::BoxOptions options) : tuinator::VBox(options) {}

    bool wants_full_screen() const override { return true; }
};

std::unique_ptr<tuinator::Widget> wrap_panel(const std::string& title, const tuinator::Theme& theme,
                                             std::unique_ptr<tuinator::Widget> content) {
    auto panel = std::make_unique<tuinator::Panel>(title, theme.border, theme.heading);
    panel->set_content(std::move(content));
    panel->set_flex(1);
    return panel;
}

} // namespace

int main() {
    tuinator::Application app;
    const tuinator::Theme theme = tuinator::dark_theme();

    const auto datasets = sample_data();
    std::vector<std::string> names;
    names.reserve(datasets.size());
    for (const Dataset& entry : datasets) {
        names.push_back(entry.name);
    }

    auto status_message = std::make_shared<std::string>("Dataset: Alpha");
    auto status = std::make_unique<tuinator::StatusBar>(*status_message, theme.muted);
    tuinator::StatusBar* status_ptr = status.get();

    auto list = std::make_unique<tuinator::ListView>(theme.label, theme.button_focused);
    list->set_items(names);

    auto table = std::make_unique<tuinator::Table>(theme.heading, theme.label, theme.button_focused);
    table->set_columns({
        {"Name", 12},
        {"Value", 8},
        {"State", 8},
    });

    tuinator::Table* table_ptr = table.get();

    auto show_dataset = [&](int index) {
        if (index < 0 || index >= static_cast<int>(datasets.size())) {
            return;
        }

        const Dataset& dataset = datasets[static_cast<std::size_t>(index)];
        table_ptr->set_rows(dataset.rows);
        *status_message = "Dataset: " + dataset.name;
        status_ptr->set_text(*status_message);
    };

    list->set_on_select([&](int index, const std::string& name) {
        show_dataset(index);
        *status_message = "Dataset: " + name;
        status_ptr->set_text(*status_message);
    });

    list->set_on_activate([&](int index, const std::string& name) {
        show_dataset(index);
        *status_message = "Pinned: " + name;
        status_ptr->set_text(*status_message);
    });

    table->set_on_activate([&](int row, const std::vector<std::string>& cells) {
        if (cells.empty()) {
            return;
        }
        *status_message = "Row " + std::to_string(row + 1) + ": " + cells.front();
        status_ptr->set_text(*status_message);
    });

    show_dataset(0);

    auto split = std::make_unique<tuinator::SplitPane>(wrap_panel("Datasets", theme, std::move(list)),
                                                       wrap_panel("Rows", theme, std::move(table)),
                                                       tuinator::SplitPaneOptions{
                                                           .orientation = tuinator::SplitOrientation::Horizontal,
                                                           .first_size = 24,
                                                       });
    split->set_flex(1);

    auto root = std::make_unique<DataRoot>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    root->add_child(std::make_unique<tuinator::Label>("Tuinator Data Explorer", theme.heading));
    root->add_child(std::make_unique<tuinator::Label>(
        "Click or Up/Down on the left list to load rows | Tab switches list/table | Enter confirms", theme.muted));
    root->add_child(std::move(split));
    root->add_child(std::move(status));

    app.set_root(std::move(root));

    app.set_interval(1000, [status_ptr, status_message]() {
        const auto now = std::chrono::system_clock::now();
        const std::time_t time = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm{};
#if defined(_WIN32)
        localtime_s(&local_tm, &time);
#else
        localtime_r(&time, &local_tm);
#endif
        std::ostringstream clock;
        clock << std::put_time(&local_tm, "%H:%M:%S");
        status_ptr->set_text(*status_message + "  |  " + clock.str());
    });

    return app.run();
}
