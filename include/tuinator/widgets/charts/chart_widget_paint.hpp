#pragma once

#include <tuinator/render/paint_context.hpp>
#include <tuinator/widgets/charts/chart_widget.hpp>

namespace tuinator {

/// Shared paint-time stylesheet resolution for chart widgets.
struct ChartPaintSupport {
    mutable ChartStyleBundle styles{};

    void prepare(const PaintContext& ctx, const Widget& widget, const Style& title, const Style& axis,
                 const Style& grid, const Style& value) const {
        prepare_chart_paint(ctx, widget, title, axis, grid, value, styles);
    }
};

} // namespace tuinator
