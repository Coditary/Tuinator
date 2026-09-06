#pragma once

#include <algorithm>

namespace tuinator {

struct Point {
    int x = 0;
    int y = 0;

    constexpr Point() = default;
    constexpr Point(int x, int y) : x(x), y(y) {}
};

struct Size {
    int width = 0;
    int height = 0;

    constexpr Size() = default;
    constexpr Size(int width, int height) : width(width), height(height) {}
};

struct Rect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    constexpr Rect() = default;
    constexpr Rect(int x, int y, int width, int height)
        : x(x), y(y), width(width), height(height) {}

    constexpr Rect(Point origin, Size size)
        : x(origin.x), y(origin.y), width(size.width), height(size.height) {}

    constexpr Point origin() const { return {x, y}; }
    constexpr Size size() const { return {width, height}; }

    constexpr int right() const { return x + width; }
    constexpr int bottom() const { return y + height; }

    constexpr bool contains(Point p) const {
        return p.x >= x && p.x < right() && p.y >= y && p.y < bottom();
    }

    Rect centered_in(Rect outer) const {
        return {
            outer.x + (outer.width - width) / 2,
            outer.y + (outer.height - height) / 2,
            width,
            height,
        };
    }

    Rect clamped_to(Rect outer) const {
        const int clamped_width = std::min(width, outer.width);
        const int clamped_height = std::min(height, outer.height);
        int clamped_x = x;
        int clamped_y = y;

        if (clamped_x < outer.x) {
            clamped_x = outer.x;
        }
        if (clamped_y < outer.y) {
            clamped_y = outer.y;
        }
        if (clamped_x + clamped_width > outer.right()) {
            clamped_x = outer.right() - clamped_width;
        }
        if (clamped_y + clamped_height > outer.bottom()) {
            clamped_y = outer.bottom() - clamped_height;
        }

        return {clamped_x, clamped_y, clamped_width, clamped_height};
    }

    Rect translated(int dx, int dy) const {
        return {x + dx, y + dy, width, height};
    }

    Rect inset(int amount) const {
        return {x + amount, y + amount, width - amount * 2, height - amount * 2};
    }
};

inline Rect intersect(Rect a, Rect b) {
    const int x = std::max(a.x, b.x);
    const int y = std::max(a.y, b.y);
    const int right = std::min(a.right(), b.right());
    const int bottom = std::min(a.bottom(), b.bottom());

    if (right <= x || bottom <= y) {
        return {};
    }

    return {x, y, right - x, bottom - y};
}

inline Rect unite(Rect a, Rect b) {
    if (a.width <= 0 || a.height <= 0) {
        return b;
    }
    if (b.width <= 0 || b.height <= 0) {
        return a;
    }

    const int x = std::min(a.x, b.x);
    const int y = std::min(a.y, b.y);
    const int right = std::max(a.right(), b.right());
    const int bottom = std::max(a.bottom(), b.bottom());
    return {x, y, right - x, bottom - y};
}

} // namespace tuinator
