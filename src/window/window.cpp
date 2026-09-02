#include <tuinator/window/window.hpp>

#include <algorithm>
#include <variant>

namespace tuinator {

namespace {

constexpr int kMinWidth = 12;
constexpr int kMinHeight = 5;

Style border_style() {
    Style style{};
    style.foreground = Color::Cyan;
    return style;
}

Style title_style() {
    Style style{};
    style.foreground = Color::White;
    style.background = Color::Blue;
    style.bold = true;
    return style;
}

} // namespace

Window::Window(std::string title, Rect bounds, std::unique_ptr<Widget> content, WindowOptions options)
    : title_(std::move(title)),
      options_(options),
      content_(std::move(content)),
      frame_bounds_(bounds) {}

void Window::set_bounds(Rect bounds) {
    frame_bounds_ = {
        bounds.x,
        bounds.y,
        std::max(kMinWidth, bounds.width),
        std::max(kMinHeight, bounds.height),
    };
    layout(frame_bounds_);
    mark_dirty();
}

void Window::set_on_close(std::function<void()> callback) {
    on_close_ = std::move(callback);
}

Size Window::preferred_size() const {
    if (!content_) {
        return {kMinWidth, kMinHeight};
    }

    const Size content_size = content_->preferred_size();
    return {
        std::max(kMinWidth, content_size.width + 2),
        std::max(kMinHeight, content_size.height + 2),
    };
}

Rect Window::content_area_in_window() const {
    return {
        1,
        1,
        std::max(0, frame_bounds_.width - 2),
        std::max(0, frame_bounds_.height - 2),
    };
}

Rect Window::content_bounds_local() const {
    return {{0, 0}, content_area_in_window().size()};
}

void Window::layout(Rect bounds) {
    frame_bounds_ = {
        bounds.x,
        bounds.y,
        std::max(kMinWidth, bounds.width),
        std::max(kMinHeight, bounds.height),
    };
    bounds_ = {{0, 0}, frame_bounds_.size()};

    if (content_) {
        content_->layout(content_bounds_local());
    }
}

void Window::paint(Canvas& canvas) const {
    const int width = frame_bounds_.width;
    const int height = frame_bounds_.height;
    if (width < 2 || height < 2) {
        return;
    }

    const Style border = border_style();
    canvas.draw_box({0, 0, width, height}, border);

    const std::string title = " " + title_ + " ";
    int x = 1;
    for (const char ch : title) {
        if (x >= width - 1) {
            break;
        }
        canvas.draw_char({x, 0}, ch, title_style());
        ++x;
    }

    if (options_.resizable && width >= 3 && height >= 3) {
        canvas.draw_text({width - 2, height - 1}, canvas.glyphs().resize_handle, border);
    }

    const Rect content_area = content_area_in_window();
    if (content_area.width > 0 && content_area.height > 0) {
        canvas.fill_rect(content_area, ' ', Style{});
    }

    if (content_) {
        canvas.with_clip(content_area, [&](Canvas& clipped) {
            content_->paint(clipped);
        });
    }
}

void Window::collect_focusable(std::vector<Widget*>& out) {
    if (content_) {
        content_->collect_focusable(out);
    }
}

bool Window::handle_event(const Event& event) {
    if (!content_) {
        return false;
    }

    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        const Rect area = content_area_in_window();
        if (content_->captures_pointer() || area.contains(mouse->position)) {
            MouseEvent adjusted = *mouse;
            adjusted.position = {
                mouse->position.x - area.x,
                mouse->position.y - area.y,
            };
            return content_->handle_event(adjusted);
        }

        return false;
    }

    return content_->handle_event(event);
}

bool Window::captures_pointer() const {
    return content_ && content_->captures_pointer();
}

Widget* Window::hit_test_focusable(Point point) {
    if (!frame_bounds_.contains(point)) {
        return nullptr;
    }

    const Point local{point.x - frame_bounds_.x, point.y - frame_bounds_.y};
    const Rect area = content_area_in_window();
    if (!area.contains(local)) {
        return nullptr;
    }

    const Point content_point{local.x - area.x, local.y - area.y};
    return content_ ? content_->hit_test_focusable(content_point) : nullptr;
}

bool Window::contains_point(Point point) const {
    return frame_bounds_.contains(point);
}

bool Window::is_title_bar(Point local_point) const {
    return local_point.y == 0
        && local_point.x >= 0
        && local_point.x < frame_bounds_.width;
}

bool Window::is_resize_handle(Point local_point) const {
    if (!options_.resizable) {
        return false;
    }

    return local_point.x >= frame_bounds_.width - 3
        && local_point.y >= frame_bounds_.height - 2;
}

} // namespace tuinator
