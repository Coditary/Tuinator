# Performance tests

Headless benchmarks live in `tests/perf/` and build as `tuinator-perf-tests`. They measure **median wall time** over several iterations and compare against a **budget** (milliseconds).

## Run

```bash
cmake -B build -DTUINATOR_BUILD_TESTS=ON
cmake --build build --target tuinator-perf-tests
./build/tuinator-perf-tests
```

Makefile shortcuts:

| Target | Behavior |
|--------|----------|
| `make perf-test` | Strict budgets (default) |
| `make perf-report` | `TUINATOR_PERF_REPORT_ONLY=1` — log SLOW, do not fail |
| `make perf-ci` | `TUINATOR_PERF_FACTOR=2` — looser budgets for slow runners |

Filter (substring, like unit tests):

```bash
./build/tuinator-perf-tests param_list
./build/tuinator-perf-tests app_driver
```

## Environment

| Variable | Effect |
|----------|--------|
| `TUINATOR_PERF_FACTOR` | Multiplies all budgets (e.g. `2` = twice as slow allowed) |
| `TUINATOR_PERF_REPORT_ONLY=1` | Never fail on budget; print warnings only |
| `TUINATOR_PERF_JSON=/path/lines.jsonl` | Append one JSON object per benchmark line |

## Writing benchmarks

Single case:

```cpp
#include "bench_harness.hpp"

TUINATOR_PERF_TEST(my_case) {
    return tuinator::perf::bench("my_case", 50.0, 2, 20, [&]() {
        // work under measurement
    });
}
```

Parameterized (tables + loops): `tests/support/param_perf.hpp`

```cpp
#include "param_perf.hpp"

struct Row { const char* label; int n; double budget_ms; };
constexpr Row kRows[] = {{"small", 10, 20.0}, {"large", 500, 80.0}};

TUINATOR_PERF_TEST(param_my_suite) {
    bool all_ok = true;
    TUINATOR_PERF_TABLE(my_suite, Row, kRows, 2, 15, {
        // use _tuinator_perf_row.n
    });
    return all_ok;
}
```

Dynamic grid:

```cpp
for (int n : {10, 100, 1000}) {
    TUINATOR_PERF_PARAM_CASE(my_suite, ("n" + std::to_string(n)).c_str(), 5.0 + n * 0.1, 2, 12, {
        // ...
    });
}
```

Examples: `tests/perf/perf_widgets.cpp`, `tests/perf/perf_app_driver.cpp`.

## Tuning budgets

1. Run `make perf-report` locally and note medians.
2. Set budget to ~2–3× median for CI stability, or use `TUINATOR_PERF_FACTOR` in CI only.
3. CTest label `perf` — keep perf tests out of fast pre-commit if desired: `ctest -LE perf`.

## Script

`scripts/perf/run_perf.sh` — report mode, optional JSON export, filter passthrough.
