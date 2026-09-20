#include <tuinator/widgets/display/markdown_view.hpp>

#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/paint_context.hpp>
#include <tuinator/render/text.hpp>

#include <algorithm>
#include <utility>

namespace tuinator {

namespace {

struct Atom {
    std::string glyph;
    int width = 1;
    Style style;
};

void merge_colors(Style& target, const Style& from) {
    target.foreground = from.foreground;
    target.background = from.background;
    target.foreground_rgb = from.foreground_rgb;
    target.background_rgb = from.background_rgb;
}

bool same_style(const Style& a, const Style& b) {
    return a.foreground == b.foreground && a.background == b.background && a.foreground_rgb == b.foreground_rgb &&
           a.background_rgb == b.background_rgb && a.bold == b.bold && a.dim == b.dim && a.reverse == b.reverse &&
           a.italic == b.italic && a.underline == b.underline && a.strikethrough == b.strikethrough;
}

/// Greedy word wrap over grapheme atoms: break at the last space when possible,
/// hard-break overlong words. `first_width`/`cont_width` are the free columns
/// on the first vs. continuation lines.
std::vector<std::vector<Atom>> wrap_atoms(const std::vector<Atom>& atoms, int first_width, int cont_width) {
    std::vector<std::vector<Atom>> lines;
    std::vector<Atom> current;
    int avail = std::max(1, first_width);
    int x = 0;
    std::size_t index = 0;

    while (index < atoms.size()) {
        const Atom& atom = atoms[index];
        if (x + atom.width > avail && !current.empty()) {
            std::size_t last_space = std::string::npos;
            for (std::size_t i = current.size(); i-- > 0;) {
                if (current[i].glyph == " ") {
                    last_space = i;
                    break;
                }
            }
            if (last_space != std::string::npos && last_space + 1 < current.size()) {
                std::vector<Atom> tail(current.begin() + static_cast<std::ptrdiff_t>(last_space + 1), current.end());
                current.resize(last_space);
                lines.push_back(std::move(current));
                current = std::move(tail);
                x = 0;
                for (const Atom& a : current) {
                    x += a.width;
                }
            } else {
                lines.push_back(std::move(current));
                current.clear();
                x = 0;
            }
            avail = std::max(1, cont_width);
            continue;
        }
        if (current.empty() && atom.glyph == " ") {
            ++index; // no leading spaces after a wrap
            continue;
        }
        x += atom.width;
        current.push_back(atom);
        ++index;
    }
    if (!current.empty()) {
        lines.push_back(std::move(current));
    }
    return lines;
}

std::string repeat_glyph(const std::string& glyph, int count) {
    std::string out;
    for (int i = 0; i < count; ++i) {
        out += glyph;
    }
    return out;
}

} // namespace

MarkdownStyleSet markdown_styles_from_theme(const Theme& theme) {
    MarkdownStyleSet styles;
    styles.text = theme.label;

    styles.heading = theme.heading;
    styles.heading.bold = true;
    if (!styles.heading.foreground_rgb.has_value() && styles.heading.foreground == Color::Default) {
        styles.heading.foreground_rgb = Rgb::hex(0x89B4FA);
    }

    styles.heading_sub = styles.text;
    styles.heading_sub.bold = true;

    styles.code = styles.text;
    styles.code.foreground = Color::Default;
    styles.code.foreground_rgb = Rgb::hex(0xF9E2AF);
    styles.code.background_rgb = Rgb::hex(0x313244);

    styles.code_block = styles.text;
    styles.code_block.foreground = Color::Default;
    styles.code_block.foreground_rgb = Rgb::hex(0xCDD6F4);
    styles.code_block.background_rgb = Rgb::hex(0x181825);

    styles.code_block_fill = Style{};
    styles.code_block_fill.background_rgb = Rgb::hex(0x181825);

    styles.quote_text = styles.text;
    styles.quote_text.italic = true;
    styles.quote_text.dim = true;

    styles.quote_bar = theme.accent;
    if (!styles.quote_bar.foreground_rgb.has_value() && styles.quote_bar.foreground == Color::Default) {
        styles.quote_bar.foreground_rgb = Rgb::hex(0x89B4FA);
    }

    styles.link = styles.text;
    styles.link.underline = true;
    styles.link.foreground = Color::Default;
    styles.link.foreground_rgb = Rgb::hex(0x89B4FA);

    styles.list_marker = theme.accent;
    if (!styles.list_marker.foreground_rgb.has_value() && styles.list_marker.foreground == Color::Default) {
        styles.list_marker.foreground_rgb = Rgb::hex(0xFAB387);
    }

    styles.thematic_break = theme.muted;
    styles.thematic_break.dim = true;
    return styles;
}

MarkdownView::MarkdownView(MarkdownViewOptions options)
    : options_(std::move(options)), styles_(options_.styles) {
    unicode_ = supports_unicode_text();
}

MarkdownView::MarkdownView(std::string markdown, MarkdownViewOptions options) : MarkdownView(std::move(options)) {
    set_markdown(std::move(markdown));
}

void MarkdownView::set_markdown(std::string text) {
    source_ = std::move(text);
    stable_blocks_.clear();
    stable_end_ = 0;
    reparse();
}

void MarkdownView::append(std::string_view chunk) {
    if (chunk.empty()) {
        return;
    }
    source_ += chunk;
    reparse();
}

void MarkdownView::clear() {
    if (source_.empty()) {
        return;
    }
    source_.clear();
    blocks_.clear();
    stable_blocks_.clear();
    stable_end_ = 0;
    lines_.clear();
    scroll_y_ = 0;
    mark_dirty();
}

void MarkdownView::set_follow_tail(bool follow) {
    options_.follow_tail = follow;
    if (follow) {
        scroll_to(max_scroll_y());
    }
}

void MarkdownView::set_styles(MarkdownStyleSet styles) {
    styles_ = std::move(styles);
    rewrap();
    mark_dirty();
}

int MarkdownView::max_scroll_y() const {
    return std::max(0, static_cast<int>(lines_.size()) - bounds_.height);
}

void MarkdownView::scroll_by(int dx, int dy) {
    (void)dx;
    scroll_to(scroll_y_ + dy);
}

void MarkdownView::scroll_to(int y) {
    const int next = std::clamp(y, 0, max_scroll_y());
    if (next == scroll_y_) {
        return;
    }
    scroll_y_ = next;
    mark_dirty();
}

bool MarkdownView::try_scroll(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (!bounds_.contains(mouse->position)) {
            return false;
        }
        switch (mouse->action) {
        case MouseAction::WheelUp: scroll_by(0, -3); return true;
        case MouseAction::WheelDown: scroll_by(0, 3); return true;
        default: return false;
        }
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (key == nullptr) {
        return false;
    }
    switch (key->key) {
    case Key::PageUp: scroll_by(0, -std::max(1, bounds_.height)); return true;
    case Key::PageDown: scroll_by(0, std::max(1, bounds_.height)); return true;
    case Key::Home: scroll_to(0); return true;
    case Key::End: scroll_to(max_scroll_y()); return true;
    case Key::Up: scroll_by(0, -1); return true;
    case Key::Down: scroll_by(0, 1); return true;
    default: return false;
    }
}

bool MarkdownView::handle_event(const Event& event) { return try_scroll(event); }

Size MarkdownView::preferred_size() const { return {options_.min_width, options_.min_height}; }

void MarkdownView::layout(Rect bounds) {
    const bool width_changed = bounds.width != bounds_.width;
    bounds_ = bounds;
    if (width_changed) {
        rewrap();
        clamp_scroll();
    }
}

void MarkdownView::reparse() {
    const MarkupParser parser = options_.parser != nullptr ? options_.parser : parse_markdown;
    const std::string_view tail = std::string_view(source_).substr(stable_end_);
    MarkdownDocument tail_doc = parser(tail);

    blocks_ = stable_blocks_;
    blocks_.insert(blocks_.end(), std::make_move_iterator(tail_doc.blocks.begin()),
                   std::make_move_iterator(tail_doc.blocks.end()));

    if (tail_doc.stable_prefix > 0) {
        MarkdownDocument newly_stable = parser(tail.substr(0, tail_doc.stable_prefix));
        stable_blocks_.insert(stable_blocks_.end(), std::make_move_iterator(newly_stable.blocks.begin()),
                              std::make_move_iterator(newly_stable.blocks.end()));
        stable_end_ += tail_doc.stable_prefix;
    }

    rewrap();
    if (options_.follow_tail) {
        scroll_y_ = max_scroll_y();
    }
    clamp_scroll();
    mark_dirty();
}

Style MarkdownView::span_style(const MdInlineFlags& flags) const {
    Style style = styles_.text;
    if (flags.code) {
        merge_colors(style, styles_.code);
    }
    if (flags.link) {
        merge_colors(style, styles_.link);
        style.underline = styles_.link.underline;
    }
    if (flags.image) {
        style.dim = true;
        style.underline = true;
    }
    style.bold = style.bold || flags.bold;
    style.italic = style.italic || flags.italic;
    style.strikethrough = style.strikethrough || flags.strike;
    return style;
}

void MarkdownView::append_block_lines(const MdBlock& block, bool first_block) {
    const int width = bounds_.width;
    const bool groups_with_previous = block.kind == MdBlockKind::ListItem && !lines_.empty() && !first_block &&
                                      last_block_kind_ == MdBlockKind::ListItem;
    if (!first_block && !groups_with_previous) {
        lines_.push_back(VisualLine{});
    }
    last_block_kind_ = block.kind;

    const auto emit = [&](const std::vector<Atom>& atoms, const std::string& first_prefix,
                          const std::string& cont_prefix, Style prefix_style) {
        const int indent = text_display_width(first_prefix);
        auto wrapped_lines = wrap_atoms(atoms, width - indent, width - text_display_width(cont_prefix));
        if (wrapped_lines.empty()) {
            VisualLine line;
            if (!first_prefix.empty()) {
                line.segments.push_back({first_prefix, prefix_style});
            }
            lines_.push_back(std::move(line));
            return;
        }
        bool first = true;
        for (auto& wrapped : wrapped_lines) {
            VisualLine line;
            const std::string& prefix = first ? first_prefix : cont_prefix;
            if (!prefix.empty()) {
                line.segments.push_back({prefix, prefix_style});
            }
            for (auto& atom : wrapped) {
                if (!line.segments.empty() && same_style(line.segments.back().style, atom.style)) {
                    line.segments.back().text += atom.glyph;
                } else {
                    line.segments.push_back({atom.glyph, atom.style});
                }
            }
            lines_.push_back(std::move(line));
            first = false;
        }
    };

    const auto atoms_for = [&](const std::vector<MdSpan>& spans, Style base) {
        std::vector<Atom> atoms;
        for (const auto& span : spans) {
            Style style = span_style(span.flags);
            if (span.flags.code || span.flags.link) {
                style.bold = style.bold || base.bold;
            } else {
                merge_colors(style, base);
                style.bold = style.bold || base.bold;
                style.dim = style.dim || base.dim;
                style.italic = style.italic || base.italic;
            }
            for (const TextGlyph& glyph : text_glyph_breaks(span.text)) {
                atoms.push_back({std::string(span.text.substr(glyph.offset, glyph.length)), glyph.width, style});
            }
        }
        return atoms;
    };

    switch (block.kind) {
    case MdBlockKind::CodeBlock: {
        for (const std::string& raw : block.code_lines) {
            std::string expanded;
            for (const char c : raw) {
                expanded += c == '\t' ? "    " : std::string(1, c);
            }
            const std::size_t bytes = text_byte_length_for_width(expanded, width);
            VisualLine line;
            line.code_block = true;
            line.segments.push_back({expanded.substr(0, bytes), styles_.code_block});
            lines_.push_back(std::move(line));
        }
        return;
    }

    case MdBlockKind::ThematicBreak: {
        VisualLine line;
        const std::string glyph = unicode_ ? "\xE2\x94\x80" : "-"; // ─
        line.segments.push_back({repeat_glyph(glyph, width), styles_.thematic_break});
        lines_.push_back(std::move(line));
        return;
    }

    case MdBlockKind::Heading: {
        const Style base = block.level <= 2 ? styles_.heading : styles_.heading_sub;
        emit(atoms_for(block.spans, base), "", "", base);
        return;
    }

    case MdBlockKind::Quote: {
        const std::string bar = unicode_ ? "\xE2\x96\x8E " : "| "; // ▎
        const int depth = std::max(1, block.level);
        std::string prefix;
        for (int d = 0; d < depth; ++d) {
            prefix += bar;
        }
        emit(atoms_for(block.spans, styles_.quote_text), prefix, prefix, styles_.quote_bar);
        return;
    }

    case MdBlockKind::ListItem: {
        const int depth = std::max(0, block.level);
        const std::string bullet =
            block.ordered ? std::to_string(block.number) + "." : (unicode_ ? "\xE2\x80\xA2" : "-"); // •
        const std::string first_prefix = repeat_glyph("  ", depth) + bullet + " ";
        const std::string cont_prefix(static_cast<std::size_t>(text_display_width(first_prefix)), ' ');
        emit(atoms_for(block.spans, styles_.text), first_prefix, cont_prefix, styles_.list_marker);
        return;
    }

    case MdBlockKind::Paragraph: {
        if (options_.render_images && block.spans.size() == 1 && block.spans[0].flags.image &&
            !block.spans[0].link_target.empty()) {
            append_image_lines(block.spans[0]);
            return;
        }
        emit(atoms_for(block.spans, styles_.text), "", "", styles_.text);
        return;
    }
    }
}

std::shared_ptr<const AnimatedImage> MarkdownView::cached_image(const std::string& path) {
    const auto it = image_cache_.find(path);
    if (it != image_cache_.end()) {
        return it->second;
    }
    std::shared_ptr<const AnimatedImage> image;
    if (auto gif = AnimatedImage::load_gif(path)) {
        image = std::make_shared<const AnimatedImage>(std::move(*gif));
    } else if (auto png = TerminalImage::load_png(path)) {
        image = std::make_shared<const AnimatedImage>(AnimatedImage::from_image(std::move(*png)));
    } else if (auto ppm = TerminalImage::load_ppm(path)) {
        image = std::make_shared<const AnimatedImage>(AnimatedImage::from_image(std::move(*ppm)));
    }
    image_cache_.emplace(path, image); // cache failures too: no reload per rewrap
    return image;
}

void MarkdownView::append_image_lines(const MdSpan& span) {
    const int width = std::max(4, bounds_.width);
    const auto image = cached_image(span.link_target);

    if (!image) {
        VisualLine line; // missing file: keep the styled alt-text fallback
        line.segments.push_back({span.text.empty() ? "[image: " + span.link_target + "]" : span.text,
                                 span_style(span.flags)});
        lines_.push_back(std::move(line));
        return;
    }

    // Terminal cells are ~1:2 (w:h), so 8px per column and 16px per row.
    int cols = std::clamp(image->width() / 8, 4, width);
    int rows = std::max(1, image->height() * cols / (image->width() * 2));
    if (rows > options_.max_image_rows) {
        rows = std::max(1, options_.max_image_rows);
        cols = std::clamp(image->width() * 2 * rows / std::max(1, image->height()), 4, width);
    }

    VisualLine anchor;
    anchor.image = std::move(image);
    anchor.image_rows = rows;
    anchor.image_cols = cols;
    lines_.push_back(std::move(anchor));
    for (int i = 1; i < rows; ++i) {
        lines_.push_back(VisualLine{}); // spacer rows covered by the image
    }
}

void MarkdownView::rewrap() {
    lines_.clear();
    last_block_kind_ = MdBlockKind::Paragraph;
    has_animated_image_ = false;
    if (bounds_.width <= 0) {
        drawn_frame_.clear();
        return;
    }
    bool first_block = true;
    for (const auto& block : blocks_) {
        append_block_lines(block, first_block);
        first_block = false;
    }
    for (const auto& line : lines_) {
        if (line.image && line.image->animated()) {
            has_animated_image_ = true;
            break;
        }
    }
    drawn_frame_.assign(lines_.size(), -1);
}

void MarkdownView::on_idle() {
    if (!has_animated_image_ || !anim_started_ || !on_dirty_) {
        return;
    }
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - anim_start_);

    Rect dirty;
    bool any = false;
    for (int i = scroll_y_; i < static_cast<int>(lines_.size()) && i < scroll_y_ + bounds_.height; ++i) {
        const VisualLine& line = lines_[static_cast<std::size_t>(i)];
        if (!line.image || !line.image->animated()) {
            continue;
        }
        const int row = i - scroll_y_;
        if (row + line.image_rows > bounds_.height) {
            continue; // partially visible images are not painted
        }
        const int frame = line.image->frame_index_at(elapsed);
        if (drawn_frame_[static_cast<std::size_t>(i)] != frame) {
            const Rect rows{0, row, bounds_.width, line.image_rows};
            dirty = any ? unite(dirty, rows) : rows;
            any = true;
        }
    }
    if (any) {
        on_dirty_(dirty);
    }
}

void MarkdownView::clamp_scroll() { scroll_y_ = std::clamp(scroll_y_, 0, max_scroll_y()); }

void MarkdownView::paint(PaintContext& ctx) const {
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }
    paint_bounds_background(ctx, styles_.text);

    Canvas& canvas = ctx.canvas;
    for (int row = 0; row < bounds_.height; ++row) {
        const int line_index = scroll_y_ + row;
        if (line_index >= static_cast<int>(lines_.size())) {
            break;
        }
        const VisualLine& line = lines_[static_cast<std::size_t>(line_index)];
        if (line.image) {
            // Graphics protocols cannot half-draw an image: only place it when
            // all reserved rows are on screen, otherwise clear stale graphics.
            if (row + line.image_rows <= bounds_.height) {
                if (!anim_started_) {
                    anim_start_ = std::chrono::steady_clock::now();
                    anim_started_ = true;
                }
                const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - anim_start_);
                const int frame = line.image->frame_index_at(elapsed);
                canvas.draw_image({0, row}, {line.image_cols, line.image_rows},
                                  line.image->frames()[static_cast<std::size_t>(frame)].image);
                drawn_frame_[static_cast<std::size_t>(line_index)] = frame;
            }
            continue;
        }
        if (line.code_block) {
            canvas.fill_rect({0, row, bounds_.width, 1}, ' ', styles_.code_block_fill);
        }
        int x = 0;
        for (const VisualSegment& segment : line.segments) {
            if (x >= bounds_.width) {
                break;
            }
            const std::size_t bytes = text_byte_length_for_width(segment.text, bounds_.width - x);
            const std::string_view visible = std::string_view(segment.text).substr(0, bytes);
            canvas.draw_text({x, row}, visible, segment.style);
            x += text_display_width(visible);
        }
    }
}

} // namespace tuinator
