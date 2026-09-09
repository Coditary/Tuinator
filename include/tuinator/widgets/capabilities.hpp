#pragma once

#include <tuinator/core/event.hpp>

#include <functional>
#include <vector>

#include <tuinator/widgets/capabilities/widget_roles.hpp>

namespace tuinator {

class Widget;

/// Scrollable viewport contract used by Application for focus, keys, and pointer capture.
class Scrollable {
  public:
    virtual ~Scrollable() = default;

    virtual int scroll_x() const = 0;
    virtual int scroll_y() const = 0;
    virtual bool contains_widget(const Widget* widget) const = 0;
    virtual void ensure_visible(const Widget* widget) = 0;
    virtual bool try_scroll(const Event& event) = 0;
    virtual void scroll_by(int dx, int dy) = 0;
    virtual Widget* scroll_content() const = 0;
};

/// Tab container contract used by Application for numeric tab shortcuts.
class TabHost {
  public:
    virtual ~TabHost() = default;

    virtual int tab_count() const = 0;
    virtual void activate_tab(int index) = 0;
};

Scrollable* as_scrollable(Widget* widget);
const Scrollable* as_scrollable(const Widget* widget);
TabHost* as_tab_host(Widget* widget);
const TabHost* as_tab_host(const Widget* widget);

bool walk_subtree(Widget* root, const std::function<bool(Widget*)>& visitor);

void ensure_focus_visible(Widget* root, Widget* target);

Scrollable* find_scrollable_for_widget(Widget* root, const Widget* target);
Scrollable* find_first_scrollable(Widget* root);
bool collect_scrollables_for_widget(Widget* root, const Widget* target, std::vector<Scrollable*>& out);

Widget* find_pointer_active_widget(Widget* root);
Widget* find_keyboard_capture_widget(Widget* root);
MouseEvent adjust_mouse_for_widget(Widget* root, Widget* target, const MouseEvent& mouse);

bool dispatch_keyboard_capture(Widget* root, const Event& event);
bool dispatch_scroll_keys(Widget* root, Widget* focused, const Event& event);
bool dispatch_tab_number(Widget* root, int index);

} // namespace tuinator
