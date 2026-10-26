#include <tuinator/testing/ui_tree.hpp>

#include <tuinator/backend/memory_backend.hpp>
#include <tuinator/testing/cell_styles.hpp>
#include <tuinator/testing/widget_query.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>

#include <memory>
#include <sstream>
#include <string>

namespace tuinator::testing {

namespace {

void append_json_string(std::ostringstream& out, std::string_view value) {
    out << '"';
    for (const char ch : value) {
        switch (ch) {
        case '"':
            out << "\\\"";
            break;
        case '\\':
            out << "\\\\";
            break;
        case '\n':
            out << "\\n";
            break;
        case '\r':
            out << "\\r";
            break;
        case '\t':
            out << "\\t";
            break;
        default:
            if (static_cast<unsigned char>(ch) < 0x20) {
                out << "\\u00";
                const char hex[] = "0123456789abcdef";
                out << hex[(ch >> 4) & 0xF] << hex[ch & 0xF];
            } else {
                out << ch;
            }
            break;
        }
    }
    out << '"';
}

void append_capabilities(std::ostringstream& out, const Widget& widget) {
    out << '[';
    bool first = true;
    for (const WidgetCapability capability : widget_capabilities(widget)) {
        if (!first) {
            out << ',';
        }
        first = false;
        out << '"';
        switch (capability) {
        case WidgetCapability::Scrollable: out << "scrollable"; break;
        case WidgetCapability::TabHost: out << "tab_host"; break;
        case WidgetCapability::SingleChildContainer: out << "single_child"; break;
        case WidgetCapability::BinaryContainer: out << "binary_container"; break;
        case WidgetCapability::BorderedPane: out << "bordered_pane"; break;
        case WidgetCapability::Pane: out << "pane"; break;
        case WidgetCapability::LayoutBox: out << "layout_box"; break;
        case WidgetCapability::Splittable: out << "splittable"; break;
        case WidgetCapability::TextDisplay: out << "text_display"; break;
        case WidgetCapability::TextInputField: out << "text_input"; break;
        case WidgetCapability::MultiLineTextInput: out << "multiline_text"; break;
        case WidgetCapability::Activatable: out << "activatable"; break;
        case WidgetCapability::ToggleControl: out << "toggle"; break;
        case WidgetCapability::SelectableList: out << "selectable_list"; break;
        case WidgetCapability::ValueControl: out << "value_control"; break;
        }
        out << '"';
    }
    out << ']';
}

void append_state_fields(std::ostringstream& out, const Widget& widget) {
    if (const auto* text = as_text_display(&widget)) {
        out << ",\"text\":";
        append_json_string(out, text->display_text());
    }
    if (const auto* field = as_text_input_field(&widget)) {
        out << ",\"value\":";
        append_json_string(out, field->field_value());
        if (!field->field_placeholder().empty()) {
            out << ",\"placeholder\":";
            append_json_string(out, field->field_placeholder());
        }
    }
    if (const auto* toggle = as_toggle_control(&widget)) {
        out << ",\"checked\":" << (toggle->is_checked() ? "true" : "false");
    }
    if (const auto* list = as_selectable_list(&widget)) {
        out << ",\"selected_index\":" << list->selected_index();
    }
    if (const auto* value = as_value_control(&widget)) {
        out << ",\"value_current\":" << value->value_current();
        out << ",\"value_min\":" << value->value_minimum();
        out << ",\"value_max\":" << value->value_maximum();
    }
}

void append_widget_json(std::ostringstream& out, const Widget& widget, int depth, const MemoryTerminalBackend* render,
                        bool include_render_styles) {
    out << '{';
    out << "\"type\":";
    append_json_string(out, widget.widget_type_name());

    const std::string_view selector = selector_for(widget);
    if (!selector.empty()) {
        out << ",\"id\":";
        append_json_string(out, selector);
    }
    if (!widget.test_id().empty() && widget.test_id() != widget.widget_id()) {
        out << ",\"test_id\":";
        append_json_string(out, widget.test_id());
        if (!widget.widget_id().empty()) {
            out << ",\"widget_id\":";
            append_json_string(out, widget.widget_id());
        }
    }

    const Rect bounds = widget.bounds();
    out << ",\"bounds\":[" << bounds.x << ',' << bounds.y << ',' << bounds.width << ',' << bounds.height << ']';
    out << ",\"focused\":" << (widget.is_focused() ? "true" : "false");
    out << ",\"enabled\":" << (widget.is_enabled() ? "true" : "false");
    out << ",\"hovered\":" << (widget.is_hovered() ? "true" : "false");
    out << ",\"capabilities\":";
    append_capabilities(out, widget);
    append_state_fields(out, widget);

    if (include_render_styles && render != nullptr) {
        const RegionStyleAnalysis analysis = analyze_region(*render, widget.bounds());
        out << ",\"render\":" << region_style_json(analysis);
    }

    std::ostringstream children;
    children << '[';
    bool first_child = true;
    for (const std::unique_ptr<Widget>& child : widget.children()) {
        if (!first_child) {
            children << ',';
        }
        first_child = false;
        append_widget_json(children, *child, depth + 1, render, include_render_styles);
    }
    children << ']';

    out << ",\"children\":" << children.str();
    out << '}';
}

} // namespace

std::string build_ui_tree_json(const Widget& root, UiTreeOptions options, const MemoryTerminalBackend* render) {
    std::ostringstream out;
    out << '{';
    out << "\"version\":1";
    out << ",\"root\":";
    append_widget_json(out, root, 0, render, options.include_render_styles);
    if (options.include_screen_text && options.screen_text.has_value() && !options.screen_text->empty()) {
        const std::size_t limit = static_cast<std::size_t>(std::max(0, options.max_screen_text_chars));
        std::string_view text = *options.screen_text;
        if (text.size() > limit) {
            text = text.substr(0, limit);
        }
        out << ",\"screen_text\":";
        append_json_string(out, text);
    }
    out << '}';
    return out.str();
}

} // namespace tuinator::testing
