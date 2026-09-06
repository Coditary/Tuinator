#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/core/event.hpp>

#include <variant>

#include "test_harness.hpp"

TUINATOR_TEST(memory_backend_snapshot_and_resize) {
    tuinator::MemoryTerminalBackend backend({10, 3});
    backend.init();

    backend.draw_text(1, 1, "Hi", tuinator::Style{});
    TUINATOR_CHECK(backend.snapshot().find("Hi") != std::string::npos);

    backend.resize({12, 4});
    TUINATOR_CHECK_EQ(backend.terminal_size().width, 12);
    TUINATOR_CHECK_EQ(backend.terminal_size().height, 4);
}

TUINATOR_TEST(memory_backend_event_queue) {
    tuinator::MemoryTerminalBackend backend({});
    backend.init();
    backend.push_event(tuinator::KeyPress{tuinator::Key::Enter});

    const auto event = backend.poll_event();
    TUINATOR_CHECK(event.has_value());
    TUINATOR_CHECK(std::holds_alternative<tuinator::KeyPress>(*event));
}
