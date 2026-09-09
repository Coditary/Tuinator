#pragma once

#include <tuinator/render/border_style.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/widgets/containers/split_pane_options.hpp>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

class Widget;
class StyleResolver;

/// Exactly one content child (Panel, ScrollView, …).
class SingleChildContainer {
  public:
    virtual ~SingleChildContainer() = default;

    virtual Widget* content() const = 0;
    virtual void set_content(std::unique_ptr<Widget> content) = 0;
};

/// Two-pane split layout (SplitPane).
class BinaryContainer {
  public:
    virtual ~BinaryContainer() = default;

    virtual Widget* first_child() const = 0;
    virtual Widget* second_child() const = 0;
};

/// Titled container with configurable border edges (Panel).
class BorderedPane {
  public:
    virtual ~BorderedPane() = default;

    virtual std::string_view pane_title() const = 0;
    virtual BorderEdges border_edges() const = 0;
    virtual void set_border_edges(BorderEdges edges) = 0;
};

/// Combines SingleChildContainer + BorderedPane — the classic IDE pane shape.
class Pane : public SingleChildContainer, public BorderedPane {};

/// Gap/padding layout containers (VBox, HBox, Grid).
class LayoutBox {
  public:
    virtual ~LayoutBox() = default;

    virtual int layout_gap() const = 0;
    virtual int layout_padding() const = 0;
    virtual int layout_column_count() const { return 0; }
    virtual void set_layout_gap(int gap) = 0;
    virtual void set_layout_padding(int padding) = 0;
    virtual void set_layout_column_count(int columns) {}
};

/// Draggable split divider between two children.
class Splittable {
  public:
    virtual ~Splittable() = default;

    virtual const SplitPaneOptions& split_options() const = 0;
    virtual void set_split_options(SplitPaneOptions options) = 0;
};

/// Read-only text display (Label, BigText fragments, …).
class TextDisplay {
  public:
    virtual ~TextDisplay() = default;

    virtual std::string_view display_text() const = 0;
    virtual Style display_style() const = 0;
};

/// Single-line text entry (TextInput).
class TextInputField {
  public:
    virtual ~TextInputField() = default;

    virtual std::string_view field_value() const = 0;
    virtual std::string_view field_placeholder() const = 0;
};

/// Click/activate control (Button, menu entries, …).
class Activatable {
  public:
    virtual ~Activatable() = default;

    virtual void activate() = 0;
};

/// Checkbox / toggle state.
class ToggleControl {
  public:
    virtual ~ToggleControl() = default;

    virtual bool is_checked() const = 0;
    virtual void set_checked(bool checked) = 0;
};

/// List or table with a selected row/index.
class SelectableList {
  public:
    virtual ~SelectableList() = default;

    virtual int selected_index() const = 0;
    virtual void set_selected_index(int index) = 0;
};

/// Multiline text entry (TextArea).
class MultiLineTextInput : public TextInputField {};

/// Numeric value control (Slider, ComboBox index).
class ValueControl {
  public:
    virtual ~ValueControl() = default;

    virtual int value_minimum() const = 0;
    virtual int value_maximum() const = 0;
    virtual int value_current() const = 0;
    virtual void set_value_current(int value) = 0;
};

enum class WidgetPseudoState {
    Focused,
    Selected,
    Checked,
    Disabled,
    Open,
    Hover,
};

bool widget_matches_pseudo(const Widget& widget, WidgetPseudoState pseudo);

enum class WidgetCapability {
    Scrollable,
    TabHost,
    SingleChildContainer,
    BinaryContainer,
    BorderedPane,
    Pane,
    LayoutBox,
    Splittable,
    TextDisplay,
    TextInputField,
    MultiLineTextInput,
    Activatable,
    ToggleControl,
    SelectableList,
    ValueControl,
};

bool widget_has_capability(const Widget& widget, WidgetCapability capability);
std::vector<WidgetCapability> widget_capabilities(const Widget& widget);

SingleChildContainer* as_single_child_container(Widget* widget);
const SingleChildContainer* as_single_child_container(const Widget* widget);
BinaryContainer* as_binary_container(Widget* widget);
const BinaryContainer* as_binary_container(const Widget* widget);
BorderedPane* as_bordered_pane(Widget* widget);
const BorderedPane* as_bordered_pane(const Widget* widget);
Pane* as_pane(Widget* widget);
const Pane* as_pane(const Widget* widget);
LayoutBox* as_layout_box(Widget* widget);
const LayoutBox* as_layout_box(const Widget* widget);
Splittable* as_splittable(Widget* widget);
const Splittable* as_splittable(const Widget* widget);
TextDisplay* as_text_display(Widget* widget);
const TextDisplay* as_text_display(const Widget* widget);
TextInputField* as_text_input_field(Widget* widget);
const TextInputField* as_text_input_field(const Widget* widget);
Activatable* as_activatable(Widget* widget);
const Activatable* as_activatable(const Widget* widget);
ToggleControl* as_toggle_control(Widget* widget);
const ToggleControl* as_toggle_control(const Widget* widget);
SelectableList* as_selectable_list(Widget* widget);
const SelectableList* as_selectable_list(const Widget* widget);
MultiLineTextInput* as_multiline_text_input(Widget* widget);
const MultiLineTextInput* as_multiline_text_input(const Widget* widget);
ValueControl* as_value_control(Widget* widget);
const ValueControl* as_value_control(const Widget* widget);

/// Shared stylesheet hooks used by role implementations.
void apply_layout_box_stylesheet(LayoutBox& box, Widget& widget, const StyleResolver& styles);
void apply_bordered_pane_stylesheet(BorderedPane& pane, Widget& widget, const StyleResolver& styles);
void apply_splittable_stylesheet(Splittable& split, Widget& widget, const StyleResolver& styles);
void apply_text_input_stylesheet(TextInputField& field, Widget& widget, const StyleResolver& styles);
void apply_multiline_text_input_stylesheet(MultiLineTextInput& field, Widget& widget,
                                           const StyleResolver& styles);
void apply_scroll_view_stylesheet(Widget& widget, const StyleResolver& styles);
void apply_value_control_stylesheet(ValueControl& control, Widget& widget, const StyleResolver& styles);
void apply_progress_bar_stylesheet(Widget& widget, const StyleResolver& styles);

} // namespace tuinator
