#pragma once

#include "backend/curses_config.hpp"

#include <tuinator/backend/terminal_backend.hpp>

#include <array>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace tuinator::detail {

class CursesBackend final : public TerminalBackend {
public:
    CursesBackend();
    ~CursesBackend() override;

    void init() override;
    void shutdown() override;

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
        std::string transmit;
        std::string place;
    };

    static constexpr int kColorCount = 8;
    static constexpr int kPairSlots = 1 + kColorCount + kColorCount + (kColorCount * kColorCount);
    static constexpr int kExtendedColorBase = 256;
    static constexpr int kExtendedPairBase = 4096;

    int color_pair_for(Style style);
    int extended_color_pair_for(Style style);
    bool style_uses_rgb(const Style& style) const;
    void queue_ansi_draw(int x, int y, std::string_view text, Style style);
    void flush_ansi_draws(FILE* output);
    void flush_image_draws(FILE* output);
    void draw_text_ansi(FILE* output, int x, int y, std::string_view text, Style style);
    FILE* output_stream() const;
    void write_tty_sequence(const char* sequence);
    void prepare_refresh(FILE* output);
    void present_frame();
    void init_curses_screen();
    void close_terminal_streams();
    int ensure_color_pair(int fg_code, int bg_code);
    int ensure_extended_color(Rgb rgb);
    int ensure_extended_pair(int fg_id, int bg_id);
    int resolve_extended_color_id(Color palette, const std::optional<Rgb>& rgb);
    void setup_default_color_pair();
    void enable_mouse();
    void disable_mouse();
    bool detect_true_color() const;
    std::optional<Event> read_event(bool block);
    void position_hardware_mouse_cursor(FILE* output);
    void present_text_cursor(FILE* output);
    int mouse_tracking_mode() const;
    void cleanup_kitty_graphics();
    void clear_region(Rect region);
    bool ansi_draw_visible(const AnsiDraw& draw) const;

    Rect frame_clip_{{0, 0}, {0, 0}};

    bool initialized_ = false;
    bool colors_enabled_ = false;
    bool true_color_enabled_ = false;
    bool extended_colors_available_ = false;
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
    int next_pair_id_ = 2;
    int next_extended_color_ = kExtendedColorBase;
    int next_extended_pair_ = kExtendedPairBase;
    std::array<std::int16_t, kPairSlots> pair_cache_{};
    std::unordered_map<std::uint32_t, int> extended_color_cache_;
    std::unordered_map<std::uint64_t, int> extended_pair_cache_;
    std::vector<AnsiDraw> pending_ansi_draws_;
    std::vector<ImageDraw> pending_image_draws_;
    std::uint32_t kitty_cached_hash_ = 0;
    bool kitty_image_ready_ = false;

    struct KittyPlacement {
        int x = -1;
        int y = -1;
        int cols = 0;
        int rows = 0;
        std::uint32_t hash = 0;
    };
    KittyPlacement last_kitty_placement_{};

    FILE* tty_out_ = nullptr;
    FILE* tty_in_ = nullptr;
#if defined(TUINATOR_BACKEND_NCURSES)
    SCREEN* screen_ = nullptr;
#endif
    bool unified_output_ = false;
};

} // namespace tuinator::detail
