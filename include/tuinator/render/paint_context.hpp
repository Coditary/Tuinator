#pragma once

#include <tuinator/render/canvas.hpp>
#include <tuinator/render/style_resolver.hpp>
#include <tuinator/render/theme.hpp>

#include <functional>

namespace tuinator {

class Stylesheet;

struct PaintContext {
    Canvas& canvas;
    const Theme& theme;
    const Stylesheet* stylesheet = nullptr;

    PaintContext child(Canvas& child_canvas) const { return PaintContext{child_canvas, theme, stylesheet}; }

    void with_clip(Rect rect, const std::function<void(PaintContext&)>& draw) const {
        canvas.with_clip(rect, [&](Canvas& clipped) {
            PaintContext child_ctx = child(clipped);
            draw(child_ctx);
        });
    }

    const BorderGlyphs& glyphs() const { return theme.glyphs; }

    StyleResolver styles() const { return StyleResolver{theme, stylesheet}; }
};

} // namespace tuinator
