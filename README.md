# Tuinator

A modern C++ terminal UI library with a pluggable terminal backend (ncursesw on POSIX, PDCurses on Windows).

## Requirements

- C++17 compiler
- CMake 3.16+
- **Linux / macOS / BSD:** ncursesw development headers
- **Windows:** [PDCurses](https://pdcurses.org/) or PDCursesMod (e.g. via vcpkg)

### Fedora / RHEL

```bash
sudo dnf install ncurses-devel
```

### Debian / Ubuntu

```bash
sudo apt install libncursesw-dev
```

### macOS

```bash
brew install ncurses
```

### Windows

Install PDCurses, then configure with its install prefix:

```powershell
vcpkg install pdcurses
cmake -B build -DTUINATOR_PDCURSES_ROOT=C:/path/to/vcpkg/installed/x64-windows
cmake --build build
```

Or set `PDCURSES_INCLUDE_DIR` / `PDCURSES_LIBRARY` manually if CMake finds them on `PATH`.

## Platform backends

Tuinator uses a small `TerminalBackend` interface. The default factory picks the platform backend automatically:

```cpp
auto backend = tuinator::TerminalBackend::create();  // ncursesw or PDCurses
```

| Platform | Backend | CMake define |
|----------|---------|--------------|
| Linux / macOS / BSD | ncursesw | `TUINATOR_BACKEND_NCURSES` |
| Windows | PDCurses | `TUINATOR_BACKEND_PDCURSES` |

The widget/event code is shared; only the curses layer differs. See `include/tuinator/backend/terminal_backend.hpp` and `include/tuinator/backend/platform.hpp`.

## Build

```bash
make              # build all demos (only if needed)
make counter      # build counter if needed, then run it
make help         # show all targets
```

Or with CMake directly:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Use as a library

Tuinator is a CMake package. You can consume it in three ways.

### 1. Installed package (`find_package`)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix ~/.local
```

In your `CMakeLists.txt`:

```cmake
find_package(Tuinator CONFIG REQUIRED)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE tuinator::tuinator)
```

```cpp
#include <tuinator/tuinator.hpp>

int main() {
    tuinator::Application app;
    app.set_root(std::make_unique<tuinator::Label>("Hello from Tuinator"));
    return app.run();
}
```

Point CMake at the install prefix:

```bash
cmake -B build -DCMAKE_PREFIX_PATH=$HOME/.local
```

### 2. FetchContent (GitHub / local path)

```cmake
include(FetchContent)

FetchContent_Declare(
    tuinator
    GIT_REPOSITORY https://github.com/Coditary/Tuinator.git
    GIT_TAG v0.1.0
)
set(TUINATOR_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(TUINATOR_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(tuinator)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE tuinator::tuinator)
```

### 3. `add_subdirectory`

```cmake
set(TUINATOR_BUILD_EXAMPLES OFF)
set(TUINATOR_BUILD_TESTS OFF)
add_subdirectory(third_party/Tuinator)

target_link_libraries(myapp PRIVATE tuinator::tuinator)
```

### CMake options

| Option | Default (standalone) | Description |
|--------|-------------------|-------------|
| `TUINATOR_BUILD_EXAMPLES` | ON | Build demo programs |
| `TUINATOR_BUILD_TESTS` | ON | Build unit tests |
| `TUINATOR_BUILD_SHARED_LIBS` | OFF | Build shared instead of static library |
| `TUINATOR_INSTALL` | ON | Generate `cmake --install` rules |

When Tuinator is pulled in as a dependency (`add_subdirectory` / `FetchContent`), examples and tests default to **OFF**.

## Test everything

```bash
make unit-test   # headless unit tests (MemoryTerminalBackend)
make test        # smoke-test all demos (auto-quits with q)
```

Or with CTest directly:

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

## Run the examples

```bash
make help         # list all make targets
make windows      # floating windows + modal dialog
make counter      # build + run the counter demo
```

Or use the helper script:

```bash
./examples/run-demos.sh          # lists all demos
./examples/run-demos.sh counter  # runs a specific demo
```

| Demo | Command | What it shows |
|------|---------|---------------|
| Hello | `./build/tuinator-hello` | Centered label |
| Colors | `./build/tuinator-colors` | All colors + bold/dim/reverse |
| Layout | `./build/tuinator-layout` | VBox/HBox nesting, resize |
| Counter | `./build/tuinator-counter` | Buttons updating live state |
| Buttons | `./build/tuinator-buttons` | Tab focus across many buttons |
| Form | `./build/tuinator-form` | Login form with TextInput |
| Dashboard | `./build/tuinator-dashboard` | App shell: SplitPane, Tabs, Grid, Panel, StatusBar |
| Data | `./build/tuinator-data` | ListView + Table + Application timers |
| Controls | `./build/tuinator-controls` | ComboBox, Slider, TreeView, MenuBar, Dialog |
| Scroll | `./build/tuinator-scroll` | ScrollView with toggles, Tab focus, add rows |
| Image | `./build/tuinator-image` | Terminal image test (Kitty/Sixel/iTerm2) |
| Textarea | `./build/tuinator-textarea` | Multi-line editor with line numbers |
| Throbber | `./build/tuinator-throbber` | Animated loading spinners |
| Windows | `./build/tuinator-windows` | Floating windows, drag, resize, modal |

Press `q` to quit (or use the Quit button in the buttons demo).  
Use the **mouse** to click buttons, or `Tab` / `Shift+Tab` for keyboard focus. `Enter` / `Space` activates the focused widget.  
In the windows demo: drag title bars, resize with `#` corner, `Esc` closes top window.

### Mouse not working?

1. Rebuild and try: `make mouse-test`
2. **Ghostty** (`xterm-ghostty`): supported — if clicks fail after an older build, rebuild (`make build`)
3. Debug log (does not corrupt the screen): `TUINATOR_MOUSE_DEBUG=1 make mouse-test` → `/tmp/tuinator-mouse.log` (Linux/macOS) or `%TEMP%\tuinator-mouse.log` (Windows)
4. **tmux:** run `tmux set -g mouse off` — tmux captures clicks by default

Smooth tracking uses xterm **mode 1003** (all motion events) via `/dev/tty`, like the classic ncurses gist. Override with `TUINATOR_MOUSE_TRACK=1002` for drag-only. Optional hardware `+` cursor: `TUINATOR_MOUSE_CURSOR=1` to enable (off by default; always hidden in shell terminals).

## Phase 0 status

- [x] CMake setup with ncursesw / PDCurses backend selection
- [x] RAII `Application` lifecycle
- [x] Terminal resize events
- [x] `Label` widget with centered rendering
- [x] Event loop (`q` to exit)

## Phase 1 status

- [x] Widget tree with children
- [x] `Canvas` clipping
- [x] `VBox` / `HBox` layout
- [x] `Button` widget
- [x] Keyboard focus with Tab navigation
- [x] Damage tracking (redraw only when needed)

## Phase 2 status

- [x] `Window` with title bar and border
- [x] `Desktop` window manager with Z-order
- [x] Drag (title bar) and resize (`#` handle)
- [x] Modal dialogs with dimmed background
- [x] Mouse support
- [x] `make test` smoke tests for all demos

## Phase 3 status

- [x] `TextInput` — editable fields with cursor, placeholder, submit callback
- [x] `ScrollView` — scroll long content (arrows, PgUp/PgDn, mouse wheel)
- [x] `Separator` — horizontal divider line
- [x] `Theme` — dark/light semantic style presets (`app.set_theme()`)
- [x] Updated form demo with live TextInput preview
- [x] GitHub Actions CI (Linux + macOS)

## Phase 4 status

- [x] `Toggle` — checkbox-style on/off control (click, Space/Enter)
- [x] `ScrollView` focus — inner widgets in Tab order, click-to-focus
- [x] Auto-scroll — focused widget stays visible when tabbing
- [x] Scroll vs. widget focus — Up/Down bewegen Fokus in der Liste; Wheel/PgUp/PgDn scrollen
- [x] Updated scroll demo — mixed rows (Label + Toggle), dynamic add rows

## Phase 5 status

- [x] **Flex layout** — `set_flex()` on widgets; VBox/HBox expand children to fill space
- [x] **Grid** — row/column layout with configurable columns
- [x] **Panel** — bordered container with optional title
- [x] **SplitPane** — horizontal/vertical two-pane layout
- [x] **Tabs** — tab bar with switchable content (click, 1–9, ←/→ when focused)
- [x] **StatusBar** — single-line status/footer bar
- [x] **Dashboard demo** — full-screen app shell combining all Phase 5 widgets
- [x] **`WindowHost`** — nested floating windows clipped inside a parent window

## Phase 6 status

- [x] **Timers** — `set_interval()`, `set_timeout()`, `cancel_timer()` on `Application`
- [x] **`ListView`** — selectable list with keyboard/mouse navigation
- [x] **`Table`** — column headers and selectable rows
- [x] **`MemoryTerminalBackend`** — in-memory backend with `snapshot()` for tests
- [x] **TextInput selection** — Ctrl+A select all, visual highlight, delete/replace selection
- [x] **Data demo** — ListView + Table + live status timer (`make data`)

## Phase 7 status

- [x] **Unit tests** — headless tests for text metrics, Label, layout, ListView, memory backend
- [x] **CI Windows** — PDCurses build + unit tests via vcpkg
- [x] **API docs** — [`docs/API.md`](docs/API.md) overview and testing guide
- [x] **Versioning** — `TUINATOR_VERSION` in `<tuinator/version.hpp>`, [`CHANGELOG.md`](CHANGELOG.md)

## Border glyphs (ASCII / Unicode)

Tuinator picks border characters based on terminal UTF-8 support. With a UTF-8 locale you get box-drawing characters (`┌`, `─`, `│`, …); otherwise ASCII (`+`, `-`, `|`).

```cpp
// Auto-detect (default)
app.set_theme(tuinator::dark_theme());

// Explicit glyph set
app.set_theme(tuinator::dark_theme({.glyphs = tuinator::GlyphSet::Unicode}));
app.set_theme(tuinator::dark_theme({.glyphs = tuinator::GlyphSet::UnicodeRounded}));
app.set_theme(tuinator::dark_theme({.glyphs = tuinator::GlyphSet::Ascii}));
```

Environment overrides:

| Variable | Values |
|----------|--------|
| `TUINATOR_GLYPHS` | `ascii`, `unicode` (or `box`), `rounded` |
| `TUINATOR_ASCII` | `1` forces ASCII borders |

Compare all presets: `make theme`

## True color (24-bit)

Tuinator supports 24-bit RGB when the terminal reports true color support (`COLORTERM=truecolor` or `24bit`, common in Ghostty, Kitty, WezTerm, iTerm2, Alacritty).

```cpp
using tuinator::rgb;
using tuinator::Rgb;

// Recommended: hex formatter with ANSI fallback built in
tuinator::style_hex_fg(0xFF8800);
tuinator::style_hex_fg_bg(0xFF8800, 0x0A1628);

// Or explicitly:
tuinator::ColorValue::from_hex(0xFF8800).foreground_style();

// Low-level (manual):
tuinator::Style{.foreground_rgb = rgb(255, 128, 64)};
```

When true color is unavailable, `style_hex_*` and `ColorValue` automatically pick the nearest ANSI palette color (`foreground` / `background`). When true color works, the RGB value is used via direct ANSI rendering.

On terminals without true color, RGB-only styles fall back to the nearest 8-color palette entry.
