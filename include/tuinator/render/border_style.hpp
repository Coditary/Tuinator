#pragma once

namespace tuinator {

enum class BorderStyle {
    Ascii,
    Light,   // unicode light box ─│┌┐└┘
    Heavy,   // unicode heavy ━┃┏┓┗┛
    Double,  // ═║╔╗╚╝
    Rounded, // ╭╮╰╯ with light lines
};

struct BorderEdges {
    bool top = true;
    bool right = true;
    bool bottom = true;
    bool left = true;
};

enum class BorderEdge {
    Top,
    Right,
    Bottom,
    Left,
};

} // namespace tuinator
