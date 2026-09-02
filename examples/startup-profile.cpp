#include <tuinator/debug/startup_profiler.hpp>
#include <tuinator/tuinator.hpp>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

namespace {

std::string report_path() {
    if (const char* path = std::getenv("TUINATOR_PROFILE_OUT")) {
        return path;
    }
    return "startup-profile.log";
}

bool quick_exit() {
    return std::getenv("TUINATOR_PROFILE_QUICK") != nullptr;
}

} // namespace

int main() {
    tuinator::StartupProfiler::instance().set_enabled(true);
    tuinator::startup_profile_mark("main.start");

    tuinator::Application app;
    tuinator::startup_profile_mark("main.after_application_ctor");

    auto root = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    root->add_child(std::make_unique<tuinator::Label>(
        "Startup profile",
        tuinator::Style{.foreground = tuinator::Color::Cyan, .bold = true}));
    root->add_child(std::make_unique<tuinator::Label>("Measuring startup latency..."));
    root->add_child(std::make_unique<tuinator::Label>(
        "See startup-profile.log after exit",
        tuinator::Style{.foreground = tuinator::Color::Yellow, .dim = true}));

    app.set_root(std::move(root));
    tuinator::startup_profile_mark("main.after_set_root");

    if (quick_exit()) {
        app.present();
        tuinator::startup_profile_mark("main.after_first_present");
        tuinator::StartupProfiler::instance().write_report(report_path());
        std::cerr << "Quick profile written to: " << report_path() << '\n';
        return 0;
    }

    std::cerr << "Profile will be written on exit to: " << report_path() << '\n';
    std::cerr << "Press q to quit. (Or run: TUINATOR_PROFILE_QUICK=1 make profile)\n";

    const int code = app.run();
    tuinator::StartupProfiler::instance().write_report(report_path());
    return code;
}
