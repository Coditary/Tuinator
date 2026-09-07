#pragma once

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace tuinator::perf {

struct Case {
    const char* name;
    bool (*run)();
};

inline std::vector<Case>& registry() {
    static std::vector<Case> cases;
    return cases;
}

struct Registrar {
    Registrar(const char* name, bool (*run)()) { registry().push_back({name, run}); }
};

inline double env_factor() {
    const char* raw = std::getenv("TUINATOR_PERF_FACTOR");
    if (!raw || !*raw) {
        return 1.0;
    }
    const double value = std::strtod(raw, nullptr);
    return value > 0.0 ? value : 1.0;
}

inline bool report_only() {
    const char* raw = std::getenv("TUINATOR_PERF_REPORT_ONLY");
    return raw && raw[0] == '1' && raw[1] == '\0';
}

inline double measure_median_ms(const std::function<void()>& fn, int warmup, int iterations) {
    for (int i = 0; i < warmup; ++i) {
        fn();
    }

    std::vector<double> samples;
    samples.reserve(static_cast<std::size_t>(iterations));
    for (int i = 0; i < iterations; ++i) {
        const auto start = std::chrono::steady_clock::now();
        fn();
        const auto end = std::chrono::steady_clock::now();
        const double ns =
            static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
        samples.push_back(ns / 1'000'000.0);
    }

    std::sort(samples.begin(), samples.end());
    return samples[static_cast<std::size_t>(samples.size() / 2)];
}

inline bool check_budget(const char* name, double median_ms, double budget_ms) {
    const double effective_budget = budget_ms * env_factor();
    const bool within_budget = median_ms <= effective_budget;
    const char* status = within_budget ? "OK" : "SLOW";

    std::cout << "[perf] " << name << ": " << median_ms << " ms (budget " << effective_budget << " ms) " << status
              << '\n';

    if (within_budget) {
        return true;
    }

    if (report_only()) {
        std::cerr << "       warning: exceeded budget by " << (median_ms - effective_budget) << " ms (report-only)\n";
        return true;
    }

    std::cerr << "       exceeded budget by " << (median_ms - effective_budget) << " ms\n";
    return false;
}

template <typename Fn> inline bool bench(const char* name, double budget_ms, int warmup, int iterations, Fn&& fn) {
    const double median_ms = measure_median_ms(std::function<void()>(fn), warmup, iterations);
    return check_budget(name, median_ms, budget_ms);
}

#define TUINATOR_PERF_TEST(name)                                                                                       \
    bool tuinator_perf_body_##name();                                                                                  \
    bool tuinator_perf_##name() { return tuinator_perf_body_##name(); }                                                \
    static ::tuinator::perf::Registrar tuinator_perf_registrar_##name(#name, tuinator_perf_##name);                    \
    bool tuinator_perf_body_##name()

inline int run_all() {
    int passed = 0;
    int failed = 0;

    std::cout << "Tuinator performance tests\n";
    if (report_only()) {
        std::cout << "Mode: report-only (budget violations do not fail)\n";
    } else {
        std::cout << "Mode: strict (budget violations fail the run)\n";
    }
    const double factor = env_factor();
    if (factor != 1.0) {
        std::cout << "Budget factor: " << factor << " (TUINATOR_PERF_FACTOR)\n";
    }
    std::cout << '\n';

    for (const Case& test : registry()) {
        if (test.run()) {
            ++passed;
        } else {
            std::cerr << "[perf] " << test.name << " failed\n";
            ++failed;
        }
    }

    std::cout << "\nResult: " << passed << " passed";
    if (failed > 0) {
        std::cout << ", " << failed << " failed";
    }
    std::cout << '\n';

    return failed == 0 ? 0 : 1;
}

} // namespace tuinator::perf
