#include <tuinator/widgets/chrome/status_bar.hpp>

#include <tuinator/render/text.hpp>

#include <algorithm>
#include <string>

namespace tuinator {

StatusBar::StatusBar(std::string text, Style style)
    : text_(std::move(text)), style_(style) {}

void StatusBar::set_text(std::string text) {
    text_ = std::move(text);
    mark_dirty();
}

Size StatusBar::preferred_size() const {
    return {std::max(1, text_display_width(text_)), 1};
}

void StatusBar::paint(Canvas& canvas) const {
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    Style bar_style = style_;
    if (bar_style.background == Color::Default && bar_style.foreground == Color::Default) {
        bar_style.background = Color::Blue;
        bar_style.foreground = Color::White;
    }

    const std::string fill(static_cast<std::size_t>(bounds_.width), ' ');
    canvas.draw_text({0, 0}, fill, bar_style);

    const int max_width = bounds_.width;
    const std::string clipped = text_.substr(0, static_cast<std::size_t>(std::max(0, max_width)));
    canvas.draw_text({0, 0}, clipped, bar_style);
}

} // namespace tuinator
