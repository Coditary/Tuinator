# Rendering Pitfalls

Guidance for building Tuinator UIs without screen artifacts, ghost characters, or stale overlays.
Intended for human developers and AI assistants working in this repository.

## Golden app structure

Prefer `make_screen()` as root. It is a full-screen `VBox` with `wants_full_screen()` set.

```cpp
tuinator::Application app;
const tuinator::Theme theme = tuinator::dark_theme();
// Alternate screen is on by default; disable only for inline/scrollback embeds:
// app.set_alternate_screen(false);

auto root = tuinator::make_screen({.gap = 1, .padding = 1});
root->add_child(std::make_unique<tuinator::Label>("Title", theme.heading));
root->add_child(std::make_unique<tuinator::Button>("OK", on_ok, theme.button));

return tuinator::run_screen(app, std::move(root));
```

See [golden-examples.md](golden-examples.md) for reference demos. Avoid a bare `Label` or `Button` as the only root unless the demo is intentionally minimal.

Wrap related controls in `Panel`, `VBox`, or `HBox`. Charts and lists should live inside a container with a known size.

## Widget paint contract

Every widget that draws content must clear its bounds before painting:

```cpp
void MyWidget::paint(PaintContext& ctx) const {
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    paint_bounds_background(ctx, style_);  // always first
    // ... draw content ...
}
```

Built-in widgets already follow this. New custom widgets must call `paint_bounds_background()` at the start of `paint()`.

Exceptions (intentionally transparent): `SplitPane` (only paints children and dividers).

## Dirty regions

After data or visual state changes, mark the affected rectangle dirty:

```cpp
void MyWidget::set_label(std::string label) {
    label_ = std::move(label);
    mark_dirty(bounds_);  // prefer widget bounds
}
```

Use `request_redraw()` only when the whole screen must refresh (focus changes handled by the framework).

| API | Effect |
|-----|--------|
| `mark_dirty(bounds_)` | Partial redraw; only dirty rect cleared and repainted |
| `request_redraw()` | Full redraw; entire buffer cleared (P0 behavior) |
| `mark_dirty({})` / `request_redraw()` | Full screen |

Partial redraw is faster but requires widgets to fill their bounds (see above).

## Text and UTF-8

Never clip visible text by byte count:

```cpp
// BAD
text.substr(0, max_columns);

// GOOD
const std::size_t bytes = text_byte_length_for_width(text, max_columns);
text.substr(0, bytes);
```

Use `text_display_width()` for layout and hit-testing.

## Stylesheets

- Define explicit `:focused` / `:selected` rules when you care about focus rings.
  Without them, widget fallback styles are preserved (not overwritten by the base `text` rule).
- RGB colors (`foreground-rgb`, `background-rgb`) use a direct TTY path on the curses backend.
  Partial redraws clear the dirty TTY overlay automatically; still mark dirty rects accurately when content moves.
- Re-applying a stylesheet to `TextInput` preserves `min_width` from the widget; set `min-width` in CSS if you override it.

## Images (Kitty / Sixel)

- Clearing an `ImageView` (`set_image({})`) invalidates terminal graphics and fills bounds.
- Full redraw clears Kitty placements; partial redraw clears Kitty when the dirty rect overlaps the last placement.
- Do not assume ncurses cells cover image pixels — images are terminal overlays.

## Terminal modes

```cpp
app.set_alternate_screen(true);   // default: isolated fullscreen buffer
app.set_clear_on_shutdown(true);  // erase last frame when alt screen is off
```

Use alternate screen for normal fullscreen apps. Disable only for inline band / scrollback embedding (`InlineTerminalBackend`).

## Charts

Charts call `chart_paint_background()` (same as `paint_bounds_background()`). They also paint when data is empty (cleared plot area).

Give charts explicit layout bounds via a parent container; do not rely on default sizing alone in complex layouts.

## Common artifact symptoms

| Symptom | Likely cause | Fix |
|---------|--------------|-----|
| Ghost characters after shorter text | Widget did not fill bounds on partial redraw | `paint_bounds_background()` |
| Old list rows after delete | Same | `mark_dirty(bounds_)` + background fill |
| RGB text stuck on screen | Stale TTY overlay (partial path) | Fixed in backend; ensure dirty rect covers old pixels |
| Kitty image ghost | Image removed without invalidate | `set_image({})` or hide `ImageView` |
| Focus colors wrong after CSS | Missing `:focused` rule | Add explicit focused styles |
| Broken emoji / wide chars | Byte-based clipping | `text_byte_length_for_width()` |
| Scrollback pollution | Alt screen off | `set_alternate_screen(true)` |

## Debug paint logging

Set `TUINATOR_DEBUG_PAINT=1` to log each rendered frame to stderr:

```
tuinator-paint: mode=partial dirty=0,2 80x5 fills=12
```

- `mode` — `full` or `partial`
- `dirty` — clip rectangle used for the frame
- `fills` — number of `paint_bounds_background()` calls (low count may indicate missing fills)

## Checklist for new widgets

1. `paint_bounds_background()` at the start of `paint()`
2. `mark_dirty(bounds_)` in every setter that changes appearance
3. UTF-8-safe text clipping
4. `preferred_size()` matches what `paint()` actually draws
5. Test with partial redraw: paint twice with different content without full buffer clear

## Reference tests

- `tests/core/test_dirty_region.cpp` — partial/full redraw, stale cell clearing
- `tests/render/test_tty_overlay.cpp` — TTY overlay erase
- `tests/widgets/test_list_view.cpp` — `list_view_clears_empty_rows`
- `tests/widgets/charts/test_line_chart.cpp` — `line_chart_clears_bounds_when_empty`
