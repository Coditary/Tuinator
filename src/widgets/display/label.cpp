#include <tuinator/widgets/display/label.hpp>

#include <tuinator/render/text.hpp>

#include <algorithm>

namespace tuinator {

Label::Label(std::string text, Style style)
    : text_(std::move(text)), style_(style) {}

void Label::set_text(std::string text) {
    text_ = std::move(text);
    mark_dirty();
}

void Label::set_style(Style style) {
    style_ = style;
    mark_dirty();
}

Size Label::preferred_size() const {
    int max_width = 0;
    int lines = 0;
    std::size_t start = 0;

    while (start <= text_.size()) {
        std::size_t end = text_.find('\n', start);
        if (end == std::string::npos) {
            end = text_.size();
        }

        max_width = std::max(
            max_width,
            text_display_width(std::string_view(text_.data() + start, end - start)));
        ++lines;

        if (end >= text_.size()) {
            break;
        }

        start = end + 1;
    }

    return {max_width, std::max(1, lines)};
}

void Label::layout(Rect bounds) {
    bounds_ = bounds;
}

void Label::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (text_.empty() || bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    const Size text_size = preferred_size();
    const int origin_y = (bounds_.height - text_size.height) / 2;

    int y = origin_y;
    std::size_t start = 0;
    while (start <= text_.size() && y < bounds_.height) {
        std::size_t end = text_.find('\n', start);
        if (end == std::string::npos) {
            end = text_.size();
        }

        const std::string_view line(text_.data() + start, end - start);
        const int line_width = text_display_width(line);
        const int x = std::max(0, (bounds_.width - line_width) / 2);
        const int max_columns = std::max(0, bounds_.width - x);
        if (max_columns <= 0) {
            break;
        }
        const std::size_t byte_length = text_byte_length_for_width(line, max_columns);
        if (byte_length == 0) {
            break;
        }
        canvas.draw_text({x, y}, line.substr(0, byte_length), style_);

        if (end >= text_.size()) {
            break;
        }

        ++y;
        start = end + 1;
    }
}

} // namespace tuinator
