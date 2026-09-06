#pragma once

#include <cstdlib>
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

inline int run_all() {
    failures().clear();

    int ran = 0;
    for (const TestCase& test : registry()) {
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

    std::cout << "\nResult: " << ran << " passed, 0 failed\n";
    return 0;
}

} // namespace tuinator::test
