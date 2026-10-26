#pragma once

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace tuinator::test {

struct Failure {
    std::string message;
};

inline std::vector<Failure>& failures() {
    static std::vector<Failure> items;
    return items;
}

inline void check(bool condition, const char* expr, const char* file, int line) {
    if (!condition) {
        failures().push_back({std::string(file) + ":" + std::to_string(line) + ": " + expr});
    }
}

#define TUINATOR_CHECK(expr) ::tuinator::test::check(static_cast<bool>(expr), #expr, __FILE__, __LINE__)

#define TUINATOR_CHECK_EQ(a, b) ::tuinator::test::check((a) == (b), #a " == " #b, __FILE__, __LINE__)

struct TestCase {
    const char* name;
    void (*run)();
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> cases;
    return cases;
}

struct Registrar {
    Registrar(const char* name, void (*run)()) { registry().push_back({name, run}); }
};

#define TUINATOR_TEST(name)                                                                                            \
    void tuinator_test_##name();                                                                                       \
    static ::tuinator::test::Registrar tuinator_registrar_##name(#name, tuinator_test_##name);                         \
    void tuinator_test_##name()

inline bool matches_filter(const char* test_name, const char* filter) {
    if (filter == nullptr || filter[0] == '\0') {
        return true;
    }
    return std::strstr(test_name, filter) != nullptr;
}

inline int run_all(int argc = 0, char** argv = nullptr) {
    const char* filter = (argc > 1 && argv != nullptr) ? argv[1] : nullptr;
    failures().clear();

    int ran = 0;
    int skipped = 0;
    for (const TestCase& test : registry()) {
        if (!matches_filter(test.name, filter)) {
            ++skipped;
            continue;
        }

        std::cout << "[....] " << test.name << ' ';
        failures().clear();
        test.run();
        if (failures().empty()) {
            std::cout << "OK\n";
            ++ran;
        } else {
            std::cout << "FAIL\n";
            for (const Failure& failure : failures()) {
                std::cerr << "       " << failure.message << '\n';
            }
            return 1;
        }
    }

    if (filter != nullptr && filter[0] != '\0' && ran == 0 && skipped > 0) {
        std::cerr << "No tests matched filter: " << filter << '\n';
        return 1;
    }

    std::cout << "\nResult: " << ran << " passed, 0 failed";
    if (filter != nullptr && filter[0] != '\0') {
        std::cout << " (filter: " << filter << ")";
    }
    std::cout << '\n';
    return 0;
}

/// MuCPP mode: run every registered test, write one line per test (`0` = pass, `1` = fail) to `output_path`.
/// Does not stop at the first failure (mutants are scored against the full suite).
inline int run_all_mucpp(const char* output_path) {
    std::ofstream out(output_path, std::ios::trunc);
    if (!out) {
        std::cerr << "Failed to open MuCPP output file: " << output_path << '\n';
        return 1;
    }

    int failed_tests = 0;
    for (const TestCase& test : registry()) {
        failures().clear();
        test.run();
        const bool passed = failures().empty();
        out << (passed ? '0' : '1') << '\n';
        if (!passed) {
            ++failed_tests;
            for (const Failure& failure : failures()) {
                std::cerr << "       [" << test.name << "] " << failure.message << '\n';
            }
        }
    }

    out.close();
    std::cout << "MuCPP: wrote " << registry().size() << " test results to " << output_path;
    if (failed_tests > 0) {
        std::cout << " (" << failed_tests << " failed)";
    }
    std::cout << '\n';
    return failed_tests > 0 ? 1 : 0;
}

} // namespace tuinator::test
