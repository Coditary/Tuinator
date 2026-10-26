#pragma once

#include "test_harness.hpp"

#include <cstddef>
#include <functional>

namespace tuinator::test {

/// Runs `fn` and returns whether any `TUINATOR_CHECK` failed inside it.
/// Recorded failures are removed so the outer test is not failed (use for meta-tests).
inline bool capture_failures(const std::function<void()>& fn) {
    const std::size_t start = failures().size();
    fn();
    const bool had_failure = failures().size() > start;
    if (had_failure) {
        failures().erase(failures().begin() + static_cast<std::ptrdiff_t>(start), failures().end());
    }
    return had_failure;
}

} // namespace tuinator::test

/// Assert the expression is false (negative / contrapositive expectation).
#define TUINATOR_CHECK_FALSE(expr) TUINATOR_CHECK(!(expr))

/// Expects that checks inside `body` fail (proves the harness catches bad state).
#define TUINATOR_EXPECT_CHECK_FAILS(body) TUINATOR_CHECK(::tuinator::test::capture_failures([&]() { body; }))

/// Expects that checks inside `body` all pass (proves a positive assumption still holds).
#define TUINATOR_EXPECT_CHECK_PASSES(body) TUINATOR_CHECK(!::tuinator::test::capture_failures([&]() { body; }))
