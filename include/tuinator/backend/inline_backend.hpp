#pragma once

#include <tuinator/backend/terminal_backend.hpp>

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace tuinator {

/// Options for rendering a Tuinator UI inline in the terminal scrollback
/// (below the shell prompt), similar to ESLint's live progress reporter.
struct InlineBackendOptions {
    /// Height of the inline region in terminal rows. 0 = use a sensible default.
    int height = 0;
    int min_height = 3;
    /// Maximum height (0 = no limit).
    int max_height = 0;
    /// 1-based row where the inline region starts (overrides other anchor modes).
    int anchor_row = 0;
    /// When true, pin the band to the terminal bottom instead of the current cursor row.
    bool pin_to_bottom = false;
    bool clear_on_shutdown = true;
    bool true_color = true;
    FILE* output = nullptr;
};

/// Renders into a fixed band of terminal rows without taking over the alternate screen.
/// By default the band starts at the current cursor row (directly below prior output).
class InlineTerminalBackend : public TerminalBackend {
public:
    explicit InlineTerminalBackend(InlineBackendOptions options = {});
    ~InlineTerminalBackend() override;

    void init() override;
    void shutdown() override;

    Size terminal_size() const override;
    std::optional<Event> poll_event() override;

    void begin_frame(BeginFrameOptions options = {}) override;
    void end_frame() override;

    void draw_text(int x, int y, std::string_view text, Style style) override;
    bool true_color() const override { return true_color_; }

    void set_poll_timeout_ms(int timeout_ms) override;

    /// Terminal row (1-based) where the inline region starts.
    int anchor_row() const { return anchor_row_; }

    static std::unique_ptr<InlineTerminalBackend> create(InlineBackendOptions options = {});

private:
    struct Cell {
        std::string text;
        Style style{};
        bool wide_tail = false;
    };

    int compute_region_height(int term_height, int available_rows) const;
    void place_anchor(int term_height);
    void sync_geometry(const Size& term, bool allow_reanchor);
    void sync_geometry_relative(const Size& term, bool initial);
    void resize_buffer(int width, int height);
    void emit_frame_to_terminal();
    void emit_frame_relative();
    void clear_terminal_region();
    void append_erase_lines(std::string& out, int count) const;
    void write_output(std::string_view data);
    int last_nonempty_column(int y) const;
    void write_row_content(int y, std::string& out) const;
    void write_row_absolute(int abs_row, int y) const;
    void append_style(std::string& out, const Style& style) const;
    bool style_equal(const Style& a, const Style& b) const;
    bool cell_equal(const Cell& a, const Cell& b) const;

    InlineBackendOptions options_;
    FILE* output_ = nullptr;
    FILE* owned_tty_ = nullptr;
    int anchor_row_ = 1;
    int cursor_anchor_row_ = 0;
    int band_height_ = 0;
    int region_width_ = 80;
    int region_height_ = 10;
    int poll_timeout_ms_ = -1;
    int frames_drawn_ = 0;
    bool initialized_ = false;
    bool anchor_locked_ = false;
    bool relative_draw_ = false;
    bool true_color_ = true;
    std::vector<std::vector<Cell>> cells_;
    std::vector<std::vector<Cell>> previous_cells_;
};

} // namespace tuinator
