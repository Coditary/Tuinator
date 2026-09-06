#include <tuinator/widgets/capabilities.hpp>
#include <tuinator/widgets/widget.hpp>

namespace tuinator {

Scrollable* as_scrollable(Widget* widget) { return dynamic_cast<Scrollable*>(widget); }

const Scrollable* as_scrollable(const Widget* widget) { return dynamic_cast<const Scrollable*>(widget); }

TabHost* as_tab_host(Widget* widget) { return dynamic_cast<TabHost*>(widget); }

const TabHost* as_tab_host(const Widget* widget) { return dynamic_cast<const TabHost*>(widget); }

bool walk_subtree(Widget* root, const std::function<bool(Widget*)>& visitor) {
    if (root == nullptr) {
        return true;
    }

    if (!visitor(root)) {
        return false;
    }

    bool continue_walk = true;
    root->for_each_child([&](Widget* child) {
        if (continue_walk) {
            continue_walk = walk_subtree(child, visitor);
        }
    });
    return continue_walk;
}

void ensure_focus_visible(Widget* node, Widget* target) {
    if (node == nullptr || target == nullptr) {
        return;
    }

    if (Scrollable* scroll = as_scrollable(node)) {
        if (scroll->contains_widget(target)) {
            scroll->ensure_visible(target);
            return;
        }

        if (Widget* content = scroll->scroll_content()) {
            ensure_focus_visible(content, target);
        }
        return;
    }

    node->for_each_child([&](Widget* child) { ensure_focus_visible(child, target); });
}

Scrollable* find_scrollable_for_widget(Widget* node, const Widget* target) {
    if (node == nullptr || target == nullptr) {
        return nullptr;
    }

    if (Scrollable* scroll = as_scrollable(node)) {
        if (scroll->contains_widget(target)) {
            return scroll;
        }

        if (Widget* content = scroll->scroll_content()) {
            if (Scrollable* nested = find_scrollable_for_widget(content, target)) {
                return nested;
            }
        }
        return nullptr;
    }

    Scrollable* found = nullptr;
    node->for_each_child([&](Widget* child) {
        if (found == nullptr) {
            found = find_scrollable_for_widget(child, target);
        }
    });
    return found;
}

bool collect_scrollables_for_widget(Widget* node, const Widget* target, std::vector<Scrollable*>& out) {
    if (node == nullptr || target == nullptr) {
        return false;
    }

    if (node == target) {
        return true;
    }

    if (Scrollable* scroll = as_scrollable(node)) {
        if (Widget* content = scroll->scroll_content()) {
            if (collect_scrollables_for_widget(content, target, out)) {
                out.push_back(scroll);
                return true;
            }
        }
        return false;
    }

    bool found = false;
    node->for_each_child([&](Widget* child) {
        if (!found && collect_scrollables_for_widget(child, target, out)) {
            found = true;
        }
    });
    return found;
}

Scrollable* find_first_scrollable(Widget* node) {
    if (node == nullptr) {
        return nullptr;
    }

    if (Scrollable* scroll = as_scrollable(node)) {
        return scroll;
    }

    Scrollable* found = nullptr;
    node->for_each_child([&](Widget* child) {
        if (found == nullptr) {
            found = find_first_scrollable(child);
        }
    });
    return found;
}

Widget* find_pointer_active_widget(Widget* node) {
    if (node == nullptr) {
        return nullptr;
    }

    if (node->pointer_active()) {
        return node;
    }

    Widget* found = nullptr;
    node->for_each_child([&](Widget* child) {
        if (found == nullptr) {
            found = find_pointer_active_widget(child);
        }
    });
    return found;
}

MouseEvent adjust_mouse_for_widget(Widget* root, Widget* target, const MouseEvent& mouse) {
    MouseEvent adjusted = mouse;
    std::vector<Scrollable*> scroll_chain;
    collect_scrollables_for_widget(root, target, scroll_chain);
    for (Scrollable* scroll : scroll_chain) {
        auto* widget = dynamic_cast<Widget*>(scroll);
        if (widget == nullptr) {
            continue;
        }
        adjusted.position.x = adjusted.position.x - widget->bounds().x + scroll->scroll_x();
        adjusted.position.y = adjusted.position.y - widget->bounds().y + scroll->scroll_y();
    }
    return adjusted;
}

bool dispatch_scroll_keys(Widget* root, Widget* focused, const Event& event) {
    Scrollable* scroll = find_scrollable_for_widget(root, focused);
    if (scroll == nullptr) {
        scroll = find_first_scrollable(root);
    }

    return scroll != nullptr && scroll->try_scroll(event);
}

bool dispatch_tab_number(Widget* root, int index) {
    if (root == nullptr || index < 0) {
        return false;
    }

    TabHost* tabs = nullptr;
    walk_subtree(root, [&](Widget* node) {
        if (tabs != nullptr) {
            return false;
        }

        tabs = as_tab_host(node);
        return tabs == nullptr;
    });

    if (tabs == nullptr || index >= tabs->tab_count()) {
        return false;
    }

    tabs->activate_tab(index);
    return true;
}

} // namespace tuinator
