#pragma once

#include <tuinator/widgets/widget.hpp>

#include <functional>
#include <memory>
#include <string>

namespace tuinator {

struct WindowOptions {
    bool modal = false;
    bool movable = true;
    bool resizable = true;
    bool closable = true;
};

class Window : public Widget {
public:
    Window(std::string title, Rect bounds, std::unique_ptr<Widget> content, WindowOptions options = {});

    const std::string& title() const { return title_; }
    Rect frame_bounds() const { return frame_bounds_; }

    bool is_modal() const { return options_.modal; }
    bool is_closable() const { return options_.closable; }
    bool is_movable() const { return options_.movable; }
    bool is_resizable() const { return options_.resizable; }

    int z_index() const { return z_index_; }
    void set_z_index(int z_index) { z_index_ = z_index; }

    void set_bounds(Rect bounds);
    void set_on_close(std::function<void()> callback);

    Widget* content() const { return content_.get(); }

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(Canvas& canvas) const override;
    void collect_focusable(std::vector<Widget*>& out) override;
    bool handle_event(const Event& event) override;
    bool captures_pointer() const override;
    Widget* hit_test_focusable(Point point) override;

    bool contains_point(Point point) const;
    bool is_title_bar(Point local_point) const;
    bool is_resize_handle(Point local_point) const;

    Rect content_bounds_local() const;
    Rect content_area_in_window() const;

private:
    std::string title_;
    WindowOptions options_;
    std::unique_ptr<Widget> content_;
    Rect frame_bounds_{};
    int z_index_ = 0;
    std::function<void()> on_close_;
};

} // namespace tuinator
