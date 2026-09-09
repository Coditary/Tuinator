#include <tuinator/render/box_drawing.hpp>
#include <tuinator/render/glyphs.hpp>
#include <tuinator/widgets/capabilities.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/containers/panel.hpp>
#include <tuinator/widgets/containers/split_pane.hpp>

#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <variant>

namespace tuinator {

namespace {

bool panel_touches_edge(const Panel& panel, const Rect& region, BorderEdge edge) {
    const Rect bounds = panel.bounds();
    switch (edge) {
    case BorderEdge::Top: return bounds.y == region.y;
    case BorderEdge::Right: return bounds.right() == region.right();
    case BorderEdge::Bottom: return bounds.bottom() == region.bottom();
    case BorderEdge::Left: return bounds.x == region.x;
    }
    return false;
}

void disable_panel_border_on_edge(Widget* root, const Rect& region, BorderEdge edge) {
    if (root == nullptr) {
        return;
    }

    walk_subtree(root, [&](Widget* widget) {
        auto* panel = dynamic_cast<Panel*>(widget);
        if (panel == nullptr || !panel_touches_edge(*panel, region, edge)) {
            return true;
        }

        BorderEdges edges = panel->border_edges();
        switch (edge) {
        case BorderEdge::Top: edges.top = false; break;
        case BorderEdge::Right: edges.right = false; break;
        case BorderEdge::Bottom: edges.bottom = false; break;
        case BorderEdge::Left: edges.left = false; break;
        }
        panel->set_border_edges(edges);
        return true;
    });
}

BoxDrawing junction_glyph(bool up, bool down, bool left, bool right) {
    if (up && down && left && right) {
        return BoxDrawing::Cross;
    }
    if (up && down && right && !left) {
        return BoxDrawing::VerticalAndRight;
    }
    if (up && down && left && !right) {
        return BoxDrawing::VerticalAndLeft;
    }
    if (down && left && right && !up) {
        return BoxDrawing::DownAndHorizontal;
    }
    if (up && left && right && !down) {
        return BoxDrawing::UpAndHorizontal;
    }
    if (up && down) {
        return BoxDrawing::Vertical;
    }
    if (left && right) {
        return BoxDrawing::Horizontal;
    }
    if (down && right) {
        return BoxDrawing::DownRight;
    }
    if (down && left) {
        return BoxDrawing::DownLeft;
    }
    if (up && right) {
        return BoxDrawing::UpRight;
    }
    if (up && left) {
        return BoxDrawing::UpLeft;
    }
    if (down || up) {
        return BoxDrawing::Vertical;
    }
    return BoxDrawing::Horizontal;
}

void connections_at(int x, int y, const std::vector<SplitDividerLine>& lines, bool& up, bool& down, bool& left,
                    bool& right) {
    up = false;
    down = false;
    left = false;
    right = false;

    for (const SplitDividerLine& line : lines) {
        if (line.vertical && line.fixed == x && y >= line.start && y <= line.end) {
            up |= y > line.start;
            down |= y < line.end;
        }
        if (!line.vertical && line.fixed == y && x >= line.start && x <= line.end) {
            left |= x > line.start;
            right |= x < line.end;
        }
    }
}

void merge_outer_frame_connections(int x, int y, const Rect& outer, bool& up, bool& down, bool& left, bool& right) {
    if (outer.width <= 0 || outer.height <= 0) {
        return;
    }

    const int left_x = outer.x;
    const int right_x = outer.right() - 1;
    const int top_y = outer.y;
    const int bottom_y = outer.bottom() - 1;

    if (x == left_x && y >= top_y && y <= bottom_y) {
        up |= y > top_y;
        down |= y < bottom_y;
    }
    if (x == right_x && y >= top_y && y <= bottom_y) {
        up |= y > top_y;
        down |= y < bottom_y;
    }
    if (y == top_y && x >= left_x && x <= right_x) {
        left |= x > left_x;
        right |= x < right_x;
    }
    if (y == bottom_y && x >= left_x && x <= right_x) {
        left |= x > left_x;
        right |= x < right_x;
    }
}

void add_perimeter_cells(const Rect& outer, std::set<std::pair<int, int>>& cells) {
    if (outer.width <= 0 || outer.height <= 0) {
        return;
    }

    const int left_x = outer.x;
    const int right_x = outer.right() - 1;
    const int top_y = outer.y;
    const int bottom_y = outer.bottom() - 1;

    for (int x = left_x; x <= right_x; ++x) {
        cells.insert({x, top_y});
        cells.insert({x, bottom_y});
    }
    for (int y = top_y; y <= bottom_y; ++y) {
        cells.insert({left_x, y});
        cells.insert({right_x, y});
    }
}

GlyphSet glyph_set_for_theme(const Theme& theme) {
    const BorderGlyphs ascii = ascii_border_glyphs();
    if (theme.glyphs.horizontal == ascii.horizontal && theme.glyphs.vertical == ascii.vertical) {
        return GlyphSet::Ascii;
    }
    return GlyphSet::Unicode;
}

void reset_panel_border_edges(Widget* root) {
    if (root == nullptr) {
        return;
    }

    walk_subtree(root, [](Widget* widget) {
        if (auto* panel = dynamic_cast<Panel*>(widget)) {
            panel->reset_border_edges();
        }
        return true;
    });
}

void trim_split_child_borders(Widget* child, SplitOrientation orientation, bool is_first_child) {
    if (child == nullptr) {
        return;
    }

    reset_panel_border_edges(child);

    const Rect region = child->bounds();
    if (orientation == SplitOrientation::Horizontal) {
        disable_panel_border_on_edge(child, region, is_first_child ? BorderEdge::Right : BorderEdge::Left);
        return;
    }

    disable_panel_border_on_edge(child, region, is_first_child ? BorderEdge::Bottom : BorderEdge::Top);
}

} // namespace

SplitPane::SplitPane(std::unique_ptr<Widget> first, std::unique_ptr<Widget> second, SplitPaneOptions options)
    : first_(std::move(first)), second_(std::move(second)), options_(options) {
    attach_child_widget(first_.get());
    attach_child_widget(second_.get());
}

void SplitPane::set_options(SplitPaneOptions options) {
    options_ = options;
    mark_layout_dirty();
}

void SplitPane::apply_stylesheet(const StyleResolver& styles) {
    apply_splittable_stylesheet(*this, *this, styles);
}

void SplitPane::set_on_dirty(std::function<void(Rect)> callback) {
    Widget::set_on_dirty(std::move(callback));
    if (first_) {
        first_->set_on_dirty(on_dirty_);
    }
    if (second_) {
        second_->set_on_dirty(on_dirty_);
    }
}

void SplitPane::set_on_layout(std::function<void()> callback) {
    Widget::set_on_layout(std::move(callback));
    if (first_) {
        first_->set_on_layout(on_layout_);
    }
    if (second_) {
        second_->set_on_layout(on_layout_);
    }
}

Size SplitPane::preferred_size() const {
    const Size first_size = first_ ? first_->preferred_size() : Size{};
    const Size second_size = second_ ? second_->preferred_size() : Size{};

    if (options_.orientation == SplitOrientation::Horizontal) {
        return {
            first_size.width + 1 + second_size.width,
            std::max(first_size.height, second_size.height),
        };
    }

    return {
        std::max(first_size.width, second_size.width),
        first_size.height + 1 + second_size.height,
    };
}

int SplitPane::divider_position() const {
    if (options_.orientation == SplitOrientation::Horizontal) {
        return first_ ? first_->bounds().width : options_.first_size;
    }

    return first_ ? first_->bounds().height : options_.first_size;
}

Rect SplitPane::divider_bounds() const {
    const int position = divider_position();

    if (options_.orientation == SplitOrientation::Horizontal) {
        return {bounds_.x + position, bounds_.y, 1, bounds_.height};
    }

    return {bounds_.x, bounds_.y + position, bounds_.width, 1};
}

Rect SplitPane::divider_hit_bounds() const {
    const int slop = std::max(0, options_.divider_hit_slop);
    Rect divider = divider_bounds();

    if (options_.orientation == SplitOrientation::Horizontal) {
        divider.x -= slop;
        divider.width += slop * 2 + 1;
        return divider;
    }

    divider.y -= slop;
    divider.height += slop * 2 + 1;
    return divider;
}

bool SplitPane::contains_divider(Point point) const { return divider_hit_bounds().contains(point); }

void SplitPane::update_first_size_from_mouse(Point global_position) {
    const Point local{global_position.x - bounds_.x, global_position.y - bounds_.y};
    const int min_size = std::max(1, options_.min_pane_size);
    const int total = options_.orientation == SplitOrientation::Horizontal ? bounds_.width : bounds_.height;
    const int max_first = std::max(min_size, total - min_size - 1);

    int new_size = options_.orientation == SplitOrientation::Horizontal ? local.x : local.y;
    new_size = std::clamp(new_size, min_size, max_first);

    if (new_size == options_.first_size) {
        return;
    }

    options_.first_size = new_size;
    mark_layout_dirty();
}

void SplitPane::begin_drag() {
    if (dragging_) {
        return;
    }

    dragging_ = true;
}

void SplitPane::end_drag() {
    dragging_ = false;
}

void SplitPane::layout(Rect bounds) {
    bounds_ = bounds;

    if (options_.orientation == SplitOrientation::Horizontal) {
        const int divider = 1;
        const int first_width = std::clamp(options_.first_size, 0, std::max(0, bounds.width - divider));
        const int second_width = std::max(0, bounds.width - first_width - divider);

        if (first_) {
            first_->layout({bounds.x, bounds.y, first_width, bounds.height});
        }
        if (second_) {
            second_->layout({bounds.x + first_width + divider, bounds.y, second_width, bounds.height});
        }

        trim_split_child_borders(first_.get(), options_.orientation, true);
        trim_split_child_borders(second_.get(), options_.orientation, false);
        return;
    }

    const int divider = 1;
    const int first_height = std::clamp(options_.first_size, 0, std::max(0, bounds.height - divider));
    const int second_height = std::max(0, bounds.height - first_height - divider);

    if (first_) {
        first_->layout({bounds.x, bounds.y, bounds.width, first_height});
    }
    if (second_) {
        second_->layout({bounds.x, bounds.y + first_height + divider, bounds.width, second_height});
    }

    trim_split_child_borders(first_.get(), options_.orientation, true);
    trim_split_child_borders(second_.get(), options_.orientation, false);
}

void SplitPane::append_divider_line(std::vector<SplitDividerLine>& lines) const {
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    if (options_.orientation == SplitOrientation::Horizontal) {
        const int x = bounds_.x + divider_position();
        lines.push_back(SplitDividerLine{true, x, bounds_.y, bounds_.y + bounds_.height - 1});
        return;
    }

    const int y = bounds_.y + divider_position();
    lines.push_back(SplitDividerLine{false, y, bounds_.x, bounds_.x + bounds_.width - 1});
}

void SplitPane::paint(PaintContext& ctx) const {
    auto paint_child = [&](const Widget* child) {
        if (!child) {
            return;
        }

        const Rect local{
            child->bounds().x - bounds_.x,
            child->bounds().y - bounds_.y,
            child->bounds().width,
            child->bounds().height,
        };

        ctx.with_clip(local, [&](PaintContext& child_ctx) { child->paint(child_ctx); });
    };

    paint_child(first_.get());
    paint_child(second_.get());
}

void join_split_divider_lines(std::vector<SplitDividerLine>& lines) {
    for (SplitDividerLine& horizontal : lines) {
        if (horizontal.vertical) {
            continue;
        }

        for (const SplitDividerLine& vertical : lines) {
            if (!vertical.vertical) {
                continue;
            }

            if (vertical.fixed == horizontal.start - 1 && horizontal.fixed >= vertical.start &&
                horizontal.fixed <= vertical.end) {
                horizontal.start = vertical.fixed;
            }

            if (vertical.fixed == horizontal.end + 1 && horizontal.fixed >= vertical.start &&
                horizontal.fixed <= vertical.end) {
                horizontal.end = vertical.fixed;
            }
        }
    }

    for (SplitDividerLine& vertical : lines) {
        if (!vertical.vertical) {
            continue;
        }

        for (const SplitDividerLine& horizontal : lines) {
            if (horizontal.vertical) {
                continue;
            }

            if (horizontal.fixed == vertical.end + 1 && vertical.fixed >= horizontal.start &&
                vertical.fixed <= horizontal.end) {
                vertical.end = horizontal.fixed;
            }

            if (horizontal.fixed == vertical.start - 1 && vertical.fixed >= horizontal.start &&
                vertical.fixed <= horizontal.end) {
                vertical.start = horizontal.fixed;
            }
        }
    }
}

bool line_contains(const SplitDividerLine& line, int x, int y) {
    if (line.vertical) {
        return line.fixed == x && y >= line.start && y <= line.end;
    }
    return line.fixed == y && x >= line.start && x <= line.end;
}

void paint_split_dividers(Widget& root, PaintContext& ctx) {
    std::vector<SplitDividerLine> lines;
    std::vector<SplitPane*> line_owners;
    Rect outer_bounds{};
    bool draw_outer_border = false;

    root.for_each_descendant([&](Widget* widget) {
        if (auto* split = dynamic_cast<SplitPane*>(widget)) {
            split->append_divider_line(lines);
            line_owners.push_back(split);
            outer_bounds = unite(outer_bounds, split->bounds());
            draw_outer_border |= split->wants_outer_border();
        }
    });

    if (lines.empty()) {
        return;
    }

    join_split_divider_lines(lines);

    std::set<std::pair<int, int>> cells;
    for (const SplitDividerLine& line : lines) {
        if (line.vertical) {
            for (int y = line.start; y <= line.end; ++y) {
                cells.insert({line.fixed, y});
            }
            continue;
        }

        for (int x = line.start; x <= line.end; ++x) {
            cells.insert({x, line.fixed});
        }
    }

    if (draw_outer_border) {
        add_perimeter_cells(outer_bounds, cells);
    }

    const Point origin = ctx.canvas.origin();

    for (const auto& [x, y] : cells) {
        bool up = false;
        bool down = false;
        bool left = false;
        bool right = false;
        connections_at(x, y, lines, up, down, left, right);
        if (draw_outer_border) {
            merge_outer_frame_connections(x, y, outer_bounds, up, down, left, right);
        }

        SplitPane* owner = nullptr;
        for (std::size_t index = 0; index < lines.size(); ++index) {
            if (line_contains(lines[index], x, y)) {
                owner = line_owners[index];
            }
        }

        Style divider_style = owner != nullptr ? owner->options().divider_style : Style{};
        if (divider_style.foreground == Color::Default && !divider_style.foreground_rgb) {
            divider_style.foreground = Color::White;
            divider_style.dim = true;
        }
        if (owner != nullptr) {
            divider_style = ctx.styles().divider(*owner, divider_style);
        }

        GlyphSet glyph_set = owner != nullptr ? ctx.styles().glyph_set(*owner) : glyph_set_for_theme(ctx.theme);
        if (glyph_set == GlyphSet::Auto) {
            glyph_set = detect_glyph_set();
        }

        const BoxDrawing kind = junction_glyph(up, down, left, right);
        const std::string glyph = box_drawing_glyph(kind, glyph_set);
        ctx.canvas.draw_text({x - origin.x, y - origin.y}, glyph, divider_style);
    }
}

Widget* SplitPane::hit_test(Point point) {
    if (!bounds_.contains(point)) {
        return nullptr;
    }

    if (contains_divider(point)) {
        return this;
    }

    for (Widget* pane : {second_.get(), first_.get()}) {
        if (pane == nullptr) {
            continue;
        }

        if (Widget* hit = pane->hit_test(point)) {
            return hit;
        }
    }

    return this;
}

bool SplitPane::handle_event(const Event& event) {
    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (dragging_) {
            if (mouse->action == MouseAction::Release) {
                end_drag();
                return true;
            }

            if (mouse->action == MouseAction::Move && mouse->left_pressed) {
                update_first_size_from_mouse(mouse->position);
                return true;
            }
        }

        const bool drag_motion = mouse->action == MouseAction::Move && mouse->left_pressed;
        if (mouse->action == MouseAction::Press || mouse->action == MouseAction::Click || drag_motion) {
            if (contains_divider(mouse->position)) {
                begin_drag();
                if (mouse->action != MouseAction::Click) {
                    update_first_size_from_mouse(mouse->position);
                }
                return true;
            }
        }

        if (mouse->action == MouseAction::Release && dragging_) {
            end_drag();
            return true;
        }
    }

    for (Widget* pane : {first_.get(), second_.get()}) {
        if (pane != nullptr && pane->has_focused_descendant() && pane->handle_event(event)) {
            return true;
        }
    }

    if (const auto* mouse = std::get_if<MouseEvent>(&event)) {
        if (!bounds_.contains(mouse->position)) {
            return false;
        }

        for (Widget* pane : {first_.get(), second_.get()}) {
            if (pane == nullptr) {
                continue;
            }

            if (Widget* hit = pane->hit_test(mouse->position)) {
                return hit->handle_event(event);
            }
        }
    }

    return false;
}

bool SplitPane::pointer_active() const { return dragging_; }

bool SplitPane::has_focused_descendant() const {
    return (first_ && first_->has_focused_descendant()) || (second_ && second_->has_focused_descendant());
}

void SplitPane::collect_focusable(std::vector<Widget*>& out) {
    if (first_) {
        first_->collect_focusable(out);
    }
    if (second_) {
        second_->collect_focusable(out);
    }
}

void SplitPane::for_each_child(const std::function<void(Widget*)>& visitor) {
    if (first_) {
        visitor(first_.get());
    }
    if (second_) {
        visitor(second_.get());
    }
}

void SplitPane::for_each_descendant(const std::function<void(Widget*)>& visitor) {
    visitor(this);
    for_each_child([&](Widget* child) {
        if (child != nullptr) {
            child->for_each_descendant(visitor);
        }
    });
}

} // namespace tuinator
