#include <tuinator/render/text.hpp>

#include <string>
#include <string_view>

#include "test_harness.hpp"

TUINATOR_TEST(utf8_from_codepoint_euro) { TUINATOR_CHECK_EQ(tuinator::utf8_from_codepoint(0x20AC), "\xE2\x82\xAC"); }

TUINATOR_TEST(text_display_width_ascii) {
    TUINATOR_CHECK_EQ(tuinator::text_display_width("hello"), 5);
    TUINATOR_CHECK_EQ(tuinator::text_display_width("Tuinator Data Explorer"), 22);
}

TUINATOR_TEST(text_byte_length_matches_width) {
    const std::string text = "Tuinator Data Explorer";
    TUINATOR_CHECK_EQ(tuinator::text_byte_length_for_width(text, 80), static_cast<std::size_t>(text.size()));
}

TUINATOR_TEST(text_string_view_from_buffer) {
    const std::string text = "Tuinator Data Explorer";
    const std::string_view view(text.data(), text.size());
    TUINATOR_CHECK_EQ(tuinator::text_display_width(view), 22);
    TUINATOR_CHECK_EQ(tuinator::text_byte_length_for_width(view, 70), static_cast<std::size_t>(text.size()));
}

TUINATOR_TEST(text_caret_index_at_column_ascii) {
    const std::string_view text = "hello";
    TUINATOR_CHECK_EQ(tuinator::text_caret_index_at_column(text, 0, tuinator::CaretAffinity::Before), 0U);
    TUINATOR_CHECK_EQ(tuinator::text_caret_index_at_column(text, 4, tuinator::CaretAffinity::Before), 4U);
    TUINATOR_CHECK_EQ(tuinator::text_caret_index_at_column(text, 4, tuinator::CaretAffinity::After), 5U);
}

TUINATOR_TEST(text_display_width_emoji_and_variation_selector) {
    TUINATOR_CHECK_EQ(tuinator::text_display_width("⠋"), 1);
    TUINATOR_CHECK_EQ(tuinator::text_display_width("☀️"), 2);
    TUINATOR_CHECK_EQ(tuinator::text_display_width("❤️"), 2);
    TUINATOR_CHECK_EQ(tuinator::text_display_width("🕛"), 2);
}

TUINATOR_TEST(text_substr_view_must_not_use_temp_string) {
    std::string text = "Tuinator Data Explorer";
    const std::string_view good(text.data(), text.size());
    TUINATOR_CHECK_EQ(tuinator::text_display_width(good), 22);

    // std::string::substr returns a temporary; binding to string_view is unsafe.
    // This documents the Label fix — use data()+size instead.
    std::string owned = text.substr(0, text.size());
    const std::string_view stable(owned.data(), owned.size());
    TUINATOR_CHECK_EQ(tuinator::text_display_width(stable), 22);
}
