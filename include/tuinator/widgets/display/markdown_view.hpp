#pragma once

#include <tuinator/render/animated_image.hpp>
#include <tuinator/render/markdown.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/render/terminal_image.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/capabilities.hpp>
#include <tuinator/widgets/widget.hpp>

#include <cstddef>
#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace tuinator {

/// Style bundle used by MarkdownView. Defaults come from markdown_styles_from_theme().
struct MarkdownStyleSet {
    Style text;
    Style heading;      ///< H1–H2
    Style heading_sub;  ///< H3–H6
    Style code;         ///< Inline `code` spans
    Style code_block;   ///< Code block glyphs
    Style code_block_fill; ///< Code block row background (full width)
    Style quote_text;
    Style quote_bar;
    Style link;
    Style list_marker;
    Style thematic_break;
};

MarkdownStyleSet markdown_styles_from_theme(const Theme& theme);

struct MarkdownViewOptions {
    int min_width = 16;
    int min_height = 3;
    /// When true, appending keeps the view scrolled to the end (streaming).
    bool follow_tail = true;
    /// Defaults to styles derived from dark_theme(); pass
    /// markdown_styles_from_theme(theme) to match another app theme.
    MarkdownStyleSet styles = markdown_styles_from_theme(dark_theme());
    /// Document parser; nullptr means parse_markdown. AsciiDocView sets
    /// parse_asciidoc here.
    MarkupParser parser = nullptr;
    /// Render block-level images (a paragraph containing only an image, e.g.
    /// `![alt](path)` / `image::path[alt]`) via the terminal graphics
    /// protocol. Missing/unloadable files fall back to styled alt text;
    /// inline images inside text always render as styled alt text.
    bool render_images = true;
    /// Maximum terminal rows an image may occupy.
    int max_image_rows = 12;
};

/// Read-only markdown display with streaming support.
///
/// append() feeds raw markdown (e.g. tokens from an AI backend); the tolerant
/// parser handles partial input — unclosed fences render as code, unclosed
/// emphasis stays literal. Only the widget bounds are marked dirty per update,
/// and the terminal backend emits just the cells that changed.
class MarkdownView : public Widget, public Scrollable {
  public:
    explicit MarkdownView(MarkdownViewOptions options = {});
    MarkdownView(std::string markdown, MarkdownViewOptions options = {});

    /// Replace the whole document.
    void set_markdown(std::string text);
    /// Append a chunk of markdown (streaming). Reparses only the unstable tail.
    void append(std::string_view chunk);
    void clear();
    const std::string& source() const { return source_; }

    void set_follow_tail(bool follow);
    bool follow_tail() const { return options_.follow_tail; }

    void set_styles(MarkdownStyleSet styles);
    const MarkdownStyleSet& styles() const { return styles_; }

    /// Wrapped visual line count at the current width.
    int content_height() const { return static_cast<int>(lines_.size()); }
    int max_scroll_y() const;

    int scroll_x() const override { return 0; }
    int scroll_y() const override { return scroll_y_; }
    bool contains_widget(const Widget* widget) const override { return widget == this; }
    void ensure_visible(const Widget* widget) override { (void)widget; }
    bool try_scroll(const Event& event) override;
    void scroll_by(int dx, int dy) override;
    void scroll_to(int y);
    Widget* scroll_content() const override { return const_cast<MarkdownView*>(this); }

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;

    /// Animated images (GIF) drive repaints via the app's idle polling;
    /// on_idle() marks only the image rows dirty, not the whole view.
    bool needs_periodic_idle() const override { return has_animated_image_; }
    void on_idle() override;

    std::string_view widget_type_name() const override { return "MarkdownView"; }

  private:
    struct VisualSegment {
        std::string text;
        Style style;
    };

    struct VisualLine {
        std::vector<VisualSegment> segments;
        bool code_block = false;
        /// Set when this line anchors an image covering image_rows lines
        /// (this line plus the following spacer lines). Animated GIFs pick
        /// their frame by elapsed time at paint.
        std::shared_ptr<const AnimatedImage> image;
        int image_rows = 0;
        int image_cols = 0;
    };

    void reparse();
    void rewrap();
    void clamp_scroll();
    void append_block_lines(const MdBlock& block, bool first_block);
    void append_image_lines(const MdSpan& span);
    std::shared_ptr<const AnimatedImage> cached_image(const std::string& path);
    Style span_style(const MdInlineFlags& flags) const;

    MarkdownViewOptions options_{};
    MarkdownStyleSet styles_{};
    std::string source_;
    std::vector<MdBlock> blocks_;
    /// Incremental parse cache: blocks ending before stable_end_ are final.
    std::vector<MdBlock> stable_blocks_;
    std::size_t stable_end_ = 0;
    std::vector<VisualLine> lines_;
    MdBlockKind last_block_kind_ = MdBlockKind::Paragraph;
    std::unordered_map<std::string, std::shared_ptr<const AnimatedImage>> image_cache_;
    int scroll_y_ = 0;
    bool unicode_ = true;
    bool has_animated_image_ = false;
    mutable std::chrono::steady_clock::time_point anim_start_{};
    mutable bool anim_started_ = false;
    /// Frame index last painted per visual line (-1 = not drawn), parallel to lines_.
    mutable std::vector<int> drawn_frame_;
};

} // namespace tuinator
