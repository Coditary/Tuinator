#pragma once

namespace tuinator {

enum class BorderStyle {
    Ascii,
    Light,   // unicode light box ─│┌┐└┘
    Heavy,   // unicode heavy ━┃┏┓┗┛
    Double,  // ═║╔╗╚╝
    Rounded, // ╭╮╰╯ with light lines
};

} // namespace tuinator
