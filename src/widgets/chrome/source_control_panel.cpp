#include <tuinator/widgets/chrome/source_control_panel.hpp>

#include <tuinator/core/event.hpp>
#include <tuinator/render/file_icon.hpp>
#include <tuinator/render/git_change_status.hpp>
#include <tuinator/render/text.hpp>

#include <algorithm>
#include <string>
#include <variant>

namespace tuinator {

namespace {

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

Style with_optional_bg(const Style& base, const Rgb& bg) {
    Style style = base;
    style.background_rgb = bg;
    return style;
}

Style with_optional_fg(const Style& base, const Rgb& fg) {
    Style style = base;
    style.foreground_rgb = fg;
    return style;
}

} // namespace

SourceControlPanel::SourceControlPanel(SourceControlPanelStyle style)
    : style_(std::move(style)) {}

void SourceControlPanel::set_header(std::string title, std::string subtitle) {
    header_title_ = std::move(title);
    header_subtitle_ = std::move(subtitle);
    mark_dirty();
}

void SourceControlPanel::set_sections(std::vector<SourceControlSection> sections) {
    sections_ = std::move(sections);
    selected_section_ = std::clamp(selected_section_, 0, std::max(0, static_cast<int>(sections_.size()) - 1));
    if (!sections_.empty()) {
        selected_entry_ = std::clamp(
            selected_entry_,
            0,
            std::max(0, static_cast<int>(sections_[static_cast<std::size_t>(selected_section_)].entries.size()) - 1));
    } else {
        selected_entry_ = 0;
    }
    clamp_scroll();
    mark_dirty();
}

void SourceControlPanel::set_selected(int section_index, int entry_index) {
    if (sections_.empty()) {
        return;
    }

    for (SourceControlSection& section : sections_) {
        for (SourceControlEntry& entry : section.entries) {
            entry.selected = false;
        }
    }

    selected_section_ = std::clamp(section_index, 0, static_cast<int>(sections_.size()) - 1);
    SourceControlSection& section = sections_[static_cast<std::size_t>(selected_section_)];
    if (section.entries.empty()) {
        selected_entry_ = 0;
    } else {
        selected_entry_ = std::clamp(entry_index, 0, static_cast<int>(section.entries.size()) - 1);
        section.entries[static_cast<std::size_t>(selected_entry_)].selected = true;
    }

    ensure_selection_visible();
    mark_dirty();
}

void SourceControlPanel::set_on_select(
    std::function<void(int section_index, int entry_index, const SourceControlEntry&)> callback) {
    on_select_ = std::move(callback);
}

Size SourceControlPanel::preferred_size() const {
    return {style_.preferred_width, std::max(1, content_height())};
}

void SourceControlPanel::layout(Rect bounds) {
    bounds_ = bounds;
    clamp_scroll();
    ensure_selection_visible();
}

int SourceControlPanel::content_height() const {
    int height = 3;
    for (const SourceControlSection& section : sections_) {
        ++height;
        if (section.expanded) {
            height += static_cast<int>(section.entries.size());
        }
    }
    if (style_.show_footer) {
        ++height;
    }
    return height;
}

void SourceControlPanel::clamp_scroll() {
    const int viewport = std::max(1, bounds_.height);
    const int max_scroll = std::max(0, content_height() - viewport);
    scroll_y_ = std::clamp(scroll_y_, 0, max_scroll);
}

void SourceControlPanel::ensure_selection_visible() {
    if (sections_.empty() || bounds_.height <= 0) {
        return;
    }

    int y = 3;
    int target_y = 0;
    bool found = false;

    for (int section_index = 0; section_index < static_cast<int>(sections_.size()); ++section_index) {
        const SourceControlSection& section = sections_[static_cast<std::size_t>(section_index)];
        if (section_index == selected_section_ && !section.expanded) {
            target_y = y;
            found = true;
            break;
        }

        if (section_index == selected_section_) {
            target_y = y + 1 + selected_entry_;
            found = true;
            break;
        }

        ++y;
        if (section.expanded) {
            y += static_cast<int>(section.entries.size());
        }
    }

    if (!found) {
        return;
    }

    if (target_y < scroll_y_) {
        scroll_y_ = target_y;
    } else if (target_y >= scroll_y_ + bounds_.height) {
        scroll_y_ = target_y - bounds_.height + 1;
    }

    clamp_scroll();
}

SourceControlPanel::HitTarget SourceControlPanel::hit_target_at(Point local) const {
    HitTarget target;
    if (local.y < 0) {
        return target;
    }

    const int row = scroll_y_ + local.y;
    int y = 0;

    if (row == 0 || row == 1) {
        return target;
    }

    y = 3;
    if (row < y) {
        return target;
    }

    for (int section_index = 0; section_index < static_cast<int>(sections_.size()); ++section_index) {
        if (row == y) {
            target.section = section_index;
            target.is_section_header = true;
            return target;
        }
        ++y;

        const SourceControlSection& section = sections_[static_cast<std::size_t>(section_index)];
        if (!section.expanded) {
            continue;
        }

        for (int entry_index = 0; entry_index < static_cast<int>(section.entries.size()); ++entry_index) {
            if (row == y) {
                target.section = section_index;
                target.entry = entry_index;
                return target;
            }
            ++y;
        }
    }

    return target;
}

void SourceControlPanel::select_entry(int section_index, int entry_index) {
    if (section_index < 0 || section_index >= static_cast<int>(sections_.size())) {
        return;
    }

    const SourceControlSection& section = sections_[static_cast<std::size_t>(section_index)];
    if (entry_index < 0 || entry_index >= static_cast<int>(section.entries.size())) {
        return;
    }

    set_selected(section_index, entry_index);

    if (on_select_) {
        on_select_(section_index, entry_index, section.entries[static_cast<std::size_t>(entry_index)]);
    }
}

void SourceControlPanel::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    const int width = bounds_.width;
    const int height = bounds_.height;
    if (width <= 0 || height <= 0) {
        return;
    }

    canvas.fill_rect({{0, 0}, {width, height}}, ' ', style_.background);

    const int footer_rows = style_.show_footer ? 1 : 0;
    const int content_rows = std::max(0, height - footer_rows);

    int y = -scroll_y_;
    auto draw_row = [&](int row, const auto& draw_fn) {
        if (row >= 0 && row < content_rows) {
            draw_fn(row);
        }
    };

    draw_row(y, [&](int row) { canvas.draw_text({1, row}, header_title_, style_.header); });
    ++y;
    draw_row(y, [&](int row) { canvas.draw_text({1, row}, header_subtitle_, style_.muted); });
    y += 2;

    for (const SourceControlSection& section : sections_) {
        draw_row(y, [&](int row) { canvas.draw_text({1, row}, section.title, style_.section_title); });
        ++y;

        if (!section.expanded) {
            continue;
        }

        for (const SourceControlEntry& entry : section.entries) {
            draw_row(y, [&](int row) {
                const Rgb row_bg = entry.selected ? style_.selection_bg : style_.panel_bg;
                Style row_style = with_optional_fg(with_optional_bg(style_.entry_text, row_bg), style_.text_fg);

                if (entry.selected) {
                    canvas.fill_rect({{0, row}, {width, 1}}, ' ', row_style);
                }

                GitChangeStatusColorDefaults status_colors;
                status_colors.modified = style_.status_modified;
                status_colors.added = style_.status_added;
                status_colors.deleted = style_.status_deleted;
                status_colors.text = style_.text_fg;
                status_colors.muted = style_.muted_fg;
                const Rgb status_color = git_change_status_color(entry.status, status_colors);
                const std::string status_text(1, git_change_status_char(entry.status));
                canvas.draw_text(
                    {1, row},
                    status_text,
                    with_optional_fg(with_optional_bg(style_.entry_text, row_bg), status_color));

                const FileIcon icon = file_icon_for_path(entry.path);
                const std::string icon_glyph = file_icon_glyph(icon);
                const int icon_width = std::max(1, text_display_width(icon_glyph));
                Style icon_style = with_optional_bg(style_.entry_text, row_bg);
                icon_style.foreground_rgb = file_icon_color(icon);
                canvas.draw_text({3, row}, icon_glyph, icon_style);

                const int name_x = 3 + icon_width + 1;

                const bool has_adds = !entry.additions.empty();
                const bool has_dels = !entry.deletions.empty();
                const std::string adds_text = has_adds ? "+" + entry.additions : "";
                const std::string dels_text = has_dels ? "-" + entry.deletions : "";

                int stats_inline_width = 0;
                if (has_adds) {
                    stats_inline_width += 1 + static_cast<int>(adds_text.size());
                }
                if (has_dels) {
                    stats_inline_width += 1 + static_cast<int>(dels_text.size());
                }

                std::string tag_display;
                int tag_x = width;
                if (!entry.directory_tag.empty() && style_.tag_max_columns > 0) {
                    tag_display =
                        truncate_middle(entry.directory_tag, static_cast<std::size_t>(style_.tag_max_columns));
                    tag_x = width - static_cast<int>(tag_display.size()) - 1;
                }

                int name_right = tag_display.empty() ? width - 1 : tag_x - 2;
                name_right -= stats_inline_width;
                int name_budget = std::max(0, name_right - name_x + 1);
                if (style_.name_max_columns > 0) {
                    name_budget = std::min(name_budget, style_.name_max_columns);
                }

                std::string display_name = entry.path;
                if (static_cast<int>(entry.path.size()) > name_budget) {
                    display_name = truncate_middle(entry.path, static_cast<std::size_t>(name_budget));
                }
                canvas.draw_text({name_x, row}, display_name, row_style);

                int cursor_x = name_x + static_cast<int>(display_name.size());
                if (has_adds) {
                    ++cursor_x;
                    auto add_style = with_optional_fg(with_optional_bg(style_.entry_text, row_bg), style_.status_added);
                    canvas.draw_text({cursor_x, row}, adds_text, add_style);
                    cursor_x += static_cast<int>(adds_text.size());
                }
                if (has_dels) {
                    ++cursor_x;
                    auto del_style =
                        with_optional_fg(with_optional_bg(style_.entry_text, row_bg), style_.status_deleted);
                    canvas.draw_text({cursor_x, row}, dels_text, del_style);
                }

                if (!tag_display.empty() && tag_x > cursor_x + 1) {
                    auto tag_style = with_optional_fg(with_optional_bg(style_.muted, row_bg), style_.muted_fg);
                    tag_style.dim = true;
                    canvas.draw_text({tag_x, row}, tag_display, tag_style);
                }
            });
            ++y;
        }
    }

    if (style_.show_footer && height > 0) {
        auto footer = style_.footer;
        footer.dim = true;
        canvas.draw_text({1, height - 1}, style_.footer_text, footer);
    }
}

bool SourceControlPanel::handle_event(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (mouse->action != MouseAction::Click && mouse->action != MouseAction::Release) {
            return false;
        }

        if (!contains_point(mouse->position)) {
            return false;
        }

        const Point local{
            mouse->position.x - bounds_.x,
            mouse->position.y - bounds_.y,
        };
        const HitTarget target = hit_target_at(local);
        if (target.section >= 0 && target.entry >= 0) {
            select_entry(target.section, target.entry);
            return true;
        }

        if (target.is_section_header) {
            sections_[static_cast<std::size_t>(target.section)].expanded =
                !sections_[static_cast<std::size_t>(target.section)].expanded;
            clamp_scroll();
            mark_dirty();
            return true;
        }

        return false;
    }

    const auto* key = std::get_if<KeyPress>(&event);
    if (!key || !is_focused() || sections_.empty()) {
        return false;
    }

    switch (key->key) {
    case Key::Up: {
        if (selected_entry_ > 0) {
            select_entry(selected_section_, selected_entry_ - 1);
        } else if (selected_section_ > 0) {
            const int prev = selected_section_ - 1;
            const int last = static_cast<int>(sections_[static_cast<std::size_t>(prev)].entries.size()) - 1;
            select_entry(prev, std::max(0, last));
        }
        return true;
    }
    case Key::Down: {
        const auto& section = sections_[static_cast<std::size_t>(selected_section_)];
        if (selected_entry_ + 1 < static_cast<int>(section.entries.size())) {
            select_entry(selected_section_, selected_entry_ + 1);
        } else if (selected_section_ + 1 < static_cast<int>(sections_.size())) {
            select_entry(selected_section_ + 1, 0);
        }
        return true;
    }
    default:
        break;
    }

    return false;
}

} // namespace tuinator
