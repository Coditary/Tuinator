#include <tuinator/widgets/chrome/shell_terminal.hpp>

#include "test_harness.hpp"

TUINATOR_TEST(shell_terminal_preferred_size_matches_typical_tty) {
    tuinator::ShellTerminal terminal;
    const tuinator::Size size = terminal.preferred_size();
    TUINATOR_CHECK_EQ(size.width, 80);
    TUINATOR_CHECK_EQ(size.height, 24);
}

TUINATOR_TEST(shell_terminal_is_shell_widget) {
    tuinator::ShellTerminal terminal;
    TUINATOR_CHECK(terminal.is_shell_terminal());
    TUINATOR_CHECK(terminal.wants_full_screen());
    TUINATOR_CHECK(terminal.wants_initial_focus());
    TUINATOR_CHECK(!terminal.is_running());
}

TUINATOR_TEST(shell_terminal_layout_resizes_ansi_buffer) {
    tuinator::ShellTerminal terminal;
    terminal.layout({0, 0, 52, 16});
    const tuinator::Size buffer = terminal.buffer().size();
    TUINATOR_CHECK_EQ(buffer.width, 52);
    TUINATOR_CHECK_EQ(buffer.height, 16);
}
