# Changelog

All notable changes to Tuinator follow [Semantic Versioning](https://semver.org/).

## [Unreleased]

### Added

- `TextArea` widget: multi-line editing, optional line numbers, status bar, mouse/keyboard scrolling
- Replaceable `TextArea` gutter (`set_gutter_renderer`): absolute/relative/hybrid presets, custom symbols
- Demo: `make textarea` (`Tab` cycles gutter styles)
- `Throbber` loading spinners (named sets: dots, braille, clock, aesthetic, …) and `make throbber`
- `BigText` widget: 8x8 pixel densities (block/half/quadrant/sextant/octant/braille), letter-fill, stacked shadow, FIGlet fonts (`make bigtext`)
- `BigText` drop shadow: `shadow_style`, `shadow_dx` / `shadow_dy`, `shadow_layers`
- `BigText` RGB gradients (`gradient_stops`, `gradient_axis`) and named looks (`omarchy`, `latchdark`, `sunset`, …)
- `Checkbox` widget: custom unchecked/checked glyphs, per-state styles, presets (`basic`, `info`, `warning`, `checkmark`, …) — `make checkbox`
- `MenuBar`: floating dropdown, shortcuts, separators, mnemonics, checkbox items, submenus, icons, hints, style presets, Alt+mnemonics
- `ContextMenu`, `CommandPalette` (Ctrl+P), and `ActionRegistry` for shared menu/shortcut wiring
- Demo: `make menu` — full menu catalog (submenus, context menu, palette, presets)
- True-color text via 24-bit ANSI after refresh (so `make colors` no longer maps RGB to 8 ANSI colors)

## [0.1.0] - 2026-09-01

First feature-complete preview (Phases 0–6).

### Added

- Terminal UI core: `Application`, event loop, damage tracking, keyboard focus
- Layout: `VBox`, `HBox`, `Grid`, flex sizing, `Panel`, `SplitPane`, `Tabs`, `StatusBar`
- Widgets: `Label`, `Button`, `TextInput`, `Toggle`, `ScrollView`, `ListView`, `Table`, `Separator`
- Window system: floating `Window`, `Desktop`, modal dialogs, `WindowHost`
- Themes and true-color styles (`Theme`, `ColorValue`, `style_hex_*`)
- Timers: `set_interval()`, `set_timeout()`, `cancel_timer()`
- `MemoryTerminalBackend` for headless tests and snapshot rendering
- Example demos (`make hello`, `make dashboard`, `make data`, …)
- Smoke tests (`make test`) and unit tests (`make unit-test`)
- CI on Linux, macOS, and Windows

### Fixed

- Label rendering: avoid dangling `string_view` from `std::string::substr()` (garbled header text)

[0.1.0]: https://github.com/coditary/Tuinator/releases/tag/v0.1.0
