#include <tuinator/debug/startup_profiler.hpp>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <tuple>
#include <vector>

namespace tuinator {

namespace {

std::size_t now_ns() {
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<std::size_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(now).count());
}

} // namespace

StartupProfiler& StartupProfiler::instance() {
    static StartupProfiler profiler;
    return profiler;
}

void StartupProfiler::reset() {
    entries_.clear();
    start_ns_ = now_ns();
    last_ns_ = start_ns_;
}

void StartupProfiler::set_enabled(bool enabled) {
    enabled_ = enabled;
    if (enabled_) {
        reset();
    }
}

void StartupProfiler::mark(std::string label) {
    if (!enabled_) {
        return;
    }

    const std::size_t current = now_ns();
    const double total_ms = static_cast<double>(current - start_ns_) / 1'000'000.0;
    const double delta_ms = static_cast<double>(current - last_ns_) / 1'000'000.0;
    last_ns_ = current;

    entries_.emplace_back(std::move(label), total_ms, delta_ms);
}

void StartupProfiler::write_report(const std::string& path) const {
    std::ofstream out(path);
    if (!out) {
        return;
    }

    out << std::fixed << std::setprecision(3);
    out << "Tuinator startup profile\n";
    out << "label                          total_ms  delta_ms\n";
    out << "---------------------------------------------------\n";

    for (const auto& entry : entries_) {
        out << std::get<0>(entry) << std::string(31 - std::min<std::size_t>(31, std::get<0>(entry).size()), ' ')
            << std::setw(8) << std::get<1>(entry) << "  "
            << std::setw(8) << std::get<2>(entry) << '\n';
    }
}

void startup_profile_mark(std::string label) {
    StartupProfiler::instance().mark(std::move(label));
}

} // namespace tuinator
