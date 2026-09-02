#include <tuinator/tuinator.hpp>

#include <algorithm>
#include <cctype>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

// Tokyo Night–inspired palette (diffview.nvim look).
struct NvPalette {
    tuinator::Rgb bg{0x1a, 0x1b, 0x26};
    tuinator::Rgb sidebar{0x16, 0x16, 0x1e};
    tuinator::Rgb border{0x29, 0x2e, 0x42};
    tuinator::Rgb text{0xc0, 0xca, 0xf5};
    tuinator::Rgb muted{0x56, 0x5f, 0x89};
    tuinator::Rgb cyan{0x7d, 0xcf, 0xff};
    tuinator::Rgb purple{0xbb, 0x9a, 0xf7};
    tuinator::Rgb orange{0xe0, 0xaf, 0x68};
    tuinator::Rgb green{0x9e, 0xce, 0x6a};
    tuinator::Rgb red{0xf7, 0x76, 0x8e};
    tuinator::Rgb del_bg{0x40, 0x28, 0x38};
    tuinator::Rgb add_bg{0x28, 0x34, 0x57};
    tuinator::Rgb selection{0x3b, 0x42, 0x61};
    tuinator::Rgb tab_active{0x29, 0x32, 0x50};
    tuinator::Rgb status_accent{0x7d, 0xcf, 0xff};
    tuinator::Rgb cursor_line{0x29, 0x2e, 0x42};
    tuinator::Rgb hunk_bar_add{0x9e, 0xce, 0x6a};
    tuinator::Rgb hunk_bar_del{0xe0, 0xaf, 0x68};

    tuinator::Style bg_style() const { return tuinator::style_fg_bg(text, bg); }
    tuinator::Style sidebar_style() const { return tuinator::style_fg_bg(text, sidebar); }
    tuinator::Style muted_style() const { return tuinator::style_fg_bg(muted, sidebar); }
    tuinator::Style border_style() const { return tuinator::style_fg_bg(border, bg); }
    tuinator::Style del_row() const { return tuinator::style_fg_bg(text, del_bg); }
    tuinator::Style add_row() const { return tuinator::style_fg_bg(text, add_bg); }
    tuinator::Style cursor_row() const { return tuinator::style_fg_bg(cyan, cursor_line); }
};

tuinator::Style file_icon_row_style(const NvPalette& palette, tuinator::FileIcon icon, tuinator::Rgb row_bg) {
    (void)palette;
    return tuinator::style_fg_bg(tuinator::file_icon_color(icon), row_bg);
}

tuinator::SourceControlPanelStyle make_source_control_style(const NvPalette& palette) {
    tuinator::SourceControlPanelStyle style;
    style.panel_bg = palette.sidebar;
    style.text_fg = palette.text;
    style.muted_fg = palette.muted;
    style.selection_bg = palette.selection;
    style.status_modified = palette.orange;
    style.status_added = palette.green;
    style.status_deleted = palette.red;
    style.preferred_width = 56;
    style.background = palette.sidebar_style();
    style.header = palette.sidebar_style();
    style.section_title = palette.sidebar_style();
    style.entry_text = palette.sidebar_style();
    style.muted = palette.muted_style();
    style.footer = palette.muted_style();
    return style;
}

tuinator::StatusLineStyle make_status_line_style(const NvPalette& palette) {
    tuinator::StatusLineStyle style;
    style.background = palette.bg_style();
    return style;
}

std::unique_ptr<tuinator::StatusLine> make_diffview_status_line(const NvPalette& palette) {
    auto status = std::make_unique<tuinator::StatusLine>(make_status_line_style(palette));
    status->set_left({
        {
            .kind = tuinator::StatusSegmentKind::Box,
            .text = " DIFFVIEWFILES ",
            .foreground_rgb = palette.text,
            .background_rgb = palette.sidebar,
        },
        {
            .kind = tuinator::StatusSegmentKind::Separator,
            .line_icon = tuinator::LineIcon::RightHalfCircleThin,
            .foreground_rgb = palette.status_accent,
        },
        {
            .text = "LUA",
            .foreground_rgb = palette.text,
            .bold = true,
        },
        {
            .icon = tuinator::FileIcon::Lua,
            .text = " file_history_panel.lua",
            .foreground_rgb = palette.status_accent,
        },
    });
    status->set_right({
        {
            .kind = tuinator::StatusSegmentKind::Pill,
            .text = " NORMAL ",
            .foreground_rgb = palette.bg,
            .background_rgb = palette.status_accent,
            .bold = true,
        },
        {
            .ui_icon = tuinator::UiIcon::File,
            .text = " 9.5k",
            .foreground_rgb = palette.status_accent,
        },
        {
            .icon = tuinator::FileIcon::Lua,
            .text = " file_history_panel.lua",
            .foreground_rgb = palette.purple,
        },
        {
            .ui_icon = tuinator::UiIcon::DiffAdded,
            .text = "43",
            .foreground_rgb = palette.green,
        },
        {
            .ui_icon = tuinator::UiIcon::DiffModified,
            .text = "12",
            .foreground_rgb = palette.cyan,
        },
        {
            .ui_icon = tuinator::UiIcon::DiffRemoved,
            .text = "244",
            .foreground_rgb = palette.red,
        },
        {.text = "87:1", .foreground_rgb = palette.muted},
        {.text = "26%", .foreground_rgb = palette.muted},
        {
            .ui_icon = tuinator::UiIcon::SymbolNumeric,
            .text = " 333",
            .foreground_rgb = palette.muted,
        },
        {
            .ui_icon = tuinator::UiIcon::Warning,
            .text = " UTF-8",
            .foreground_rgb = palette.muted,
        },
        {
            .ui_icon = tuinator::UiIcon::GitBranch,
            .text = " feat/file-history",
            .foreground_rgb = palette.purple,
        },
    });
    return status;
}

std::vector<tuinator::SourceControlSection> make_source_sections() {
    return {
        {
            "Changes (12)",
            {
                {.path = "file_history_panel.lua", .status = tuinator::GitChangeStatus::Modified, .additions = "55", .deletions = "29", .selected = true},
                {.path = "diffview.lua", .status = tuinator::GitChangeStatus::Modified, .additions = "21", .deletions = "9"},
                {.path = "file_history_view.lua", .status = tuinator::GitChangeStatus::Modified, .additions = "17", .deletions = "6"},
                {.path = "listeners.lua", .status = tuinator::GitChangeStatus::Modified, .additions = "9", .deletions = "3"},
                {.path = "buffer.lua", .status = tuinator::GitChangeStatus::Modified, .additions = "12", .deletions = "4"},
                {.path = "command.lua", .status = tuinator::GitChangeStatus::Modified, .additions = "8", .deletions = "2"},
                {.path = "config.lua", .status = tuinator::GitChangeStatus::Modified, .additions = "3", .deletions = "1"},
                {.path = "history.lua", .status = tuinator::GitChangeStatus::Modified, .additions = "6", .deletions = "2"},
                {.path = "init.lua", .status = tuinator::GitChangeStatus::Modified, .additions = "4", .deletions = "1"},
                {.path = "state.lua", .status = tuinator::GitChangeStatus::Modified, .additions = "5", .deletions = "2"},
                {.path = "ui.lua", .status = tuinator::GitChangeStatus::Modified, .additions = "7", .deletions = "1"},
                {.path = "diffview.vim", .status = tuinator::GitChangeStatus::Modified, .additions = "4", .deletions = "4"},
            },
        },
        {
            "Staged changes (6)",
            {
                {.path = "diffview/main.lua", .status = tuinator::GitChangeStatus::Modified, .additions = "10", .deletions = "2"},
                {.path = "diffview/help.lua", .status = tuinator::GitChangeStatus::Modified, .additions = "4", .deletions = "0"},
                {.path = "diffview/hunk.lua", .status = tuinator::GitChangeStatus::Modified, .additions = "6", .deletions = "1"},
                {.path = "diffview/list.lua", .status = tuinator::GitChangeStatus::Modified, .additions = "3", .deletions = "0"},
                {.path = "diffview/render.lua", .status = tuinator::GitChangeStatus::Modified, .additions = "8", .deletions = "3"},
                {.path = "diffview/state.lua", .status = tuinator::GitChangeStatus::Modified, .additions = "2", .deletions = "1"},
            },
        },
    };
}

std::string truncate_middle(std::string_view text, std::size_t max_len) {
    if (text.size() <= max_len) {
        return std::string(text);
    }
    if (max_len <= 3) {
        return std::string(text.substr(0, max_len));
    }
    const std::size_t head = (max_len - 3) / 2;
    const std::size_t tail = max_len - 3 - head;
    return std::string(text.substr(0, head)) + "..." + std::string(text.substr(text.size() - tail));
}

struct DiffRow {
    std::optional<int> line_no;
    std::string text;
    bool hunk = false;
};

DiffRow code_row(int line_no, std::string text, bool hunk = false) {
    return {.line_no = line_no, .text = std::move(text), .hunk = hunk};
}

struct SplitDiffRow {
    std::optional<DiffRow> left;
    std::optional<DiffRow> right;
};

tuinator::Style with_bg(const NvPalette& palette, tuinator::Rgb bg) {
    return tuinator::style_fg_bg(palette.text, bg);
}

bool is_lua_keyword(std::string_view word) {
    static constexpr const char* kKeywords[] = {
        "function", "local", "return", "if", "then", "else", "end", "for", "in", "do",
        "nil",  "true", "false", "self", "require",
    };
    for (const char* keyword : kKeywords) {
        if (word == keyword) {
            return true;
        }
    }
    return false;
}

void paint_lua_line(
    tuinator::Canvas& canvas,
    int x,
    int y,
    std::string_view line,
    const NvPalette& palette,
    tuinator::Style row_style) {
    if (line.size() >= 2 && line[0] == '-' && line[1] == '-') {
        auto style = row_style;
        style.foreground_rgb = palette.muted;
        style.dim = true;
        canvas.draw_text({x, y}, line, style);
        return;
    }

    std::size_t pos = 0;
    int cursor = x;
    while (pos < line.size()) {
        if (line[pos] == ' ' || line[pos] == '\t') {
            const std::size_t start = pos;
            while (pos < line.size() && (line[pos] == ' ' || line[pos] == '\t')) {
                ++pos;
            }
            canvas.draw_text({cursor, y}, std::string(line.substr(start, pos - start)), row_style);
            cursor += static_cast<int>(pos - start);
            continue;
        }

        if (line[pos] == '"' || line[pos] == '\'') {
            const char quote = line[pos];
            const std::size_t start = pos;
            ++pos;
            while (pos < line.size() && line[pos] != quote) {
                ++pos;
            }
            if (pos < line.size()) {
                ++pos;
            }
            auto style = row_style;
            style.foreground_rgb = palette.green;
            canvas.draw_text({cursor, y}, std::string(line.substr(start, pos - start)), style);
            cursor += static_cast<int>(pos - start);
            continue;
        }

        if (line[pos] == 'f' && line.substr(pos, 5) == "false") {
            auto style = row_style;
            style.foreground_rgb = palette.orange;
            canvas.draw_text({cursor, y}, "false", style);
            cursor += 5;
            pos += 5;
            continue;
        }

        if (line[pos] == 't' && line.substr(pos, 4) == "true") {
            auto style = row_style;
            style.foreground_rgb = palette.orange;
            canvas.draw_text({cursor, y}, "true", style);
            cursor += 4;
            pos += 4;
            continue;
        }

        const std::size_t start = pos;
        while (pos < line.size()
               && (std::isalnum(static_cast<unsigned char>(line[pos])) || line[pos] == '_' || line[pos] == '.')) {
            ++pos;
        }

        if (start == pos) {
            const char ch = line[pos++];
            canvas.draw_text({cursor, y}, std::string(1, ch), row_style);
            ++cursor;
            continue;
        }

        const std::string_view token = line.substr(start, pos - start);
        auto style = row_style;
        if (token == "api" || (token.size() > 4 && token.substr(0, 4) == "api.")) {
            style.foreground_rgb = palette.cyan;
        } else if (token == "self" || token == "conf" || token == "log_opts") {
            style.foreground_rgb = palette.cyan;
        } else if (is_lua_keyword(token)) {
            style.foreground_rgb = palette.purple;
        } else if (token.size() > 2 && token[0] == 'F' && token.find("Panel") != std::string_view::npos) {
            style.foreground_rgb = palette.green;
        }
        canvas.draw_text({cursor, y}, std::string(token), style);
        cursor += static_cast<int>(token.size());
    }
}

void paint_hatch_row(tuinator::Canvas& canvas, int x, int y, int width, const NvPalette& palette) {
    const std::string fill = tuinator::line_icon_glyph(tuinator::LineIcon::DiffFill);
    const int fill_width = std::max(1, tuinator::text_display_width(fill));
    auto style = tuinator::style_fg_bg(palette.border, palette.bg);

    for (int col = x; col < x + width; col += fill_width) {
        canvas.draw_text({col, y}, fill, style);
    }
}

void paint_diff_cell(
    tuinator::Canvas& canvas,
    int x,
    int y,
    int width,
    const std::optional<DiffRow>& cell,
    bool is_right,
    int cursor_row,
    int row_index,
    const NvPalette& palette) {
    const int gutter_width = 6;
    const int text_x = x + gutter_width + 1;

    if (!cell.has_value()) {
        paint_hatch_row(canvas, x, y, width, palette);
        return;
    }

    const DiffRow& line = *cell;
    tuinator::Style row_style = palette.bg_style();
    if (line.hunk) {
        row_style = is_right ? palette.add_row() : palette.del_row();
    }
    if (row_index == cursor_row) {
        row_style = palette.cursor_row();
    }

    canvas.fill_rect({{x, y}, {width, 1}}, ' ', row_style);

    if (line.hunk) {
        const tuinator::Rgb bar_color = is_right ? palette.hunk_bar_add : palette.hunk_bar_del;
        auto bar_style = row_style;
        bar_style.foreground_rgb = bar_color;
        bar_style.bold = true;
        canvas.draw_text({x, y}, tuinator::line_icon_glyph(tuinator::LineIcon::DiffSign), bar_style);
    }

    if (line.line_no.has_value()) {
        const std::string number = std::to_string(*line.line_no);
        auto num_style = row_style;
        num_style.foreground_rgb = palette.muted;
        const int num_x = x + std::max(0, gutter_width - static_cast<int>(number.size()));
        canvas.draw_text({num_x, y}, number, num_style);
    }

    paint_lua_line(canvas, text_x, y, line.text, palette, row_style);
}

class NvTabBar : public tuinator::Widget {
public:
    explicit NvTabBar(std::vector<std::string> tabs, int active = 0)
        : tabs_(std::move(tabs)), active_(active) {}

    tuinator::Size preferred_size() const override { return {0, 1}; }

    void paint(tuinator::PaintContext& ctx) const override {
        tuinator::Canvas& canvas = ctx.canvas;
        canvas.fill_rect({{0, 0}, bounds_.size()}, ' ', palette_.bg_style());

        int x = 0;
        for (int i = 0; i < static_cast<int>(tabs_.size()); ++i) {
            const bool active = i == active_;
            const std::string& tab_name = tabs_[static_cast<std::size_t>(i)];
            const tuinator::FileIcon icon = tuinator::file_icon_for_path(tab_name);
            const std::string icon_glyph = tuinator::file_icon_glyph(icon);
            const int icon_width = std::max(1, tuinator::text_display_width(icon_glyph));

            const tuinator::Rgb tab_bg = active ? palette_.tab_active : palette_.bg;
            auto style = active
                ? tuinator::style_fg_bg(palette_.text, tab_bg)
                : tuinator::style_fg_bg(palette_.muted, tab_bg);
            if (active) {
                style.bold = true;
            }

            const int accent_width = active ? 1 : 0;
            const std::string label = " " + truncate_middle(tab_name, 14) + " ";
            const std::string close_glyph = tuinator::ui_icon_glyph(tuinator::UiIcon::Close);
            const int close_width = std::max(1, tuinator::text_display_width(close_glyph)) + 1;
            const int tab_width = accent_width + 1 + icon_width + static_cast<int>(label.size()) + close_width;
            if (x + tab_width > bounds_.width) {
                break;
            }

            if (active) {
                auto accent_style = tuinator::style_fg_bg(palette_.status_accent, palette_.status_accent);
                canvas.fill_rect({{x, 0}, {accent_width, 1}}, ' ', accent_style);
                canvas.fill_rect({{x + accent_width, 0}, {tab_width - accent_width, 1}}, ' ', style);
            }

            int cursor = x + accent_width + 1;
            canvas.draw_text(
                {cursor, 0},
                icon_glyph,
                file_icon_row_style(palette_, icon, tab_bg));
            cursor += icon_width;
            canvas.draw_text({cursor, 0}, label, style);
            cursor += static_cast<int>(label.size());

            auto close_style = tuinator::style_fg_bg(tuinator::ui_icon_color(tuinator::UiIcon::Close), tab_bg);
            close_style.dim = true;
            canvas.draw_text({cursor, 0}, " " + close_glyph, close_style);

            x += tab_width + 1;
        }

        auto pane_style = palette_.muted_style();
        pane_style.dim = true;
        canvas.draw_text({std::max(0, bounds_.width - 4), 0}, "1  2", pane_style);
    }

private:
    NvPalette palette_;
    std::vector<std::string> tabs_;
    int active_ = 0;
};

class NvDiffSplit : public tuinator::Widget {
public:
    explicit NvDiffSplit(std::vector<SplitDiffRow> rows, int cursor_row = 1)
        : rows_(std::move(rows)), cursor_row_(cursor_row) {}

    tuinator::Size preferred_size() const override {
        return {80, std::max(1, static_cast<int>(rows_.size()))};
    }

    void paint(tuinator::PaintContext& ctx) const override {
        tuinator::Canvas& canvas = ctx.canvas;
        const int width = bounds_.width;
        const int height = bounds_.height;
        if (width <= 0 || height <= 0) {
            return;
        }

        canvas.fill_rect({{0, 0}, {width, height}}, ' ', palette_.bg_style());

        const int divider_x = width / 2;
        const int left_width = divider_x;
        const int right_width = std::max(0, width - divider_x - 1);
        const int right_x = divider_x + 1;

        const int row_count = std::min(height, static_cast<int>(rows_.size()));
        for (int row = 0; row < row_count; ++row) {
            const SplitDiffRow& aligned = rows_[static_cast<std::size_t>(row)];
            const bool filler_row = !aligned.left.has_value() && !aligned.right.has_value();

            paint_diff_cell(canvas, 0, row, left_width, aligned.left, false, cursor_row_, row, palette_);
            paint_diff_cell(canvas, right_x, row, right_width, aligned.right, true, cursor_row_, row, palette_);

            auto divider_style = palette_.border_style();
            divider_style.dim = true;
            if (filler_row) {
                divider_style.foreground_rgb = palette_.muted;
            }
            const char divider_ch = filler_row ? ':' : '|';
            canvas.draw_char({divider_x, row}, divider_ch, divider_style);
        }
    }

private:
    NvPalette palette_;
    std::vector<SplitDiffRow> rows_;
    int cursor_row_ = 0;
};

class DiffviewRoot : public tuinator::VBox {
public:
    explicit DiffviewRoot(tuinator::BoxOptions options) : tuinator::VBox(options) {}
    bool wants_full_screen() const override { return true; }
};

std::vector<SplitDiffRow> make_aligned_diff_rows() {
    return {
        {code_row(5, "local M = {}"), code_row(16, "local api = vim.api")},
        {code_row(80, "function FileHistoryPanel:get_sorted_entry_ids(log_opts)", true), std::nullopt},
        {code_row(81, "  local entries = self.state.entries", true), std::nullopt},
        {code_row(82, "  local sorted = {}", true), std::nullopt},
        {code_row(83, "  for _, entry in ipairs(entries) do", true), std::nullopt},
        {code_row(84, "    table.insert(sorted, entry.id)", true), std::nullopt},
        {code_row(85, "  end", true), std::nullopt},
        {code_row(86, "  api.nvim_buf_set_lines(self.entry_buf, 0, -1, false, sorted)", true), std::nullopt},
        {code_row(87, "  return sorted", true), std::nullopt},
        {code_row(88, "end", true), std::nullopt},
        {std::nullopt, std::nullopt},
        {std::nullopt, std::nullopt},
        {std::nullopt, std::nullopt},
        {std::nullopt, std::nullopt},
        {std::nullopt, std::nullopt},
        {std::nullopt, std::nullopt},
        {std::nullopt, code_row(87, "function FileHistoryPanel:get_sorted_entry_ids(conf)", true)},
        {std::nullopt, code_row(88, "  api.nvim_buf_delete(self.entry_buf, { force = true })", true)},
        {std::nullopt, code_row(89, "  self.state.entry_buf = nil", true)},
        {std::nullopt, code_row(90, "end", true)},
        {std::nullopt, code_row(91, "")},
        {code_row(120, "function FileHistoryPanel:invalidate()"), code_row(92, "function FileHistoryPanel:invalidate()")},
        {code_row(121, "  self.state.dirty = true"), code_row(93, "  self.state.dirty = true")},
        {code_row(122, "end"), code_row(94, "end")},
    };
}

} // namespace

int main() {
    tuinator::Application app;

    NvPalette palette;
    tuinator::Theme theme = tuinator::dark_theme();
    theme.heading = palette.sidebar_style();
    theme.label = palette.bg_style();
    theme.muted = palette.muted_style();
    theme.border = palette.border_style();
    theme.button = palette.bg_style();
    theme.text_input_focused = palette.sidebar_style();
    app.set_theme(theme);

    auto root = std::make_unique<DiffviewRoot>(tuinator::BoxOptions{.gap = 0, .padding = 0});

    auto body = std::make_unique<tuinator::HBox>(tuinator::BoxOptions{.gap = 0, .padding = 0});

    auto sidebar = std::make_unique<tuinator::SourceControlPanel>(make_source_control_style(palette));
    sidebar->set_header("Source Control", "../Documents/git/diffview.nvim");
    sidebar->set_sections(make_source_sections());
    sidebar->set_flex(0);

    auto main_column = std::make_unique<tuinator::VBox>(tuinator::BoxOptions{.gap = 0, .padding = 0});
    main_column->set_flex(1);

    auto tabs = std::make_unique<NvTabBar>(
        std::vector<std::string>{
            "file_history_panel.lua",
            "diffview.lua",
            "file_history_view.lua",
            "listeners.lua",
            "diffview.vim",
        },
        0);

    auto diff_split = std::make_unique<NvDiffSplit>(make_aligned_diff_rows(), 1);
    diff_split->set_flex(1);

    main_column->add_child(std::move(tabs));
    main_column->add_child(std::move(diff_split));

    body->add_child(std::move(sidebar));
    body->add_child(std::move(main_column));
    body->set_flex(1);

    root->add_child(std::move(body));
    root->add_child(make_diffview_status_line(palette));

    app.set_root(std::move(root));
    return app.run();
}
