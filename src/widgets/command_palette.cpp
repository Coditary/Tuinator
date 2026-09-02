#include <tuinator/widgets/command_palette.hpp>

#include <tuinator/core/event.hpp>
#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/text.hpp>
#include <tuinator/render/theme.hpp>

#include <algorithm>
#include <cctype>
#include <variant>

namespace tuinator {

namespace {

bool fuzzy_match(std::string_view haystack, std::string_view needle) {
    if (needle.empty()) {
        return true;
    }

    std::size_t start = 0;
    for (char ch : needle) {
        const char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        bool found = false;
        for (std::size_t i = start; i < haystack.size(); ++i) {
            const char candidate =
                static_cast<char>(std::tolower(static_cast<unsigned char>(haystack[i])));
            if (candidate == lower) {
                start = i + 1;
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

Style palette_border_style() {
    Style style{};
    style.foreground = Color::Cyan;
    return style;
}

} // namespace

CommandPalette::CommandPalette() = default;

void CommandPalette::set_entries(std::vector<CommandPaletteEntry> entries) {
    entries_ = std::move(entries);
    rebuild_matches();
}

void CommandPalette::set_on_close(std::function<void()> callback) {
    on_close_ = std::move(callback);
}

void CommandPalette::open() {
    open_ = true;
    query_.clear();
    selected_ = 0;
    rebuild_matches();
    mark_layout_dirty();
}

void CommandPalette::close() {
    if (!open_) {
        return;
    }
    open_ = false;
    query_.clear();
    selected_ = 0;
    matches_.clear();
    mark_layout_dirty();
    if (on_close_) {
        on_close_();
    }
}

int CommandPalette::panel_width() const {
    int width = 36;
    for (const int index : matches_) {
        const CommandPaletteEntry& entry = entries_[static_cast<std::size_t>(index)];
        width = std::max(width, text_display_width(entry.label) + text_display_width(entry.shortcut) + 8);
    }
    return std::min(width, std::max(24, bounds_.width - 4));
}

int CommandPalette::panel_height() const {
    const int rows = std::min(8, static_cast<int>(matches_.size()));
    return rows + 4;
}

Size CommandPalette::preferred_size() const {
    if (!open_) {
        return {0, 0};
    }
    return {panel_width(), panel_height()};
}

void CommandPalette::rebuild_matches() {
    matches_.clear();
    for (int i = 0; i < static_cast<int>(entries_.size()); ++i) {
        const CommandPaletteEntry& entry = entries_[static_cast<std::size_t>(i)];
        const std::string haystack = entry.label + " " + entry.category + " " + entry.id;
        if (fuzzy_match(haystack, query_)) {
            matches_.push_back(i);
        }
    }

    selected_ = std::clamp(selected_, 0, std::max(0, static_cast<int>(matches_.size()) - 1));
    mark_dirty();
}

void CommandPalette::move_selection(int delta) {
    if (matches_.empty()) {
        return;
    }

    selected_ = (selected_ + delta + static_cast<int>(matches_.size())) % static_cast<int>(matches_.size());
    mark_dirty();
}

void CommandPalette::activate_selection() {
    if (matches_.empty()) {
        return;
    }

    const CommandPaletteEntry& entry = entries_[static_cast<std::size_t>(matches_[static_cast<std::size_t>(selected_)])];
    close();
    if (entry.action) {
        entry.action();
    }
}

void CommandPalette::paint(Canvas& canvas) const {
    if (!open_) {
        return;
    }

    const int width = panel_width();
    const int height = panel_height();
    const int x = std::max(0, (bounds_.width - width) / 2);
    const int y = 1;

    Style dim{};
    dim.dim = true;
    canvas.fill_rect({0, 0, bounds_.width, bounds_.height}, ' ', dim);

    const Rect box{x, y, width, height};
    canvas.draw_box(box, palette_border_style(), unicode_rounded_border_glyphs());

    const Theme theme = dark_theme();
    canvas.draw_text({x + 2, y + 1}, "> " + query_ + "_", theme.heading);

    const int list_y = y + 2;
    const int visible = std::min(8, static_cast<int>(matches_.size()));
    for (int row = 0; row < visible; ++row) {
        const CommandPaletteEntry& entry =
            entries_[static_cast<std::size_t>(matches_[static_cast<std::size_t>(row)])];
        const bool selected = row == selected_;
        Style row_style = selected ? theme.button_focused : theme.label;
        std::string line = entry.label;
        if (!entry.category.empty()) {
            line += "  (" + entry.category + ")";
        }
        canvas.draw_text({x + 2, list_y + row}, (selected ? "> " : "  ") + line, row_style);
        if (!entry.shortcut.empty()) {
            const int shortcut_x = x + width - 2 - text_display_width(entry.shortcut);
            Style shortcut_style = theme.muted;
            shortcut_style.dim = true;
            canvas.draw_text({shortcut_x, list_y + row}, entry.shortcut, shortcut_style);
        }
    }
}

bool CommandPalette::handle_event(const Event& event) {
    if (!open_) {
        return false;
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key) {
        return true;
    }

    if (key->key == Key::Escape) {
        close();
        return true;
    }

    if (key->key == Key::Up) {
        move_selection(-1);
        return true;
    }
    if (key->key == Key::Down) {
        move_selection(1);
        return true;
    }
    if (key->key == Key::Enter) {
        activate_selection();
        return true;
    }
    if (key->key == Key::Backspace) {
        if (!query_.empty()) {
            query_.pop_back();
            rebuild_matches();
        }
        return true;
    }

    if (key->character >= 32 && key->character <= 126) {
        query_.push_back(key->character);
        rebuild_matches();
        return true;
    }

    return true;
}

} // namespace tuinator
