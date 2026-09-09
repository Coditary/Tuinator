# Golden Examples

Reference apps and patterns for building Tuinator UIs without rendering pitfalls.
Run demos from the build directory: `./tuinator-<name>`.

## Start here

| Demo | Binary | What it demonstrates |
|------|--------|----------------------|
| **Golden app** | `tuinator-golden-app` | `make_screen()` root, theme, minimal structure |
| **Counter** | `tuinator-counter` | Buttons, state, `mark_dirty`, keyboard hints |
| **Form** | `tuinator-form` | `TextInput`, focus, tab order |
| **Controls** | `tuinator-controls` | Widget gallery in a scrollable `Screen` |

## Layout and containers

| Demo | Binary | Pattern |
|------|--------|---------|
| **Layout** | `tuinator-layout` | `VBox` / `HBox` / `Grid` |
| **Split panes** | `tuinator-split-panes` | Nested `SplitPane`, drag dividers |
| **Dashboard** | `tuinator-dashboard` | `Tabs`, multiple views |
| **Terminal frame** | `tuinator-terminal-frame` | Chrome, status line, bordered surface |

## Styling

| Demo | Binary | Pattern |
|------|--------|---------|
| **Theme** | `tuinator-theme` | Built-in theme tokens |
| **Stylesheet** | `tuinator-stylesheet` | `.theme` file, widget classes, RGB |
| **Colors** | `tuinator-colors` | True-color styles |

## Data and charts

| Demo | Binary | Pattern |
|------|--------|---------|
| **Charts** | `tuinator-charts` | Charts inside `Panel` / layout containers |
| **Data** | `tuinator-data` | `Table`, `ListView` |

## Inline / embedded

| Demo | Binary | Pattern |
|------|--------|---------|
| **Inline tasks** | `tuinator-inline-tasks` | `InlineView`, scrollback-safe band |

## Recommended app skeleton

```cpp
#include <tuinator/tuinator.hpp>

int main() {
    tuinator::Application app;
    const tuinator::Theme theme = tuinator::dark_theme();

    auto root = tuinator::make_screen({.gap = 1, .padding = 1});
    root->add_child(std::make_unique<tuinator::Label>("Title", theme.heading));
    // ... more children ...

    return tuinator::run_screen(app, std::move(root));
}
```

See also [rendering-pitfalls.md](rendering-pitfalls.md) and `.cursor/rules/tuinator-rendering.mdc`.

## Debugging renders

```bash
TUINATOR_DEBUG_PAINT=1 ./tuinator-counter
```

Logs each frame: `full` vs `partial`, dirty rectangle, and how many widgets called `paint_bounds_background()`.
