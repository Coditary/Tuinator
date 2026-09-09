# Widget Capabilities & Roles

Tuinator widgets share behavior through **capability interfaces** (like `Scrollable` and `TabHost`).  
Use `as_<role>(widget)` or `widget_has_capability(widget, cap)` to query widgets generically.

## Architecture

```
Widget                          ← base: bounds, paint, events, id/class, flex, enabled
├── capability interfaces       ← small virtual contracts (multiple inheritance)
│   ├── Scrollable              ← ScrollView
│   ├── TabHost                 ← Tabs
│   ├── SingleChildContainer    ← Panel, ScrollView
│   ├── BinaryContainer         ← SplitPane
│   ├── BorderedPane            ← Panel
│   ├── Pane                    ← Panel (= SingleChild + Bordered)
│   ├── LayoutBox               ← VBox, HBox, Grid
│   ├── Splittable              ← SplitPane
│   ├── TextDisplay             ← Label
│   ├── TextInputField          ← TextInput
│   ├── MultiLineTextInput      ← TextArea
│   ├── Activatable             ← Button
│   ├── ToggleControl           ← Checkbox, Toggle
│   ├── SelectableList          ← ListView, Table, ComboBox
│   └── ValueControl            ← Slider, ComboBox
└── concrete widgets
```

## Stylesheet features

### Pseudo-states

```css
Button:focused { color: black; background: cyan; }
Checkbox:checked { color: green; }
ComboBox:open { color: yellow; }
TextInput:disabled { dim: true; }
ListView:selected { color: yellow; }
```

Supported pseudos: `:focused`, `:selected`, `:checked`, `:disabled`, `:open`, `:hover`.

`Application` tracks hover via `wants_hover()` and logs stylesheet option warnings to stderr.

Legacy property aliases (`focused-color`, `selected-color`, …) still work.

### Theme tokens

```css
@theme {
  accent: #58a6ff;
  surface: cyan dim;
  glyph-set: unicode;
}

Button { color: $accent; }
Panel.sidebar { border-color: $surface; }
```

### Options schema

Unknown options per widget type produce validation warnings via
`validate_widget_options()` / `apply_stylesheet_to_tree(..., &warnings)`.

## Widget matrix

| Widget | Capabilities | Styles (paint) | Options (apply) |
|--------|--------------|----------------|-----------------|
| **Panel** | Pane | border, title, glyphs | `border-*` |
| **SplitPane** | BinaryContainer, Splittable | divider | `outer-border`, `first-size`, … |
| **ScrollView** | Scrollable, SingleChildContainer | background | `width`, `height`, `scrollbar-*` |
| **VBox / HBox / Grid** | LayoutBox | — | `gap`, `padding`, `columns` |
| **Label** | TextDisplay | text | — |
| **Button** | Activatable | text, focused | — |
| **TextInput** | TextInputField | text, focused | `min-width`, `placeholder` |
| **TextArea** | MultiLineTextInput | text, focused | `line-numbers`, `placeholder`, … |
| **Checkbox** | ToggleControl | text, focused | — |
| **Toggle** | ToggleControl | text, focused, selected | — |
| **TreeView** | SelectableList | text, selected | — |
| **ListView / Table** | SelectableList | text, selected | — |
| **ProgressBar** | — | fill, track, label (via slots) | `min-width`, `show-percent` |
| **ComboBox** | ValueControl, SelectableList | text, focused, selected | `min-width` |
| **Slider** | ValueControl | text, focused | `min-width` |
| **Tabs** | TabHost | text, selected | — |
| **CustomPaint** | — | text (background) | `width`, `height` |
| **BarChart / LineChart / …** | ChartWidget | title, axis, grid, value | `min-width`, `glyph-style`, … |

## CustomPaint

`CustomPaint` is a generic drawing surface for free-form terminal rendering and composition:

- **`set_on_paint(callback)`** — draw with `PaintContext` and local `Rect` bounds.
- **`add_child(widget)`** — overlay or underlay child widgets (`set_paint_children_first`).
- **`set_background(style)`** — optional fill before painting.
- **`set_needs_periodic_idle(true)`** — for animations (tick from `Application` idle loop).

```cpp
auto host = std::make_unique<tuinator::CustomPaint>();
host->set_preferred_size({40, 12});
host->set_on_paint([](tuinator::PaintContext& ctx, tuinator::Rect area) {
    ctx.canvas.fill_rect(area, '.', {});
    ctx.canvas.draw_text({2, 1}, "Custom UI", {});
});
host->add_child(std::make_unique<tuinator::Label>("Overlay"));
```

## Usage

```cpp
if (auto* pane = tuinator::as_pane(widget)) {
    pane->set_border_edges({false, true, false, true});
}

if (tuinator::widget_has_capability(*widget, tuinator::WidgetCapability::ValueControl)) {
    tuinator::as_value_control(widget)->set_value_current(5);
}

std::vector<std::string> warnings;
tuinator::apply_stylesheet_to_tree(root, resolver, &warnings);
```

## Adding a new widget

1. Pick capability interfaces matching the widget shape.
2. `public` inherit `Widget` + capabilities.
3. Override `widget_type_name()` for type selectors.
4. Override `apply_stylesheet()` — call shared `apply_*_stylesheet()` helpers.
5. Use `ctx.styles()` in `paint()` for colors.
6. Register `as_<role>()` in `widget_roles.cpp` and add options to `widget_option_schema.cpp`.
