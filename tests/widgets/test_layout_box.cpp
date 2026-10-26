#include <tuinator/layout/box.hpp>
#include <tuinator/layout/grid.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/display/label.hpp>

#include "test_harness.hpp"

#include <memory>

namespace {

std::unique_ptr<tuinator::Widget> label_at(std::string text) {
    return std::make_unique<tuinator::Label>(std::move(text));
}

} // namespace

TUINATOR_TEST(vbox_stacks_children_vertically) {
    tuinator::VBox box(tuinator::BoxOptions{.gap = 0, .padding = 0});
    box.add_child(label_at("top"));
    box.add_child(label_at("bottom"));
    box.layout({0, 0, 20, 8});

    const tuinator::Rect top = box.children()[0]->bounds();
    const tuinator::Rect bottom = box.children()[1]->bounds();
    TUINATOR_CHECK(top.y < bottom.y);
    TUINATOR_CHECK_EQ(top.x, bottom.x);
}

TUINATOR_TEST(hbox_places_children_left_to_right) {
    tuinator::HBox row(tuinator::BoxOptions{.gap = 1, .padding = 0});
    row.add_child(label_at("L"));
    row.add_child(label_at("R"));
    row.layout({0, 0, 24, 3});

    const tuinator::Rect left = row.children()[0]->bounds();
    const tuinator::Rect right = row.children()[1]->bounds();
    TUINATOR_CHECK(left.x < right.x);
    TUINATOR_CHECK_EQ(left.y, right.y);
}

TUINATOR_TEST(layout_grid_places_children_in_row_major_order) {
    tuinator::Grid grid(tuinator::GridOptions{.columns = 2, .gap = 1, .padding = 0});
    grid.add_child(label_at("A"));
    grid.add_child(label_at("B"));
    grid.add_child(label_at("C"));
    grid.add_child(label_at("D"));
    grid.layout({0, 0, 30, 10});

    const tuinator::Rect a = grid.children()[0]->bounds();
    const tuinator::Rect b = grid.children()[1]->bounds();
    const tuinator::Rect c = grid.children()[2]->bounds();

    TUINATOR_CHECK(a.y == b.y);
    TUINATOR_CHECK(a.x < b.x);
    TUINATOR_CHECK(c.y > a.y);
}

TUINATOR_TEST(layout_boxes_expose_layout_box_capability) {
    tuinator::VBox vbox;
    tuinator::HBox hbox;
    tuinator::Grid grid;

    TUINATOR_CHECK(tuinator::as_layout_box(&vbox) != nullptr);
    TUINATOR_CHECK(tuinator::as_layout_box(&hbox) != nullptr);
    TUINATOR_CHECK(tuinator::as_layout_box(&grid) != nullptr);
    TUINATOR_CHECK_EQ(grid.columns(), 2);
}
