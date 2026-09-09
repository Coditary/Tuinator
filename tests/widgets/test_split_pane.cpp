#include <tuinator/render/box_drawing.hpp>
#include <tuinator/widgets/containers/panel.hpp>
#include <tuinator/widgets/containers/split_pane.hpp>
#include <tuinator/widgets/display/label.hpp>

#include "test_harness.hpp"

namespace {

std::unique_ptr<tuinator::Widget> make_label(const std::string& text) {
    return std::make_unique<tuinator::Label>(text);
}

tuinator::Point horizontal_divider_point(const tuinator::SplitPane& pane, int first_size) {
    const tuinator::Rect bounds = pane.bounds();
    return {bounds.x + first_size, bounds.y + bounds.height / 2};
}

tuinator::Point vertical_divider_point(const tuinator::SplitPane& pane, int first_size) {
    const tuinator::Rect bounds = pane.bounds();
    return {bounds.x + bounds.width / 2, bounds.y + first_size};
}

} // namespace

TUINATOR_TEST(split_pane_nested_dividers_are_hit_testable) {
    auto inner = std::make_unique<tuinator::SplitPane>(
        make_label("left"), make_label("right"),
        tuinator::SplitPaneOptions{
            .orientation = tuinator::SplitOrientation::Horizontal,
            .first_size = 10,
        });
    auto* inner_ptr = inner.get();

    auto outer = std::make_unique<tuinator::SplitPane>(
        std::move(inner), make_label("bottom"),
        tuinator::SplitPaneOptions{
            .orientation = tuinator::SplitOrientation::Vertical,
            .first_size = 6,
        });
    auto* outer_ptr = outer.get();

    outer->layout({0, 0, 40, 20});

    const tuinator::Point inner_divider = horizontal_divider_point(*inner_ptr, inner_ptr->first_size());
    const tuinator::Point outer_divider = vertical_divider_point(*outer_ptr, outer_ptr->first_size());

    TUINATOR_CHECK(outer_ptr->hit_test(inner_divider) == inner_ptr);
    TUINATOR_CHECK(outer_ptr->hit_test(outer_divider) == outer_ptr);
}

TUINATOR_TEST(split_pane_propagates_layout_callback_to_nested_children) {
    int layout_passes = 0;

    auto inner = std::make_unique<tuinator::SplitPane>(
        make_label("left"), make_label("right"),
        tuinator::SplitPaneOptions{
            .orientation = tuinator::SplitOrientation::Horizontal,
            .first_size = 10,
        });

    auto outer = std::make_unique<tuinator::SplitPane>(
        std::move(inner), make_label("bottom"),
        tuinator::SplitPaneOptions{
            .orientation = tuinator::SplitOrientation::Vertical,
            .first_size = 6,
        });
    outer->set_on_layout([&layout_passes]() { ++layout_passes; });
    outer->layout({0, 0, 40, 20});

    auto* inner_ptr = dynamic_cast<tuinator::SplitPane*>(outer->first());
    TUINATOR_CHECK(inner_ptr != nullptr);

    inner_ptr->mark_layout_dirty();
    TUINATOR_CHECK_EQ(layout_passes, 1);
}

TUINATOR_TEST(split_pane_outer_border_option) {
    auto split = std::make_unique<tuinator::SplitPane>(
        make_label("left"), make_label("right"),
        tuinator::SplitPaneOptions{
            .orientation = tuinator::SplitOrientation::Horizontal,
            .first_size = 10,
            .outer_border = true,
        });

    TUINATOR_CHECK(split->wants_outer_border());
}

TUINATOR_TEST(split_divider_lines_join_at_adjacent_rows) {
    std::vector<tuinator::SplitDividerLine> lines{
        {true, 10, 0, 17},
        {false, 18, 5, 20},
    };

    tuinator::join_split_divider_lines(lines);

    TUINATOR_CHECK_EQ(lines[0].end, 18);
    TUINATOR_CHECK_EQ(lines[1].start, 5);
}

TUINATOR_TEST(split_pane_collects_nested_divider_lines) {
    auto inner = std::make_unique<tuinator::SplitPane>(
        make_label("left"), make_label("right"),
        tuinator::SplitPaneOptions{
            .orientation = tuinator::SplitOrientation::Horizontal,
            .first_size = 10,
        });

    auto outer = std::make_unique<tuinator::SplitPane>(
        std::move(inner), make_label("bottom"),
        tuinator::SplitPaneOptions{
            .orientation = tuinator::SplitOrientation::Vertical,
            .first_size = 6,
        });
    outer->layout({0, 0, 40, 20});

    std::vector<tuinator::SplitDividerLine> lines;
    outer->for_each_descendant([&](tuinator::Widget* widget) {
        if (auto* split = dynamic_cast<tuinator::SplitPane*>(widget)) {
            split->append_divider_line(lines);
        }
    });

    TUINATOR_CHECK_EQ(lines.size(), 2U);
}

TUINATOR_TEST(split_pane_junction_glyph_selection) {
    auto junction = [](bool up, bool down, bool left, bool right) {
        if (up && down && left && right) {
            return tuinator::BoxDrawing::Cross;
        }
        if (up && down && right && !left) {
            return tuinator::BoxDrawing::VerticalAndRight;
        }
        if (down && left && right && !up) {
            return tuinator::BoxDrawing::DownAndHorizontal;
        }
        if (up && down) {
            return tuinator::BoxDrawing::Vertical;
        }
        return tuinator::BoxDrawing::Horizontal;
    };

    TUINATOR_CHECK(junction(true, true, true, true) == tuinator::BoxDrawing::Cross);
    TUINATOR_CHECK(junction(true, true, false, true) == tuinator::BoxDrawing::VerticalAndRight);
    TUINATOR_CHECK(junction(false, true, true, true) == tuinator::BoxDrawing::DownAndHorizontal);
    TUINATOR_CHECK_EQ(tuinator::box_drawing_glyph(tuinator::BoxDrawing::VerticalAndRight, tuinator::GlyphSet::Unicode),
                      "\xe2\x94\x9c");
}

TUINATOR_TEST(split_pane_trims_touching_panel_borders) {
    auto left = std::make_unique<tuinator::Panel>("Left");
    auto right = std::make_unique<tuinator::Panel>("Right");
    auto* left_ptr = left.get();
    auto* right_ptr = right.get();

    auto split = std::make_unique<tuinator::SplitPane>(
        std::move(left), std::move(right),
        tuinator::SplitPaneOptions{
            .orientation = tuinator::SplitOrientation::Horizontal,
            .first_size = 12,
        });
    split->layout({0, 0, 40, 10});

    TUINATOR_CHECK(!left_ptr->border_edges().right);
    TUINATOR_CHECK(!right_ptr->border_edges().left);
    TUINATOR_CHECK(left_ptr->border_edges().left);
    TUINATOR_CHECK(right_ptr->border_edges().right);
}

TUINATOR_TEST(split_pane_restores_panel_borders_on_relayout) {
    auto left = std::make_unique<tuinator::Panel>("Left");
    auto right = std::make_unique<tuinator::Panel>("Right");
    auto* left_ptr = left.get();

    auto split = std::make_unique<tuinator::SplitPane>(
        std::move(left), std::move(right),
        tuinator::SplitPaneOptions{
            .orientation = tuinator::SplitOrientation::Horizontal,
            .first_size = 12,
        });

    split->layout({0, 0, 40, 10});
    TUINATOR_CHECK(!left_ptr->border_edges().right);

    split->layout({0, 0, 40, 10});
    TUINATOR_CHECK(!left_ptr->border_edges().right);
    TUINATOR_CHECK(left_ptr->border_edges().top);
    TUINATOR_CHECK(left_ptr->border_edges().bottom);
}

TUINATOR_TEST(split_pane_nested_drag_resizes_each_splitter) {
    auto inner = std::make_unique<tuinator::SplitPane>(
        make_label("left"), make_label("right"),
        tuinator::SplitPaneOptions{
            .orientation = tuinator::SplitOrientation::Horizontal,
            .first_size = 10,
        });
    auto* inner_ptr = inner.get();

    auto outer = std::make_unique<tuinator::SplitPane>(
        std::move(inner), make_label("bottom"),
        tuinator::SplitPaneOptions{
            .orientation = tuinator::SplitOrientation::Vertical,
            .first_size = 6,
        });
    auto* outer_ptr = outer.get();

    outer->layout({0, 0, 40, 20});

    const tuinator::Point inner_divider = horizontal_divider_point(*inner_ptr, inner_ptr->first_size());
    const tuinator::Point outer_divider = vertical_divider_point(*outer_ptr, outer_ptr->first_size());

    TUINATOR_CHECK(inner_ptr->handle_event(tuinator::MouseEvent{
        inner_divider, tuinator::MouseButton::Left, tuinator::MouseAction::Press, true}));
    TUINATOR_CHECK(inner_ptr->handle_event(tuinator::MouseEvent{
        {inner_divider.x + 5, inner_divider.y}, tuinator::MouseButton::Left, tuinator::MouseAction::Move, true}));
    TUINATOR_CHECK_EQ(inner_ptr->first_size(), 15);
    TUINATOR_CHECK(inner_ptr->handle_event(tuinator::MouseEvent{
        {inner_divider.x + 5, inner_divider.y}, tuinator::MouseButton::Left, tuinator::MouseAction::Release, false}));

    TUINATOR_CHECK(outer_ptr->handle_event(tuinator::MouseEvent{
        outer_divider, tuinator::MouseButton::Left, tuinator::MouseAction::Press, true}));
    TUINATOR_CHECK(outer_ptr->handle_event(tuinator::MouseEvent{
        {outer_divider.x, outer_divider.y + 4}, tuinator::MouseButton::Left, tuinator::MouseAction::Move, true}));
    TUINATOR_CHECK_EQ(outer_ptr->first_size(), 10);
}
