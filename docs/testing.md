# Testing and UI automation

Tuinator supports three complementary ways to test terminal UIs: **widget unit tests** (fast, many cases), **`AppDriver` integration tests** (full app loop, hover, colors), and a **JSON line protocol** for scripts and AI agents that inspect state dynamically (“debugger style”).

Headers live under `include/tuinator/testing/`.

## Quick start

```bash
cmake -B build -DTUINATOR_BUILD_TESTS=ON -DTUINATOR_BUILD_EXAMPLES=ON
cmake --build build
./build/tuinator-tests
./build/examples/tuinator-testing-agent   # demo JSON agent (stdin/stdout)
```

Assign selectors in your UI:

```cpp
row->set_test_id("sidebar-row-identitaet");
button->set_widget_id("submit");  // stylesheet/scene id; also used if test_id is empty
```

---

## Test IDs and Release builds

| API | Purpose |
|-----|---------|
| `set_test_id()` / `test_id()` | Stable selectors for tests and agents (like `data-testid`) |
| `set_widget_id()` / `widget_id()` | Stylesheets, scenes; fallback for `find_by_selector()` |

`tuinator::testing::find_by_selector(root, "foo")` matches **`test_id` first**, then **`widget_id`**.

### Automatic stripping in Release

CMake option **`TUINATOR_ENABLE_TEST_IDS`** (default):

| Configuration | Default |
|---------------|---------|
| `Release` / `MinSizeRel` | **OFF** — `set_test_id()` is a no-op, nothing stored, `test_id()` always empty |
| `Debug` / `RelWithDebInfo` | **ON** |
| `TUINATOR_BUILD_TESTS=ON` | **ON** (even in Release) |

Override:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DTUINATOR_ENABLE_TEST_IDS=ON
```

Your app can keep `set_test_id(...)` calls everywhere; in a normal Release ship build they cost no storage. Link against `tuinator::tuinator` so your app compiles with the same `TUINATOR_ENABLE_TEST_IDS` as the library.

---

## 1. Widget unit tests (most tests, fastest)

Use this for toggles, checkboxes, inputs, labels, charts, etc. No full `Application` — mount one widget, paint into `MemoryTerminalBackend`, send events.

Pattern (see `tests/widgets/test_checkbox.cpp`, `tests/support/render_helper.hpp`):

```cpp
#include "render_helper.hpp"
#include "test_harness.hpp"

TUINATOR_TEST(checkbox_toggles_with_space) {
    tuinator::Checkbox checkbox("Label", false);
    checkbox.layout({0, 0, 24, 1});
    checkbox.set_focused(true);
    checkbox.handle_event(tuinator::KeyPress{.character = ' '});
    TUINATOR_CHECK(checkbox.checked());
}
```

```cpp
tuinator::MemoryTerminalBackend backend({40, 5});
backend.init();
tuinator::test::render_root(widget, backend);
TUINATOR_CHECK(tuinator::test::row_contains(backend, 0, "expected text"));
```

Run: `make unit-test` or `ctest --test-dir build`.

Behavior-focused component suites that use fixed terminal sizes, `AppDriver`, clicks, and
resize live in `tests/widgets/test_component_app_driver.cpp`.

---

## 2. `AppDriver` (integration, hover, colors)

`tuinator::testing::AppDriver` runs a real `Application` on `MemoryTerminalBackend` (pointer hover enabled). Use when multiple widgets interact, focus/tab matters, or you assert **painted colors** across a row.

```cpp
#include <tuinator/testing/app_driver.hpp>

auto root = build_my_ui();
tuinator::testing::AppDriver driver({120, 40}, std::move(root));

driver.hover("sidebar-row-2");
const auto style = driver.analyze_selector("sidebar-row-2");

TUINATOR_CHECK(style.uniform_background);
TUINATOR_CHECK(style.left_background == style.right_background);
```

### Common methods

| Method | Effect |
|--------|--------|
| `present()` | Layout, focus, one full render |
| `step_frame()` | Poll events, timers, idle, render (non-blocking) |
| `focus(id)` / `click(id)` / `activate(id)` | Focus or pointer/activate |
| `type_into(id, text)` / `type_text(text)` / `press_key(key, ch)` | Keyboard |
| `hover(id)` / `hover_at(point)` | Mouse move → hover state + redraw |
| `snapshot_text()` | ASCII screen from backend |
| `snapshot_ansi()` | Screen with RGB SGR sequences |
| `ui_tree_json(options)` | Widget tree JSON |
| `analyze_selector(id)` / `selector_styles_json(id)` | Color analysis in widget bounds |
| `write_snapshot_png(path)` | Raster snapshot (layout/colors) |
| `resize_terminal(size)` | Resize memory backend and re-layout (tests responsive TUIs) |

### Color analysis (`analyze_selector`)

Reads **terminal cells** inside the widget’s `bounds` (what was actually painted), not internal C++ fields. Useful for bugs like hover highlighting the label but not the timestamp column.

`RegionStyleAnalysis` includes:

- `uniform_background` / `uniform_foreground`
- `distinct_backgrounds` / `distinct_foregrounds` (resolved RGB)
- `edges.left_background` / `edges.right_background` (middle row, inset columns)

Example: simulate mismatched row backgrounds in `tests/core/test_testing_driver.cpp` (`region_style_detects_mismatched_row_background`).

### UI tree options

```cpp
tuinator::testing::UiTreeOptions options;
options.include_screen_text = true;   // embed terminal snapshot in tree JSON
options.include_render_styles = true; // per-widget "render" color summary (needs backend)
driver.ui_tree_json(options);
```

---

## 3. JSON protocol (agents, debugger-style loops)

One **JSON object per line** on **stdin**; one **JSON response per line** on **stdout** (NDJSON / JSONL-friendly).

Entry points:

- `tuinator::testing::handle_json_command(AppDriver&, line)` — single command
- `tuinator::testing::run_json_protocol_loop(AppDriver&)` — read until EOF or `quit`

### Demo binary

`examples/testing-agent.cpp` → target `tuinator-testing-agent` (small login form). Pipe commands:

```bash
printf '%s\n' \
  '{"cmd":"hover","id":"username"}' \
  '{"cmd":"styles","id":"username"}' \
  '{"cmd":"tree","include_render":true}' \
  '{"cmd":"quit"}' \
  | ./build/examples/tuinator-testing-agent
```

Or from a file:

```bash
cat scenario.jsonl | ./build/examples/tuinator-testing-agent
```

### Lua (and other scripting languages)

Tuinator does **not** embed a Lua VM. You still write tests in Lua by driving the **same JSON protocol**: your script prints one JSON object per line; the agent reads stdin and prints responses on stdout.

**Static scenario** — Lua generates JSONL, shell pipes it:

```bash
lua scripts/my_scenario.lua | ./build/examples/tuinator-testing-agent
```

Minimal `scripts/my_scenario.lua` (no extra libraries; hand-built JSON for simple strings):

```lua
local function emit(cmd)
  io.write('{"cmd":"' .. cmd.cmd .. '"')
  if cmd.id then io.write(',"id":"' .. cmd.id .. '"') end
  if cmd.text then io.write(',"text":"' .. cmd.text .. '"') end
  io.write("}\n")
end

emit({ cmd = "hover", id = "username" })
emit({ cmd = "styles", id = "username" })
emit({ cmd = "quit" })
```

For nested JSON or reading responses, use a small library (e.g. `dkjson`) or run the agent from Lua with **bidirectional** I/O (`lua-protobuf`/LuaSocket, or Python as a thin bridge). Pattern:

1. Spawn `tuinator-testing-agent` (or your app’s agent binary) as a subprocess with stdin/stdout pipes.
2. Write a line, flush, read a line, `json.decode` the response.
3. Branch on `tree`, `styles`, `uniform_background`, etc.

That gives you **full debugger-style loops in Lua** without recompiling C++.

**Note:** The agent process must be built with test IDs enabled (`Debug` or `-DTUINATOR_ENABLE_TEST_IDS=ON`) so `id` in JSON resolves.

### Command reference

All successful responses include `"ok": true`. Failures: `"ok": false`, `"error": "..."`.

| `cmd` | Fields | Description |
|-------|--------|-------------|
| `tree` | `include_screen` (bool), `include_render` (bool) | Widget tree; optional screen text and per-widget render stats |
| `snapshot` | — | Plain text screen |
| `ansi` | — | ANSI-colored screen |
| `styles` | `id` (required) | Color analysis for selector bounds |
| `png` | `path` (required) | Write PNG snapshot |
| `hover` | `id` | Mouse hover center of widget |
| `focus` | `id` | Keyboard focus |
| `click` | `id` | Press / release / click |
| `activate` | `id` | `Activatable::activate()` or click |
| `type` | `id` (optional), `text` | Type into focused field or into `id` |
| `key` | `key` | `Enter`, `Escape`, `Tab`, `Space`, arrows, or single character |
| `present` | — | Force render |
| `step` | — | One event-loop iteration |
| `quit` / `exit` | — | End loop (response may include `"quit": true`) |

### AI / debugger workflow

The model or script does **not** need a fixed scenario file. Typical loop:

1. `{"cmd":"tree","include_screen":true,"include_render":true}` — structure, state, optional screen + colors  
2. Decide action from tree (which `id`, focused, checked, …)  
3. `hover` / `click` / `type` / `key`  
4. `styles` or `tree` again — verify hover colors, text, selection  
5. Repeat  

This is **observation → action → observation**, similar to a visual debugger without breakpoints.

### Wire your own app

Replace the demo root with your UI:

```cpp
#include <tuinator/testing/app_driver.hpp>
#include <tuinator/testing/json_protocol.hpp>

int main() {
    tuinator::testing::AppDriver driver({120, 40}, build_my_root());
    return tuinator::testing::run_json_protocol_loop(driver);
}
```

For production binaries you normally ship **without** this `main`. Use a **`myapp-test-agent`** target or internal build flavor. Agent builds should use **`TUINATOR_ENABLE_TEST_IDS=ON`** (Debug or explicit CMake flag) so `id` in JSON resolves.

A thin **MCP server** or Python wrapper can spawn that process and forward lines to stdin/out.

---

## Choosing an approach

| Goal | Use |
|------|-----|
| Many toggles, inputs, render lines | Widget tests + `render_helper` |
| Hover row, same background everywhere | `AppDriver` + `analyze_selector` |
| Full login flow, focus order | `AppDriver` |
| LLM explores UI dynamically | JSON protocol + custom agent binary |
| CI on every PR | `tuinator-tests` (C++) |

---

## CMake reference

| Variable | Meaning |
|----------|---------|
| `TUINATOR_BUILD_TESTS` | Build `tuinator-tests` |
| `TUINATOR_BUILD_EXAMPLES` | Includes `tuinator-testing-agent` |
| `TUINATOR_ENABLE_TEST_IDS` | Store test IDs (see table above) |

---

## Related docs

- [`API.md`](API.md) — short testing section and API overview  
- [`rendering-pitfalls.md`](rendering-pitfalls.md) — paint/dirty rules for stable snapshots  
- `tests/core/test_testing_driver.cpp` — AppDriver, JSON, region style examples  
