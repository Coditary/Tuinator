#pragma once

#include <tuinator/render/canvas.hpp>
#include <tuinator/render/theme.hpp>

#include <functional>

namespace tuinator {

struct PaintContext {
    Canvas& canvas;
    const Theme& theme;

    PaintContext child(Canvas& child_canvas) const { return PaintContext{child_canvas, theme}; }

    void with_clip(Rect rect, const std::function<void(PaintContext&)>& draw) const {
        canvas.with_clip(rect, [&](Canvas& clipped) {
            PaintContext child_ctx = child(clipped);
            draw(child_ctx);
        });
    }

    const BorderGlyphs& glyphs() const { return theme.glyphs; }
};

} // namespace tuinator
