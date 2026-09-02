#include "test_harness.hpp"

#include <cstring>

#include <tuinator/render/file_icon.hpp>
#include <tuinator/render/glyphs.hpp>

TUINATOR_TEST(file_icon_descriptor_matches_enum_order) {
    TUINATOR_CHECK(std::strcmp(tuinator::file_icon_descriptor(tuinator::FileIcon::Lua).name, "lua") == 0);
    TUINATOR_CHECK(std::strcmp(tuinator::file_icon_descriptor(tuinator::FileIcon::Rust).name, "rs") == 0);
    TUINATOR_CHECK(std::strcmp(tuinator::file_icon_descriptor(tuinator::FileIcon::Folder).name, "folder") == 0);
}

TUINATOR_TEST(file_icon_for_extension_detects_languages) {
    TUINATOR_CHECK_EQ(tuinator::file_icon_for_extension("lua"), tuinator::FileIcon::Lua);
    TUINATOR_CHECK_EQ(tuinator::file_icon_for_extension("rs"), tuinator::FileIcon::Rust);
    TUINATOR_CHECK_EQ(tuinator::file_icon_for_extension("cpp"), tuinator::FileIcon::Cpp);
    TUINATOR_CHECK_EQ(tuinator::file_icon_for_extension("py"), tuinator::FileIcon::Python);
}

TUINATOR_TEST(file_icon_for_filename_special_cases) {
    TUINATOR_CHECK_EQ(tuinator::file_icon_for_filename("Makefile"), tuinator::FileIcon::Makefile);
    TUINATOR_CHECK_EQ(tuinator::file_icon_for_filename("CMakeLists.txt"), tuinator::FileIcon::CMake);
    TUINATOR_CHECK_EQ(tuinator::file_icon_for_filename(".gitignore"), tuinator::FileIcon::Git);
    TUINATOR_CHECK_EQ(tuinator::file_icon_for_filename("package.json"), tuinator::FileIcon::PackageJson);
}

TUINATOR_TEST(file_icon_for_path_directories) {
    TUINATOR_CHECK_EQ(tuinator::file_icon_for_path("src", true), tuinator::FileIcon::Folder);
    TUINATOR_CHECK_EQ(
        tuinator::file_icon_for_path("src", true, true),
        tuinator::FileIcon::FolderOpen);
}

TUINATOR_TEST(file_icon_glyph_ascii_fallback) {
    TUINATOR_CHECK_EQ(tuinator::file_icon_glyph(tuinator::FileIcon::Lua, tuinator::GlyphSet::Ascii), "l");
    TUINATOR_CHECK_EQ(tuinator::file_icon_glyph(tuinator::FileIcon::Folder, tuinator::GlyphSet::Ascii), "/");
}

TUINATOR_TEST(file_icon_glyph_unicode_differs_from_ascii) {
    const std::string lua = tuinator::file_icon_glyph(tuinator::FileIcon::Lua, tuinator::GlyphSet::Unicode);
    TUINATOR_CHECK(lua != "l");
    TUINATOR_CHECK(!lua.empty());
}

TUINATOR_TEST(file_icon_glyph_for_path_uses_basename) {
    TUINATOR_CHECK_EQ(
        tuinator::file_icon_for_path("diffview/file_history_panel.lua"),
        tuinator::FileIcon::Lua);
}

TUINATOR_TEST(file_icon_color_lua) {
    const tuinator::Rgb color = tuinator::file_icon_color(tuinator::FileIcon::Lua);
    TUINATOR_CHECK_EQ(color.r, 0x51);
    TUINATOR_CHECK_EQ(color.g, 0xA0);
    TUINATOR_CHECK_EQ(color.b, 0xCF);
}
