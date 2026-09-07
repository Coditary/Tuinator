#include <tuinator/render/text.hpp>

#include <string>

#include "bench_harness.hpp"

namespace {

std::string mixed_utf8_text(std::size_t repeats) {
    static constexpr char kChunk[] = "Tuinator ☀️❤️🕛 日本語 αβγ 数据探索 ";
    std::string out;
    for (std::size_t i = 0; i < repeats; ++i) {
        out.append(kChunk);
    }
    return out;
}

} // namespace

TUINATOR_PERF_TEST(text_display_width_large_mixed_utf8) {
    const std::string text = mixed_utf8_text(2'000);
    return tuinator::perf::bench("text_display_width_large_mixed_utf8", 80.0, 2, 20,
                                 [&]() { tuinator::text_display_width(text); });
}

TUINATOR_PERF_TEST(text_byte_length_for_width_large_mixed_utf8) {
    const std::string text = mixed_utf8_text(2'000);
    return tuinator::perf::bench("text_byte_length_for_width_large_mixed_utf8", 120.0, 2, 20,
                                 [&]() { tuinator::text_byte_length_for_width(text, 120); });
}

TUINATOR_PERF_TEST(text_display_width_many_small_strings) {
    static constexpr char kSamples[][16] = {
        "hello", "☀️", "日本語", "αβγ", "数据探索", "⠋", "❤️",
    };

    return tuinator::perf::bench("text_display_width_many_small_strings", 40.0, 2, 20, [&]() {
        for (int round = 0; round < 10'000; ++round) {
            for (const char* sample : kSamples) {
                tuinator::text_display_width(sample);
            }
        }
    });
}
