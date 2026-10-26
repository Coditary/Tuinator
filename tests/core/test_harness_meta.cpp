#include "negative_test.hpp"
#include "test_harness.hpp"

/// Verifies the test harness actually fails when assertions are wrong (guards against silent/no-op checks).
TUINATOR_TEST(meta_harness_detects_failed_check) {
    TUINATOR_EXPECT_CHECK_FAILS({ TUINATOR_CHECK(false); });
}

TUINATOR_TEST(meta_harness_detects_failed_equality) {
    TUINATOR_EXPECT_CHECK_FAILS({ TUINATOR_CHECK_EQ(1, 2); });
}

TUINATOR_TEST(meta_harness_passing_checks_do_not_count_as_failure) {
    TUINATOR_EXPECT_CHECK_PASSES({
        TUINATOR_CHECK(true);
        TUINATOR_CHECK_EQ(3, 3);
    });
}

TUINATOR_TEST(meta_check_false_fails_when_expression_is_true) {
    TUINATOR_EXPECT_CHECK_FAILS({ TUINATOR_CHECK_FALSE(2 + 2 == 4); });
}

TUINATOR_TEST(meta_inverted_expectation_would_fail_on_correct_impl) {
    // If this meta-test passes, a buggy test that expects the wrong outcome would be caught
    // by pairing positive + negative suites (see test_edge_negative.cpp).
    TUINATOR_EXPECT_CHECK_FAILS({
        int value = 5;
        value = 99;
        TUINATOR_CHECK_EQ(value, 5);
    });
}
