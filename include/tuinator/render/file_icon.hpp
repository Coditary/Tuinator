#pragma once

#include <tuinator/render/color.hpp>
#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/style.hpp>

#include <string>
#include <string_view>

namespace tuinator {

/// Semantic file / folder kinds with Nerd Font glyphs (nvim-web-devicons compatible).
enum class FileIcon {
    Default,
    Folder,
    FolderOpen,

    C,
    Cpp,
    Header,
    CSharp,
    Go,
    Java,
    JavaScript,
    Jsx,
    Kotlin,
    Lua,
    Php,
    Python,
    Ruby,
    Rust,
    Shell,
    Swift,
    TypeScript,
    Tsx,
    Vim,
    Zig,

    Css,
    Html,
    Json,
    Markdown,
    Scss,
    Sql,
    Toml,
    Yaml,

    Archive,
    CMake,
    Docker,
    Executable,
    Git,
    GitBranch,
    Image,
    Lock,
    Makefile,
    PackageJson,
    Pdf,
    Svg,
    Text,
};

struct FileIconDescriptor {
    FileIcon kind = FileIcon::Default;
    const char* name = "default";
    char32_t nerd_codepoint = 0xF0F6;
    char ascii_fallback = '*';
    Rgb color{0x6D, 0x80, 0x86};
};

const FileIconDescriptor& file_icon_descriptor(FileIcon icon);

/// Nerd-font glyph or ASCII fallback, depending on `glyphs`.
std::string file_icon_glyph(FileIcon icon, GlyphSet glyphs = GlyphSet::Auto);

/// Foreground color used by nvim-web-devicons for this icon.
Rgb file_icon_color(FileIcon icon);

Style file_icon_style(FileIcon icon, GlyphSet glyphs = GlyphSet::Auto);

FileIcon file_icon_for_extension(std::string_view extension);
FileIcon file_icon_for_filename(std::string_view filename);

/// Detect icon from a path. Pass `is_directory` for folders.
FileIcon file_icon_for_path(std::string_view path, bool is_directory = false, bool folder_open = false);

std::string file_icon_glyph_for_path(std::string_view path, GlyphSet glyphs = GlyphSet::Auto, bool is_directory = false,
                                     bool folder_open = false);

} // namespace tuinator
