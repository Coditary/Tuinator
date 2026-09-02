#include <tuinator/terminal/ansi_terminal_buffer.hpp>

#include <tuinator/render/color.hpp>
#include <algorithm>
#include <string>

extern "C" {
#include <vterm.h>
#include <vterm_keycodes.h>
}

namespace tuinator {

namespace {

std::string utf8_from(char32_t cp) {
    std::string out;
    if (cp <= 0x7F) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    return out;
}

Color indexed_color(int index) {
    static constexpr Color palette[] = {
        Color::Black, Color::Red, Color::Green, Color::Yellow,
        Color::Blue, Color::Magenta, Color::Cyan, Color::White,
    };

    if (index >= 0 && index < 8) {
        return palette[index];
    }
    if (index >= 8 && index < 16) {
        static constexpr Color bright_palette[] = {
            Color::Black, Color::Red, Color::Green, Color::Yellow,
            Color::Blue, Color::Magenta, Color::Cyan, Color::White,
        };
        return bright_palette[index - 8];
    }
    return Color::Default;
}

void apply_vterm_color(
    const VTermScreen* screen,
    VTermColor color,
    bool bold,
    bool is_foreground,
    Style& style) {
    const bool is_default = is_foreground
        ? VTERM_COLOR_IS_DEFAULT_FG(&color)
        : VTERM_COLOR_IS_DEFAULT_BG(&color);

    if (is_default) {
        if (is_foreground) {
            style.foreground = Color::Default;
        } else {
            style.background = Color::Default;
        }
        return;
    }

    bool emit_bold = bold;
    if (bold && VTERM_COLOR_IS_INDEXED(&color) && color.indexed.idx >= 0 && color.indexed.idx < 8) {
        vterm_color_indexed(&color, static_cast<uint8_t>(color.indexed.idx + 8));
        emit_bold = false;
    }

    vterm_screen_convert_color_to_rgb(screen, &color);
    if (VTERM_COLOR_IS_RGB(&color)) {
        const Rgb rgb{color.rgb.red, color.rgb.green, color.rgb.blue};
        if (is_foreground) {
            style.foreground_rgb = rgb;
            if (emit_bold) {
                style.bold = true;
            }
        } else {
            style.background_rgb = rgb;
        }
        return;
    }

    if (VTERM_COLOR_IS_INDEXED(&color)) {
        if (is_foreground) {
            style.foreground = indexed_color(color.indexed.idx);
            if (emit_bold) {
                style.bold = true;
            }
        } else {
            style.background = indexed_color(color.indexed.idx);
        }
    }
}

Style style_from_cell(const VTermScreen* screen, const VTermScreenCell& cell) {
    Style style{};

    apply_vterm_color(screen, cell.fg, cell.attrs.bold != 0, true, style);
    apply_vterm_color(screen, cell.bg, false, false, style);

    style.dim = false;
    style.reverse = cell.attrs.reverse;
    return style;
}

VTermModifier modifier_from_key(const KeyPress& key) {
    VTermModifier mod = VTERM_MOD_NONE;
    if (key.ctrl) {
        mod = static_cast<VTermModifier>(mod | VTERM_MOD_CTRL);
    }
    if (key.alt) {
        mod = static_cast<VTermModifier>(mod | VTERM_MOD_ALT);
    }
    return mod;
}

bool map_special_key(const KeyPress& key, VTermKey& out) {
    switch (key.key) {
    case Key::Enter: out = VTERM_KEY_ENTER; return true;
    case Key::Tab: out = VTERM_KEY_TAB; return true;
    case Key::BackTab: out = VTERM_KEY_TAB; return true;
    case Key::Backspace: out = VTERM_KEY_BACKSPACE; return true;
    case Key::Escape: out = VTERM_KEY_ESCAPE; return true;
    case Key::Up: out = VTERM_KEY_UP; return true;
    case Key::Down: out = VTERM_KEY_DOWN; return true;
    case Key::Left: out = VTERM_KEY_LEFT; return true;
    case Key::Right: out = VTERM_KEY_RIGHT; return true;
    case Key::Home: out = VTERM_KEY_HOME; return true;
    case Key::End: out = VTERM_KEY_END; return true;
    case Key::Delete: out = VTERM_KEY_DEL; return true;
    case Key::PageUp: out = VTERM_KEY_PAGEUP; return true;
    case Key::PageDown: out = VTERM_KEY_PAGEDOWN; return true;
    default: return false;
    }
}

} // namespace

struct AnsiTerminalBuffer::Impl {
    VTerm* vt = nullptr;
    VTermScreen* screen = nullptr;
    VTermScreenCallbacks callbacks{};
    int cursor_row = 0;
    int cursor_col = 0;
    bool cursor_visible = true;

    static int on_movecursor(VTermPos pos, VTermPos /*oldpos*/, int visible, void* user) {
        auto* self = static_cast<Impl*>(user);
        self->cursor_row = pos.row;
        self->cursor_col = pos.col;
        self->cursor_visible = visible != 0;
        return 0;
    }

    static int on_settermprop(VTermProp prop, VTermValue* val, void* user) {
        auto* self = static_cast<Impl*>(user);
        if (prop == VTERM_PROP_CURSORVISIBLE) {
            self->cursor_visible = val->boolean != 0;
        }
        return 1;
    }

    void install_callbacks() {
        if (screen == nullptr) {
            return;
        }

        callbacks = {};
        callbacks.movecursor = on_movecursor;
        callbacks.settermprop = on_settermprop;
        vterm_screen_set_callbacks(screen, &callbacks, this);

        VTermState* state = vterm_obtain_state(vt);
        if (state == nullptr) {
            return;
        }

        vterm_state_set_bold_highbright(state, 1);

        VTermPos pos{};
        vterm_state_get_cursorpos(state, &pos);
        cursor_row = pos.row;
        cursor_col = pos.col;
    }

    void clear_callbacks() {
        if (screen != nullptr) {
            vterm_screen_set_callbacks(screen, nullptr, nullptr);
        }
    }
};

AnsiTerminalBuffer::AnsiTerminalBuffer() : impl_(std::make_unique<Impl>()) {}

AnsiTerminalBuffer::~AnsiTerminalBuffer() {
    if (impl_->vt != nullptr) {
        impl_->clear_callbacks();
        vterm_free(impl_->vt);
        impl_->vt = nullptr;
        impl_->screen = nullptr;
    }
}

void AnsiTerminalBuffer::resize(Size size) {
    size_.width = std::max(1, size.width);
    size_.height = std::max(1, size.height);

    if (impl_->vt == nullptr) {
        impl_->vt = vterm_new(size_.height, size_.width);
        vterm_set_utf8(impl_->vt, 1);
        impl_->screen = vterm_obtain_screen(impl_->vt);
        vterm_screen_enable_reflow(impl_->screen, 0);
        vterm_screen_reset(impl_->screen, 1);
        impl_->install_callbacks();

        VTermState* state = vterm_obtain_state(impl_->vt);
        if (state != nullptr) {
            vterm_state_set_bold_highbright(state, 1);
        }
        return;
    }

    vterm_set_size(impl_->vt, size_.height, size_.width);
}

void AnsiTerminalBuffer::reset() {
    if (impl_->screen != nullptr) {
        vterm_screen_reset(impl_->screen, 1);
    }
}

void AnsiTerminalBuffer::feed(std::string_view bytes) {
    if (impl_->vt == nullptr || bytes.empty()) {
        return;
    }

    vterm_input_write(impl_->vt, bytes.data(), bytes.size());
    if (impl_->screen != nullptr) {
        vterm_screen_flush_damage(impl_->screen);
    }
}

std::string AnsiTerminalBuffer::keyboard_bytes(const KeyPress& key) const {
    if (impl_->vt == nullptr) {
        return {};
    }

    VTermModifier mod = modifier_from_key(key);
    if (key.key == Key::BackTab) {
        mod = static_cast<VTermModifier>(mod | VTERM_MOD_SHIFT);
    }

    VTermKey special = VTERM_KEY_NONE;
    if (map_special_key(key, special)) {
        vterm_keyboard_key(impl_->vt, special, mod);
    } else if (key.character != '\0') {
        vterm_keyboard_unichar(impl_->vt, static_cast<uint32_t>(static_cast<unsigned char>(key.character)), mod);
    } else {
        return {};
    }

    std::string out;
    char buffer[64];
    while (true) {
        const size_t bytes = vterm_output_read(impl_->vt, buffer, sizeof(buffer));
        if (bytes == 0) {
            break;
        }
        out.append(buffer, bytes);
    }
    return out;
}

bool AnsiTerminalBuffer::cell_at(int row, int col, std::string& glyph, Style& style) const {
    glyph.clear();
    style = Style{};
    if (impl_->screen == nullptr) {
        return false;
    }

    VTermScreenCell cell{};
    VTermPos pos{};
    pos.row = row;
    pos.col = col;
    if (!vterm_screen_get_cell(impl_->screen, pos, &cell)) {
        return false;
    }
    if (cell.chars[0] == static_cast<uint32_t>(-1)) {
        return false;
    }

    style = style_from_cell(impl_->screen, cell);
    if (cell.chars[0] != 0) {
        for (int i = 0; i < VTERM_MAX_CHARS_PER_CELL && cell.chars[i] != 0; ++i) {
            glyph += utf8_from(static_cast<char32_t>(cell.chars[i]));
        }
    }
    return true;
}

void AnsiTerminalBuffer::cursor_position(int& row, int& col, bool& visible) const {
    row = impl_->cursor_row;
    col = impl_->cursor_col;
    visible = impl_->cursor_visible;

    if (impl_->vt == nullptr) {
        visible = false;
        return;
    }

    VTermState* state = vterm_obtain_state(impl_->vt);
    if (state == nullptr) {
        visible = false;
        return;
    }

    VTermPos pos{};
    vterm_state_get_cursorpos(state, &pos);
    row = pos.row;
    col = pos.col;
}

bool cell_needs_background_fill(const Style& style) {
    return style.background != Color::Default
        || style.background_rgb.has_value()
        || style.reverse;
}

void AnsiTerminalBuffer::paint(PaintContext& ctx, Point origin) const {
    if (impl_->screen == nullptr) {
        return;
    }

    Canvas& canvas = ctx.canvas;
    for (int row = 0; row < size_.height; ++row) {
        for (int col = 0; col < size_.width; ++col) {
            VTermScreenCell cell{};
            VTermPos pos{};
            pos.row = row;
            pos.col = col;
            if (!vterm_screen_get_cell(impl_->screen, pos, &cell)) {
                continue;
            }

            if (cell.chars[0] == static_cast<uint32_t>(-1)) {
                continue;
            }

            const int cell_width = std::max(1, static_cast<int>(cell.width));
            const Style style = style_from_cell(impl_->screen, cell);

            if (cell.chars[0] == 0) {
                for (int w = 0; w < cell_width; ++w) {
                    canvas.draw_char({origin.x + col + w, origin.y + row}, ' ', style);
                }
            } else {
                if (cell_needs_background_fill(style)) {
                    for (int w = 0; w < cell_width; ++w) {
                        canvas.draw_char({origin.x + col + w, origin.y + row}, ' ', style);
                    }
                }

                std::string glyph;
                for (int i = 0; i < VTERM_MAX_CHARS_PER_CELL && cell.chars[i] != 0; ++i) {
                    glyph += utf8_from(static_cast<char32_t>(cell.chars[i]));
                }
                canvas.draw_text({origin.x + col, origin.y + row}, glyph, style);
            }

            col += cell_width - 1;
        }
    }
}

} // namespace tuinator
