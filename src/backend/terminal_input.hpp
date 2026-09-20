#pragma once

#include <tuinator/core/event.hpp>

#include <atomic>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace tuinator::detail {

/// Incremental parser for raw terminal input (CSI keys, SGR mouse, plain bytes).
class TerminalInput {
  public:
    void reset();

    /// Returns a decoded event when a complete sequence or key is available.
    std::optional<Event> feed(unsigned char byte);
    /// Feeds a whole chunk and returns every decoded event in order.
    std::vector<Event> feed_bytes(const unsigned char* data, std::size_t length);

    /// True while a lone ESC byte waits for possible sequence continuation.
    bool has_pending_escape() const;
    /// Resolves a pending lone ESC into a Key::Escape event (escape timeout).
    std::optional<Event> flush_escape();

    /// Set when SIGWINCH fires; consume via take_resize_pending().
    static void notify_resize();
    static bool take_resize_pending();

  private:
    enum class State : std::uint8_t {
        Normal,
        Escape,
        Csi,
        MouseSgr,
        Paste,
    };

    std::optional<Event> finish_csi();
    std::optional<Event> finish_mouse_sgr(char final_byte);
    std::optional<KeyPress> key_from_csi_params(char final_byte) const;
    std::optional<Event> feed_paste_byte(unsigned char byte);

    State state_ = State::Normal;
    std::string buffer_;
    std::string paste_buffer_;
    int paste_end_match_ = 0;
    bool alt_pending_ = false;
    std::string utf8_buffer_;
    int utf8_remaining_ = 0;
};

void install_terminal_signal_handlers();

} // namespace tuinator::detail
