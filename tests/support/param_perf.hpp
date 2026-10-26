#pragma once

#include "bench_harness.hpp"

#include <string>

namespace tuinator::perf::param {

inline std::string qualified_name(const char* suite, const char* case_name) {
    return std::string(suite) + "/" + case_name;
}

inline std::string qualified_name(const char* suite, const std::string& case_name) {
    return std::string(suite) + "/" + case_name;
}

inline bool bench_case(const char* suite, const char* case_name, double budget_ms, int warmup, int iterations,
                       const std::function<void()>& fn) {
    return bench(qualified_name(suite, case_name).c_str(), budget_ms, warmup, iterations, fn);
}

inline bool bench_case(const char* suite, const std::string& case_name, double budget_ms, int warmup, int iterations,
                       const std::function<void()>& fn) {
    return bench(qualified_name(suite, case_name).c_str(), budget_ms, warmup, iterations, fn);
}

} // namespace tuinator::perf::param

/// Run `body` under a scoped benchmark named `suite`/`case_name`.
#define TUINATOR_PERF_PARAM_CASE(suite, case_name, budget_ms, warmup, iterations, body)                                \
    {                                                                                                                  \
        all_ok = ::tuinator::perf::param::bench_case(#suite, case_name, budget_ms, warmup, iterations,                 \
                                                     [&]() { body; }) &&                                               \
                 all_ok;                                                                                               \
    }

/// Each row needs: `label`, `budget_ms`, plus fields used in `body` (via `_tuinator_perf_row`).
#define TUINATOR_PERF_TABLE(suite, CaseType, cases, warmup, iterations, body)                                          \
    for (const CaseType& _tuinator_perf_row : cases) {                                                                 \
        TUINATOR_PERF_PARAM_CASE(suite, _tuinator_perf_row.label, _tuinator_perf_row.budget_ms, warmup, iterations,    \
                                 body);                                                                                \
    }
