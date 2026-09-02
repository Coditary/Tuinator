# Tuinator API overview

Tuinator 0.1.0 — include `<tuinator/tuinator.hpp>` for the full public surface.

## Application

```cpp
tuinator::Application app;
app.set_root(std::move(root));
app.set_theme(tuinator::dark_theme());
app.set_interval(1000, [&]() { /* periodic work */ });
return app.run();
```

| Method | Description |
|--------|-------------|
| `set_root(Widget)` | Root widget tree (replaces previous) |
| `set_theme(Theme)` | Semantic color/style presets |
| `present()` | Layout + render one frame (profiling/tests) |
| `run()` | Blocking event loop until `quit()` |
| `set_interval(ms, fn)` | Repeating timer |
| `set_timeout(ms, fn)` | One-shot timer |
| `cancel_timer(id)` | Stop a timer |
| `terminal_size()` | Current `{width, height}` |
| `quit()` | Exit `run()` |

Inject a custom backend for tests:

```cpp
auto backend = tuinator::MemoryTerminalBackend::create({80, 24});
tuinator::Application app(std::move(backend));
```

## Widget base class

Subclass `Widget` and implement:

- `preferred_size()` — intrinsic size
- `layout(Rect)` — assigned bounds from parent
- `paint(Canvas&)` — draw in local coordinates (0,0 = top-left of widget)
- `handle_event(Event)` — return `true` if consumed
- Optional: `is_focusable()`, `wants_full_screen()`, `collect_focusable()`

Helpers: `add_child()`, `set_flex(n)`, `mark_dirty()`, `bounds()`.

Parent containers clip children automatically (`Canvas::with_clip`).

## Layout

| Type | Role |
|------|------|
| `VBox` / `HBox` | Stack children vertically/horizontally (`BoxOptions`: gap, padding) |
| `Grid` | Row/column grid with fixed column count |
| `Panel` | Border + optional title + single content child |
| `SplitPane` | Two resizable panes (horizontal/vertical) |
| `Tabs` | Tab bar with switchable pages |
| `ScrollView` | Scrollport around a larger child |
| `StatusBar` | Single-line footer text |

Use `set_flex(1)` on a child to consume remaining space in a box layout.

## Widgets

| Widget | Focus | Notes |
|--------|-------|-------|
| `Label` | No | Centered single/multi-line text |
| `Button` | Yes | Click / Enter / Space |
| `TextInput` | Yes | Cursor, placeholder, Ctrl+A select all |
| `TextArea` | Yes | Multi-line editor, optional line numbers + status bar, scroll |
| `Toggle` | Yes | Checkbox-style on/off (legacy) |
| `Checkbox` | Yes | Customizable checkbox (glyphs, styles, presets) |
| `ListView` | Yes | Selectable list, callbacks on select/activate |
| `Table` | Yes | Columns + rows, header row |
| `ComboBox` | Yes | Dropdown selection (Enter/click to open) |
| `Slider` | Yes | Numeric range, Left/Right or click |
| `Spinner` | Yes | Numeric stepper (`< 5 >`) |
| `Throbber` | No | Animated loading spinner (dots, braille, clock, emoji, …) |
| `BigText` | No | Large pixel / FIGlet banners (`block`, `quadrant`, `sextant`, `slant`, `doom`, …) |
| `ImageView` | No | Kitty / iTerm2 / Sixel terminal graphics |
| `ProgressBar` | No | Plain, Bracketed, Blocks, FilledLabel, Tqdm, BrailleMetric, BrailleWave, Pulse, Shimmer, Bounce, SlideBlock, MovingDot, TaskRow |

`ProgressBarMode::Determinate` fills from the left using `set_value(0.0–1.0)`. `ProgressBarMode::Indeterminate` shows a pendulum segment when progress is unknown. For the yellow filled-label style:

```cpp
auto known = progress_bar_filled_label(theme.accent, theme.muted, "Total progress",
    ProgressBarMode::Determinate);
auto loading = progress_bar_filled_label(theme.accent, theme.muted, "Loading...",
    ProgressBarMode::Indeterminate);
ProgressBar bar(0.82, known);
ProgressBar spinner(0.0, loading);
spinner.set_animation_phase(frame); // drives the pendulum
```

**Terminal images** — `ImageView` + `TerminalImage` with Kitty (Ghostty/Kitty/WezTerm), iTerm2 inline images, and basic Sixel fallback:

```cpp
auto image = TerminalImage::gradient(160, 80);
auto view = std::make_unique<ImageView>(std::move(image), Size{48, 14});
```

Run the interactive demo: `make image` (or `./build/tuinator-image`). Override protocol with `TUINATOR_GRAPHICS=kitty|iterm2|sixel|auto`.

**TextArea** — multi-line editor with optional gutter and status bar:

```cpp
TextAreaOptions options;
options.line_numbers = true;
options.status_bar = true;
options.title = "notes.txt";
auto editor = std::make_unique<TextArea>(options, theme.text_input, theme.text_input_focused);
editor->set_value("# heading\n\nbody");
```

Run `make textarea`. Type to edit; arrows / PgUp / wheel scroll; click to place the cursor. `Tab` cycles the gutter (absolute / relative / symbols / off). `Esc` or `Ctrl+Q` quits (so `q` can be typed).

**BigText** — large banners from an 8×8 pixel font or FIGlet outlines:

```cpp
auto title = std::make_unique<BigText>("Hello", "slant", theme.heading);
BigTextOptions options;
options.kind = BigTextKind::Quadrant;
options.rainbow = true;
auto pixels = std::make_unique<BigText>("Tuinator", options, theme.accent);

BigTextOptions stacked;
stacked.kind = BigTextKind::Stacked;
stacked.shadow_style = style_fg(Rgb::hex(0x3A8EC8));
stacked.shadow_dx = 1;
stacked.shadow_dy = 1;
stacked.shadow_layers = 2; // 0 = kind default (2 stacked, 1 isometric)
auto banner = std::make_unique<BigText>("NEXIS", stacked, theme.accent);

// Vertical RGB gradient (synthwave-style)
BigTextOptions synth;
synth.kind = BigTextKind::Stacked;
synth.gradient_stops = big_text_gradient({
    {0.0f, 0xFFB347},
    {0.5f, 0xFF2A5C},
    {1.0f, 0xC020A0},
});
synth.shadow_style = style_fg(Rgb::hex(0x5A1050));
synth.shadow_dy = 2;
auto title = std::make_unique<BigText>("OMARCHY", synth, theme.heading);

// Named looks: omarchy, latchdark, sunset, ocean, neon, ice, doom-fire
apply_big_text_look(synth, *big_text_look_named("omarchy"));
```

Unset `shadow_style` keeps a dimmed copy of the fill. `gradient_stops` tints each row or column (see `gradient_axis`). Preset looks: `all_big_text_looks()` / `big_text_look_named("omarchy")`.

Styles: `block`, `half`, `narrow`, `quadrant`, `sextant`, `octant`, `braille`, `letter`, `banner`, `outline`, `stacked`, `isometric`, `slant`, `standard`, `small`, `big`, `doom`. Catalog: `all_big_text_styles()`. Demo: `make bigtext`.

**Checkbox** — customizable on/off control with symbols, colors, and focus highlight:

```cpp
auto box = std::make_unique<Checkbox>(
    "Enable notifications",
    true,
    "basic",
    [](bool on) { /* ... */ },
    theme);

CheckboxOptions custom = checkbox_options_default(theme);
custom.glyphs = {"○", "●", " "};
custom.label_checked_style = style_fg(Rgb::hex(0x39FF14));
apply_checkbox_style(custom, *checkbox_style_named("diamond"), theme);
auto fancy = std::make_unique<Checkbox>("Custom", false, custom);
```

Presets: `basic`, `info`, `warning`, `error`, `checkmark`, `circle`, `diamond`, `ascii`, `asterisk`, `plus`, `xo`. Demo: `make checkbox`. `Space` / click toggles; `Up`/`Down` navigates in scroll views.

**MenuBar** — floating dropdown, shortcuts, separators, mnemonics (`&`), checkbox items, submenus (`→`), icons, hints, style presets (`classic`, `mac`, `minimal`). Alt+letter opens top menus. `ContextMenu`, `CommandPalette` (Ctrl+P), and `ActionRegistry` for shared shortcuts. Demo: `make menu`.

Replace the gutter with a callback — relative numbers, markers, or anything else:

```cpp
editor->set_gutter_renderer([](const tuinator::GutterLine& line) {
    auto cell = tuinator::hybrid_gutter(line); // current = absolute, others = relative
    cell.text = std::string(line.current ? ">" : " ") + cell.text;
    return cell;
});
editor->set_gutter_width(5); // 0 = auto
```

Presets: `absolute_gutter`, `relative_gutter`, `hybrid_gutter`. `GutterLine` gives `index`, `number`, `cursor_index`, `current`, and `text`.

Run `make throbber` for a live catalog (`braille`, `blocks`, `bouncing-bar`, `clock`, `moon`, `pong`, …).

```cpp
auto spin = std::make_unique<Throbber>("braille", theme.accent);
app.set_interval(50, [spin_ptr]() { spin_ptr->tick(50); });
```

Look up sets with `throbber_set_named("braille")` (aliases like `"dots"` still work) or iterate `all_throbber_sets()`. Each set has an `id` and a display `title`.

**Gradient fill** — set `gradient_stops` (2+ colors, positions `0.0`–`1.0`). Works on fill glyphs and `FilledLabel` backgrounds. Uses true color when available, otherwise nearest ANSI:

```cpp
options.gradient_stops = progress_bar_gradient({
    {0.0f, 0x5FB89E},  // teal
    {0.5f, 0xB898D0},  // lavender
    {1.0f, 0xE848A0},  // magenta
});
```
| `TreeView` | Yes | Expandable tree, Left/Right to toggle |
| `MenuBar` | Yes | Top menu: dropdown, submenus, shortcuts, mnemonics, checkboxes, icons, hints |
| `ContextMenu` | — | Right-click popup menu (via `Desktop`) |
| `CommandPalette` | — | Fuzzy command search (`Ctrl+P` on `Desktop`) |
| `ActionRegistry` | — | Shared actions for menus, shortcuts, palette |
| `dialog::make_confirm` | — | Alert/confirm content for `Desktop::show_modal()` |
| `Separator` | No | Horizontal line |

## Events

`Event` is a `std::variant` of:

- `KeyPress` — `Key` enum + optional `character`
- `MouseEvent` — position, `MouseAction`, button state
- `Resize` — new terminal size

Common keys: `Tab`, `BackTab`, `Enter`, `Escape`, arrows, `PageUp`/`PageDown`, `Home`/`End`.

## Rendering

- `Canvas` — clip-aware drawing (`draw_text`, `draw_char`, `draw_box`, `fill_rect`)
- `Style` — foreground/background (`Color` or `Rgb`), bold, dim, reverse
- `Theme` — `dark_theme()`, `light_theme()` semantic styles + border glyphs
- `GlyphSet` — `Ascii`, `Unicode`, `UnicodeRounded`, `Auto` (UTF-8 locale detection)
- `ColorValue::from_hex()` / `style_hex_fg()` — true-color with ANSI fallback

## Windows

- `Desktop` — Z-ordered floating windows, drag, resize, modal dimming
- `Window` — titled frame with content area
- `WindowHost` — nested windows clipped inside a parent

## Testing

Headless rendering with `MemoryTerminalBackend`:

```cpp
tuinator::MemoryTerminalBackend backend({80, 24});
backend.init();
root.layout({0, 0, 80, 24});
backend.begin_frame();
tuinator::Canvas canvas(backend);
root.paint(canvas);
backend.end_frame();
std::string screen = backend.snapshot();
```

Run unit tests: `make unit-test` or `ctest --test-dir build`.

## Versioning

Public API version: `TUINATOR_VERSION` in `<tuinator/version.hpp>`.

Pre-1.0 semver: minor bumps may add API; patch bumps are bug fixes. Breaking changes bump minor until 1.0.
