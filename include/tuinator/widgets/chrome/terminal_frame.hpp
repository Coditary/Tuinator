#pragma once

#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/render/border_style.hpp>
#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/widgets/chrome/status_line.hpp>
#include <tuinator/widgets/widget.hpp>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace tuinator {

struct TerminalFrameStyle {
    Style border{};
    Style title{};
    Style title_background{};
    Style content_background{};
    BorderStyle border_style = BorderStyle::Rounded;
    bool show_controls = true;
    StatusLineStyle status_line{};
};

struct TerminalFrameOptions {
    bool show_status_line = true;
    /// When true, child widgets paint directly (required for live PTY shells).
    bool live_content = false;
    TerminalFrameStyle style{};
};

/// Embedded terminal surface with title bar, border, and optional status line.
///
/// Renders child widgets into an off-screen MemoryTerminalBackend and blits the
/// result into the frame. This is a display surface for Tuinator widgets — not a
/// PTY host and does not spawn your system shell (zsh/bash).
class TerminalFrame : public Widget {
  public:
    TerminalFrame(std::string title, std::unique_ptr<Widget> content, TerminalFrameOptions options = {});

    const std::string& title() const { return title_; }
    void set_title(std::string title);

    void set_content(std::unique_ptr<Widget> content);
    Widget* content() const { return content_.get(); }

    void set_status_left(std::vector<StatusSegment> segments);
    void set_status_center(std::vector<StatusSegment> segments);
    void set_status_right(std::vector<StatusSegment> segments);

    const MemoryTerminalBackend& surface() const { return *surface_; }
    void refresh_surface(const Theme& theme);

    void set_on_dirty(std::function<void(Rect)> callback) override;

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;
    void on_idle() override;
    bool needs_periodic_idle() const override;
    void for_each_descendant(const std::function<void(Widget*)>& visitor) override;
    bool handle_event(const Event& event) override;
    Widget* hit_test(Point point) override;
    Widget* hit_test_focusable(Point point) override;
    bool has_focused_descendant() const override;
    void collect_focusable(std::vector<Widget*>& out) override;
    void for_each_child(const std::function<void(Widget*)>& visitor) override;

  private:
    Rect content_bounds() const;
    Rect content_area_local() const;
    Rect status_bounds() const;
    BorderGlyphs border_glyphs() const;
    void ensure_surface_size() const;
    void render_surface(const Theme& theme) const;
    void blit_surface(Canvas& canvas, Point origin) const;
    void paint_title_bar(Canvas& canvas) const;
    void bind_content_callbacks();

    std::string title_;
    TerminalFrameOptions options_;
    std::unique_ptr<Widget> content_;
    StatusLine status_line_;
    mutable std::unique_ptr<MemoryTerminalBackend> surface_;
    mutable bool surface_dirty_ = true;
};

} // namespace tuinator
