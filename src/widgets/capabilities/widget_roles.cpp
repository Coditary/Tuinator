#include <tuinator/widgets/capabilities/widget_roles.hpp>

#include <tuinator/render/style_resolver.hpp>
#include <tuinator/widgets/capabilities.hpp>
#include <tuinator/widgets/controls/button.hpp>
#include <tuinator/widgets/controls/checkbox.hpp>
#include <tuinator/widgets/controls/combo_box.hpp>
#include <tuinator/widgets/controls/slider.hpp>
#include <tuinator/widgets/controls/text_area.hpp>
#include <tuinator/widgets/controls/text_input.hpp>
#include <tuinator/widgets/controls/toggle.hpp>
#include <tuinator/widgets/containers/panel.hpp>
#include <tuinator/widgets/containers/scroll_view.hpp>
#include <tuinator/widgets/containers/split_pane.hpp>
#include <tuinator/widgets/containers/tabs.hpp>
#include <tuinator/widgets/display/label.hpp>
#include <tuinator/widgets/display/progress_bar.hpp>
#include <tuinator/widgets/views/list_view.hpp>
#include <tuinator/widgets/views/table.hpp>
#include <tuinator/widgets/views/tree_view.hpp>
#include <tuinator/layout/box.hpp>
#include <tuinator/layout/grid.hpp>
#include <tuinator/widgets/widget.hpp>

#include <algorithm>

namespace tuinator {

namespace {

template <typename Interface, typename WidgetType>
Interface* try_as(Widget* widget) {
    return dynamic_cast<WidgetType*>(widget);
}

template <typename Interface, typename WidgetType>
const Interface* try_as(const Widget* widget) {
    return dynamic_cast<const WidgetType*>(widget);
}

} // namespace

SingleChildContainer* as_single_child_container(Widget* widget) {
    return try_as<SingleChildContainer, Panel>(widget) != nullptr
               ? try_as<SingleChildContainer, Panel>(widget)
               : try_as<SingleChildContainer, ScrollView>(widget);
}

const SingleChildContainer* as_single_child_container(const Widget* widget) {
    return try_as<SingleChildContainer, Panel>(widget) != nullptr
               ? try_as<SingleChildContainer, Panel>(widget)
               : try_as<SingleChildContainer, ScrollView>(widget);
}

BinaryContainer* as_binary_container(Widget* widget) {
    return try_as<BinaryContainer, SplitPane>(widget);
}

const BinaryContainer* as_binary_container(const Widget* widget) {
    return try_as<BinaryContainer, SplitPane>(widget);
}

BorderedPane* as_bordered_pane(Widget* widget) { return try_as<BorderedPane, Panel>(widget); }

const BorderedPane* as_bordered_pane(const Widget* widget) {
    return try_as<BorderedPane, Panel>(widget);
}

Pane* as_pane(Widget* widget) { return try_as<Pane, Panel>(widget); }

const Pane* as_pane(const Widget* widget) { return try_as<Pane, Panel>(widget); }

LayoutBox* as_layout_box(Widget* widget) {
    if (auto* box = try_as<LayoutBox, VBox>(widget)) {
        return box;
    }
    if (auto* box = try_as<LayoutBox, HBox>(widget)) {
        return box;
    }
    return try_as<LayoutBox, Grid>(widget);
}

const LayoutBox* as_layout_box(const Widget* widget) {
    if (const auto* box = try_as<LayoutBox, VBox>(widget)) {
        return box;
    }
    if (const auto* box = try_as<LayoutBox, HBox>(widget)) {
        return box;
    }
    return try_as<LayoutBox, Grid>(widget);
}

Splittable* as_splittable(Widget* widget) { return try_as<Splittable, SplitPane>(widget); }

const Splittable* as_splittable(const Widget* widget) { return try_as<Splittable, SplitPane>(widget); }

TextDisplay* as_text_display(Widget* widget) { return try_as<TextDisplay, Label>(widget); }

const TextDisplay* as_text_display(const Widget* widget) { return try_as<TextDisplay, Label>(widget); }

TextInputField* as_text_input_field(Widget* widget) { return try_as<TextInputField, TextInput>(widget); }

const TextInputField* as_text_input_field(const Widget* widget) {
    return try_as<TextInputField, TextInput>(widget);
}

Activatable* as_activatable(Widget* widget) { return try_as<Activatable, Button>(widget); }

const Activatable* as_activatable(const Widget* widget) { return try_as<Activatable, Button>(widget); }

ToggleControl* as_toggle_control(Widget* widget) {
    if (auto* control = try_as<ToggleControl, Checkbox>(widget)) {
        return control;
    }
    return try_as<ToggleControl, Toggle>(widget);
}

const ToggleControl* as_toggle_control(const Widget* widget) {
    if (const auto* control = try_as<ToggleControl, Checkbox>(widget)) {
        return control;
    }
    return try_as<ToggleControl, Toggle>(widget);
}

SelectableList* as_selectable_list(Widget* widget) {
    if (auto* list = try_as<SelectableList, ListView>(widget)) {
        return list;
    }
    if (auto* list = try_as<SelectableList, Table>(widget)) {
        return list;
    }
    if (auto* list = try_as<SelectableList, ComboBox>(widget)) {
        return list;
    }
    return try_as<SelectableList, TreeView>(widget);
}

const SelectableList* as_selectable_list(const Widget* widget) {
    if (const auto* list = try_as<SelectableList, ListView>(widget)) {
        return list;
    }
    if (const auto* list = try_as<SelectableList, Table>(widget)) {
        return list;
    }
    if (const auto* list = try_as<SelectableList, ComboBox>(widget)) {
        return list;
    }
    return try_as<SelectableList, TreeView>(widget);
}

MultiLineTextInput* as_multiline_text_input(Widget* widget) {
    return try_as<MultiLineTextInput, TextArea>(widget);
}

const MultiLineTextInput* as_multiline_text_input(const Widget* widget) {
    return try_as<MultiLineTextInput, TextArea>(widget);
}

ValueControl* as_value_control(Widget* widget) {
    if (auto* control = try_as<ValueControl, Slider>(widget)) {
        return control;
    }
    return try_as<ValueControl, ComboBox>(widget);
}

const ValueControl* as_value_control(const Widget* widget) {
    if (const auto* control = try_as<ValueControl, Slider>(widget)) {
        return control;
    }
    return try_as<ValueControl, ComboBox>(widget);
}

bool widget_matches_pseudo(const Widget& widget, WidgetPseudoState pseudo) {
    switch (pseudo) {
    case WidgetPseudoState::Focused:
        return widget.is_focused();
    case WidgetPseudoState::Selected:
        if (const auto* list = as_selectable_list(const_cast<Widget*>(&widget))) {
            return list->selected_index() >= 0;
        }
        return false;
    case WidgetPseudoState::Checked:
        if (const auto* toggle = as_toggle_control(const_cast<Widget*>(&widget))) {
            return toggle->is_checked();
        }
        return false;
    case WidgetPseudoState::Disabled:
        return !widget.is_enabled();
    case WidgetPseudoState::Open:
        return widget.is_dropdown_open();
    case WidgetPseudoState::Hover:
        return widget.is_hovered();
    }
    return false;
}

bool widget_has_capability(const Widget& widget, WidgetCapability capability) {
    return std::find(widget_capabilities(widget).begin(), widget_capabilities(widget).end(), capability) !=
           widget_capabilities(widget).end();
}

std::vector<WidgetCapability> widget_capabilities(const Widget& widget) {
    std::vector<WidgetCapability> capabilities;
    const Widget* ptr = &widget;

    if (as_scrollable(const_cast<Widget*>(ptr)) != nullptr) {
        capabilities.push_back(WidgetCapability::Scrollable);
    }
    if (as_tab_host(const_cast<Widget*>(ptr)) != nullptr) {
        capabilities.push_back(WidgetCapability::TabHost);
    }
    if (as_single_child_container(const_cast<Widget*>(ptr)) != nullptr) {
        capabilities.push_back(WidgetCapability::SingleChildContainer);
    }
    if (as_binary_container(const_cast<Widget*>(ptr)) != nullptr) {
        capabilities.push_back(WidgetCapability::BinaryContainer);
    }
    if (as_bordered_pane(const_cast<Widget*>(ptr)) != nullptr) {
        capabilities.push_back(WidgetCapability::BorderedPane);
    }
    if (as_pane(const_cast<Widget*>(ptr)) != nullptr) {
        capabilities.push_back(WidgetCapability::Pane);
    }
    if (as_layout_box(const_cast<Widget*>(ptr)) != nullptr) {
        capabilities.push_back(WidgetCapability::LayoutBox);
    }
    if (as_splittable(const_cast<Widget*>(ptr)) != nullptr) {
        capabilities.push_back(WidgetCapability::Splittable);
    }
    if (as_text_display(const_cast<Widget*>(ptr)) != nullptr) {
        capabilities.push_back(WidgetCapability::TextDisplay);
    }
    if (as_text_input_field(const_cast<Widget*>(ptr)) != nullptr) {
        capabilities.push_back(WidgetCapability::TextInputField);
    }
    if (as_multiline_text_input(const_cast<Widget*>(ptr)) != nullptr) {
        capabilities.push_back(WidgetCapability::MultiLineTextInput);
    }
    if (as_activatable(const_cast<Widget*>(ptr)) != nullptr) {
        capabilities.push_back(WidgetCapability::Activatable);
    }
    if (as_toggle_control(const_cast<Widget*>(ptr)) != nullptr) {
        capabilities.push_back(WidgetCapability::ToggleControl);
    }
    if (as_selectable_list(const_cast<Widget*>(ptr)) != nullptr) {
        capabilities.push_back(WidgetCapability::SelectableList);
    }
    if (as_value_control(const_cast<Widget*>(ptr)) != nullptr) {
        capabilities.push_back(WidgetCapability::ValueControl);
    }

    return capabilities;
}

void apply_layout_box_stylesheet(LayoutBox& box, Widget& widget, const StyleResolver& styles) {
    widget.Widget::apply_stylesheet(styles);
    const WidgetOptions opts = styles.options(widget);
    if (opts.has("gap")) {
        box.set_layout_gap(opts.int_or("gap", box.layout_gap()));
    }
    if (opts.has("padding")) {
        box.set_layout_padding(opts.int_or("padding", box.layout_padding()));
    }
    if (opts.has("columns")) {
        box.set_layout_column_count(opts.int_or("columns", box.layout_column_count()));
    } else if (opts.has("column-count")) {
        box.set_layout_column_count(opts.int_or("column-count", box.layout_column_count()));
    }
    widget.mark_layout_dirty();
}

void apply_bordered_pane_stylesheet(BorderedPane& pane, Widget& widget, const StyleResolver& styles) {
    widget.Widget::apply_stylesheet(styles);
    const WidgetOptions opts = styles.options(widget);
    BorderEdges edges = pane.border_edges();
    if (opts.has("border-top")) {
        edges.top = opts.bool_or("border-top", edges.top);
    }
    if (opts.has("border-right")) {
        edges.right = opts.bool_or("border-right", edges.right);
    }
    if (opts.has("border-bottom")) {
        edges.bottom = opts.bool_or("border-bottom", edges.bottom);
    }
    if (opts.has("border-left")) {
        edges.left = opts.bool_or("border-left", edges.left);
    }
    pane.set_border_edges(edges);
}

void apply_splittable_stylesheet(Splittable& split, Widget& widget, const StyleResolver& styles) {
    widget.Widget::apply_stylesheet(styles);
    SplitPaneOptions options = split.split_options();
    const WidgetOptions opts = styles.options(widget);
    if (opts.has("outer-border")) {
        options.outer_border = opts.bool_or("outer-border", options.outer_border);
    }
    if (opts.has("first-size")) {
        options.first_size = opts.int_or("first-size", options.first_size);
    }
    if (opts.has("min-pane-size")) {
        options.min_pane_size = opts.int_or("min-pane-size", options.min_pane_size);
    }
    if (opts.has("divider-hit-slop")) {
        options.divider_hit_slop = opts.int_or("divider-hit-slop", options.divider_hit_slop);
    }
    if (opts.has("orientation")) {
        const std::string orientation = opts.string_or("orientation", "horizontal");
        options.orientation =
            orientation == "vertical" ? SplitOrientation::Vertical : SplitOrientation::Horizontal;
    }
    split.set_split_options(options);
}

void apply_text_input_stylesheet(TextInputField& field, Widget& widget, const StyleResolver& styles) {
    widget.Widget::apply_stylesheet(styles);
    if (auto* input = dynamic_cast<TextInput*>(&widget)) {
        TextInputOptions options{.min_width = input->min_width(),
                                 .placeholder = std::string(field.field_placeholder())};
        const WidgetOptions opts = styles.options(widget);
        if (opts.has("min-width")) {
            options.min_width = opts.int_or("min-width", options.min_width);
        }
        if (opts.has("placeholder")) {
            options.placeholder = opts.string_or("placeholder", options.placeholder);
        }
        input->set_options(options);
    }
}

void apply_multiline_text_input_stylesheet(MultiLineTextInput& field, Widget& widget,
                                           const StyleResolver& styles) {
    widget.Widget::apply_stylesheet(styles);
    if (auto* area = dynamic_cast<TextArea*>(&widget)) {
        const WidgetOptions opts = styles.options(widget);
        if (opts.has("placeholder")) {
            area->set_placeholder(opts.string_or("placeholder", std::string(field.field_placeholder())));
        }
        if (opts.has("line-numbers")) {
            area->set_line_numbers(opts.bool_or("line-numbers", area->line_numbers()));
        }
        if (opts.has("status-bar")) {
            area->set_status_bar(opts.bool_or("status-bar", area->status_bar()));
        }
        if (opts.has("gutter-width")) {
            area->set_gutter_width(opts.int_or("gutter-width", 0));
        }
        if (opts.has("min-width")) {
            area->set_min_width(opts.int_or("min-width", area->min_width()));
        }
        if (opts.has("min-height")) {
            area->set_min_height(opts.int_or("min-height", area->min_height()));
        }
    }
}

void apply_scroll_view_stylesheet(Widget& widget, const StyleResolver& styles) {
    widget.Widget::apply_stylesheet(styles);
    if (auto* view = dynamic_cast<ScrollView*>(&widget)) {
        ScrollViewOptions options = view->options();
        const WidgetOptions opts = styles.options(widget);
        if (opts.has("width")) {
            options.width = opts.int_or("width", options.width);
        }
        if (opts.has("height")) {
            options.height = opts.int_or("height", options.height);
        }
        if (opts.has("scrollbar-vertical")) {
            options.scrollbars = options.scrollbars.with_vertical(opts.bool_or("scrollbar-vertical", true));
        }
        if (opts.has("scrollbar-horizontal")) {
            options.scrollbars =
                options.scrollbars.with_horizontal(opts.bool_or("scrollbar-horizontal", false));
        }
        view->set_options(options);
    }
}

void apply_progress_bar_stylesheet(Widget& widget, const StyleResolver& styles) {
    widget.Widget::apply_stylesheet(styles);
    if (auto* bar = dynamic_cast<ProgressBar*>(&widget)) {
        ProgressBarOptions options = bar->options();
        const WidgetOptions opts = styles.options(widget);
        if (opts.has("min-width")) {
            options.min_width = opts.int_or("min-width", options.min_width);
        }
        if (opts.has("show-percent")) {
            options.show_percent = opts.bool_or("show-percent", options.show_percent);
        }
        bar->set_options(options);
    }
}

void apply_value_control_stylesheet(ValueControl& /*control*/, Widget& widget, const StyleResolver& styles) {
    widget.Widget::apply_stylesheet(styles);
    if (auto* slider = dynamic_cast<Slider*>(&widget)) {
        const WidgetOptions opts = styles.options(widget);
        if (opts.has("min-width")) {
            slider->set_min_width(opts.int_or("min-width", 20));
        }
    }
    if (auto* combo = dynamic_cast<ComboBox*>(&widget)) {
        const WidgetOptions opts = styles.options(widget);
        if (opts.has("min-width")) {
            combo->set_min_width(opts.int_or("min-width", 20));
        }
    }
}

} // namespace tuinator
