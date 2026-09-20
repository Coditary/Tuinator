#include "backend/ansi_backend.hpp"
#include "test_harness.hpp"

#include <tuinator/render/terminal_image.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

namespace {

std::string frame_output(tuinator::detail::AnsiBackend& backend, FILE* mem, char* buf, int y,
                         const tuinator::TerminalImage& image) {
    backend.begin_frame({.full_redraw = false, .clear_buffer = false, .dirty_region = {0, y, 20, 12}});
    backend.draw_image(0, y, {10, 5}, image);
    backend.end_frame();
    std::fflush(mem);
    const long length = std::ftell(mem);
    std::fseek(mem, 0, SEEK_SET);
    return std::string(buf, static_cast<std::size_t>(length));
}

} // namespace

TUINATOR_TEST(ansi_backend_kitty_move_deletes_before_replace) {
    tuinator::detail::AnsiBackend backend;
    constexpr std::size_t capacity = 1 << 20;
    char* buf = static_cast<char*>(std::malloc(capacity));
    FILE* mem = fmemopen(buf, capacity, "w");
    TUINATOR_CHECK(mem != nullptr);
    backend.set_output_file_for_testing(mem);

    const tuinator::TerminalImage image = tuinator::TerminalImage::gradient(8, 8);

    // Note: the ping-pong flip runs before the first transmit, so the first
    // live id is 2; the next content change switches to 1.
    const std::string first = frame_output(backend, mem, buf, 5, image);
    TUINATOR_CHECK(first.find("a=p,i=2") != std::string::npos);
    TUINATOR_CHECK(first.find("a=d") == std::string::npos); // nothing to delete yet

    // Same image moved: old placement must be deleted BEFORE the new place,
    // otherwise terminals accumulate ghost copies when scrolling.
    const std::string moved = frame_output(backend, mem, buf, 3, image);
    const std::size_t del = moved.find("a=d,d=i,i=2");
    const std::size_t place = moved.find("a=p,i=2");
    TUINATOR_CHECK(del != std::string::npos);
    TUINATOR_CHECK(place != std::string::npos);
    TUINATOR_CHECK(del < place);

    // Changed pixels (animation frame): ping-pong to id 1. The new id must be
    // placed BEFORE the old id is deleted, or the image flashes blank.
    const std::string changed = frame_output(backend, mem, buf, 3, tuinator::TerminalImage::gradient(8, 9));
    const std::size_t place2 = changed.find("a=p,i=1");
    const std::size_t del1 = changed.find("a=d,d=i,i=2");
    TUINATOR_CHECK(place2 != std::string::npos);
    TUINATOR_CHECK(del1 != std::string::npos);
    TUINATOR_CHECK(place2 < del1);

    // Backend was never init'ed, so shutdown leaves the stream open.
    std::fclose(mem);
    std::free(buf);
}

TUINATOR_TEST(ansi_backend_kitty_places_two_images_without_deleting_each_other) {
    tuinator::detail::AnsiBackend backend;
    constexpr std::size_t capacity = 1 << 20;
    char* buf = static_cast<char*>(std::malloc(capacity));
    FILE* mem = fmemopen(buf, capacity, "w");
    TUINATOR_CHECK(mem != nullptr);
    backend.set_output_file_for_testing(mem);

    const tuinator::TerminalImage left = tuinator::TerminalImage::gradient(8, 8);
    const tuinator::TerminalImage right = tuinator::TerminalImage::gradient(9, 8);

    backend.begin_frame({.full_redraw = false, .clear_buffer = false, .dirty_region = {0, 0, 40, 12}});
    backend.draw_image(0, 2, {10, 5}, left);
    backend.draw_image(14, 2, {10, 5}, right);
    backend.end_frame();
    std::fflush(mem);
    const long length = std::ftell(mem);
    std::fseek(mem, 0, SEEK_SET);
    const std::string output(buf, static_cast<std::size_t>(length));

    const std::size_t place1 = output.find("a=p,i=2");
    const std::size_t place2 = output.find("a=p,i=4");
    TUINATOR_CHECK(place1 != std::string::npos);
    TUINATOR_CHECK(place2 != std::string::npos);
    TUINATOR_CHECK(output.find("a=d,d=i,i=2") == std::string::npos);

    std::fclose(mem);
    std::free(buf);
}

TUINATOR_TEST(ansi_backend_kitty_places_same_image_at_two_positions) {
    tuinator::detail::AnsiBackend backend;
    constexpr std::size_t capacity = 1 << 20;
    char* buf = static_cast<char*>(std::malloc(capacity));
    FILE* mem = fmemopen(buf, capacity, "w");
    TUINATOR_CHECK(mem != nullptr);
    backend.set_output_file_for_testing(mem);

    const tuinator::TerminalImage image = tuinator::TerminalImage::gradient(8, 8);

    backend.begin_frame({.full_redraw = false, .clear_buffer = false, .dirty_region = {0, 0, 40, 12}});
    backend.draw_image(0, 2, {10, 5}, image);
    backend.draw_image(14, 2, {10, 5}, image);
    backend.end_frame();
    std::fflush(mem);
    const long length = std::ftell(mem);
    std::fseek(mem, 0, SEEK_SET);
    const std::string output(buf, static_cast<std::size_t>(length));

    const std::size_t place_a = output.find("a=p,i=2");
    const std::size_t place_b = output.find("a=p,i=4");
    TUINATOR_CHECK(place_a != std::string::npos);
    TUINATOR_CHECK(place_b != std::string::npos);
    TUINATOR_CHECK(output.find("a=d,d=i,i=2") == std::string::npos);

    std::fclose(mem);
    std::free(buf);
}
