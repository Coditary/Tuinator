#pragma once

#include "test_harness.hpp"

#include <string>
#include <utility>

namespace tuinator::test::param {

/// Prefixes failures raised while this scope is active with `[case_name]`.
class CaseScope {
  public:
    explicit CaseScope(std::string name) : name_(std::move(name)), failure_start_(failures().size()) {}
    ~CaseScope() {
        for (std::size_t i = failure_start_; i < failures().size(); ++i) {
            failures()[i].message = "[" + name_ + "] " + failures()[i].message;
        }
    }

    CaseScope(const CaseScope&) = delete;
    CaseScope& operator=(const CaseScope&) = delete;

  private:
    std::string name_;
    std::size_t failure_start_;
};

inline std::string make_case_name(const char* name) { return name; }
inline std::string make_case_name(std::string name) { return name; }

/// Registers a standalone test case (shows up as its own line in `tuinator-tests` output).
inline bool register_case(const char* qualified_name, void (*run)()) {
    registry().push_back({qualified_name, run});
    return true;
}

} // namespace tuinator::test::param

/// Run `body` with failures attributed to `case_name` (`const char*` or `std::string`).
#define TUINATOR_PARAM_CASE(case_name, body)                                                                           \
    do {                                                                                                               \
        ::tuinator::test::param::CaseScope _tuinator_param_scope(                                                      \
            ::tuinator::test::param::make_case_name(case_name));                                                       \
        body;                                                                                                          \
    } while (0)

/// Iterate a container of rows; each element must expose `.label` (`const char*`).
#define TUINATOR_PARAM_TABLE(suite, CaseType, cases, body)                                                             \
    for (const CaseType& _tuinator_param_row : cases) {                                                                \
        TUINATOR_PARAM_CASE(_tuinator_param_row.label, body);                                                          \
    }

/// One row in the test registry: `suite` / `case_id` (add cases without new TUINATOR_TEST boilerplate).
#define TUINATOR_PARAM_TEST(suite, case_id, body)                                                                      \
    static void tuinator_param_body_##suite##_##case_id();                                                               \
    static const bool tuinator_param_registered_##suite##_##case_id =                                                  \
        ::tuinator::test::param::register_case(#suite "/" #case_id, tuinator_param_body_##suite##_##case_id);          \
    static void tuinator_param_body_##suite##_##case_id() { body; }
