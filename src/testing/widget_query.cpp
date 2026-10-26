#include <tuinator/testing/widget_query.hpp>

#include <stdexcept>

namespace tuinator::testing {

namespace {

bool matches_selector(const Widget& widget, std::string_view selector) {
    if (selector.empty()) {
        return false;
    }
    if (!widget.test_id().empty() && widget.test_id() == selector) {
        return true;
    }
    return widget.widget_id() == selector;
}

void collect_matches(Widget& widget, std::string_view selector, std::vector<Widget*>& out) {
    if (matches_selector(widget, selector)) {
        out.push_back(&widget);
    }
    widget.for_each_descendant([&](Widget* child) {
        if (matches_selector(*child, selector)) {
            out.push_back(child);
        }
    });
}

} // namespace

std::string_view selector_for(const Widget& widget) {
    if (!widget.test_id().empty()) {
        return widget.test_id();
    }
    return widget.widget_id();
}

Widget* find_by_selector(Widget& root, std::string_view selector) {
    std::vector<Widget*> matches;
    collect_matches(root, selector, matches);
    return matches.empty() ? nullptr : matches.front();
}

const Widget* find_by_selector(const Widget& root, std::string_view selector) {
    return find_by_selector(const_cast<Widget&>(root), selector);
}

std::vector<Widget*> find_all_by_selector(Widget& root, std::string_view selector) {
    std::vector<Widget*> matches;
    collect_matches(root, selector, matches);
    return matches;
}

Widget* require_by_selector(Widget& root, std::string_view selector) {
    Widget* widget = find_by_selector(root, selector);
    if (widget == nullptr) {
        throw std::runtime_error(std::string("No widget with test selector: ") + std::string(selector));
    }
    return widget;
}

} // namespace tuinator::testing
