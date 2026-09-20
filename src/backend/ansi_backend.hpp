#pragma once

#include <tuinator/backend/terminal_backend.hpp>

#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#if TUINATOR_PLATFORM_POSIX
#include <termios.h>
#endif

#include "backend/terminal_input.hpp"

namespace tuinator::detail {

class AnsiBackend final : public TerminalBackend {
  public:
    AnsiBackend();
    ~AnsiBackend() override;

    void init() override;
    void shutdown() override;

    /// Redirects the escape-sequence output for headless sequence tests.
    /// The backend closes the stream on shutdown; tests must not fclose it.
    void set_output_file_for_testing(FILE* output) { tty_out_ = output; }

    Size terminal_size() const override;
    std::optional<Event> poll_event() override;
    std::optional<Event> poll_event_nonblocking() override;

    void begin_frame(BeginFrameOptions options = {}) override;
    void end_frame() override;
    void refresh_mouse_cursor() override;
    void set_mouse_cursor_suppressed(bool suppressed) override;
    void set_text_cursor(std::optional<Point> position) override;

    void invalidate_graphics() override;

    void draw_text(int x, int y, std::string_view text, Style style) override;
    void draw_image(int x, int y, Size cell_size, const TerminalImage& image) override;

    bool pointer_active() const override { return left_button_down_; }
    void set_poll_timeout_ms(int timeout_ms) override;
    bool true_color() const override { return true_color_enabled_; }

  private:
    struct AnsiDraw {
        int x = 0;
        int y = 0;
        std::string text;
        Style style{};
    };

    struct ImageDraw {
        int x = 0;
        int y = 0;
        int cols = 0;
        int rows = 0;
        std::uint32_t image_id = 0;
        std::uint32_t slot_id = 0;
        std::string transmit;
        std::string place;
    };

    struct KittySlot {
        std::uint32_t ping_id_a = 0;
        std::uint32_t ping_id_b = 0;
        bool ping_use_a = false;
        std::uint32_t live_image_id = 0;
        std::uint32_t content_hash = 0;
        bool ready = false;
        bool touched_this_frame = false;
        int x = -1;
        int y = -1;
        int cols = 0;
        int rows = 0;
    };

    struct AnsiCell {
        std::string glyph;
        Style style{};
        std::uint8_t width = 1;
        bool known = false;
    };

    void queue_ansi_draw(int x, int y, std::string_view text, Style style);
    void flush_ansi_draws(FILE* output);
    void flush_image_draws(FILE* output);
    void emit_ansi_run(FILE* output, int x, int y, std::string_view text, const Style& style);
    void invalidate_ansi_cells();
    FILE* output_stream() const;
    void write_tty_sequence(const char* sequence);
    void close_terminal_streams();
    void present_frame();
    void enable_mouse();
    void disable_mouse();
    bool detect_true_color() const;
    std::optional<Event> read_event(bool block);
    void track_input_event(const Event& event);
    void position_hardware_mouse_cursor(FILE* output);
    void present_text_cursor(FILE* output);
    int mouse_tracking_mode() const;
    void cleanup_kitty_graphics();
    void clear_region(Rect region);
    Rect kitty_placement_rect() const;
    Size query_terminal_size() const;
    void acquire_stdin();
    void release_stdin();
    void drain_stdin();

    Rect frame_clip_{{0, 0}, {0, 0}};
    Rect ansi_clip_{{0, 0}, {0, 0}};
    bool full_frame_redraw_ = true;
    bool sync_updates_supported_ = false;

    bool alternate_screen_active_ = false;
    bool initialized_ = false;
    bool true_color_enabled_ = false;
    bool mouse_enabled_ = false;
    bool xterm_mouse_enabled_ = false;
    bool left_button_down_ = false;
    bool mouse_cursor_user_enabled_ = false;
    bool mouse_cursor_suppressed_ = false;
    bool mouse_cursor_visible_ = false;
    std::optional<Point> last_mouse_position_;
    std::optional<Point> text_cursor_position_;
    std::optional<Point> placed_text_cursor_;
    bool hardware_text_cursor_visible_ = false;
    int poll_timeout_ms_ = -1;
    int terminal_width_ = 80;
    int terminal_height_ = 24;
    std::vector<AnsiDraw> pending_ansi_draws_;
    std::vector<ImageDraw> pending_image_draws_;
    std::vector<AnsiCell> ansi_cells_;
    int ansi_cells_width_ = 0;
    int ansi_cells_height_ = 0;
    std::unordered_map<std::uint32_t, KittySlot> kitty_slots_;
    std::uint32_t next_kitty_slot_id_ = 1;
    std::uint32_t next_kitty_image_id_ = 1;
    std::optional<std::uint32_t> find_kitty_slot(std::uint32_t content_hash, int x, int y, int cols, int rows) const;
    /// Dirty-region/kitty overlap recorded in begin_frame; acted on in
    /// flush_image_draws once we know whether the image was re-placed.
    Rect kitty_pending_overlap_{};

    FILE* tty_out_ = nullptr;
    FILE* tty_in_ = nullptr;
    TerminalInput input_;
    std::deque<Event> pending_events_;
#if TUINATOR_PLATFORM_POSIX
    termios stdin_original_{};
    int stdin_original_flags_ = 0;
#endif
    bool stdin_captured_ = false;
    bool stdin_nonblocking_set_ = false;
};

} // namespace tuinator::detail
