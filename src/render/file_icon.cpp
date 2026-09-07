#include <tuinator/render/file_icon.hpp>
#include <tuinator/render/glyphs.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <string>

namespace tuinator {

namespace {

std::string utf8_from(char32_t cp) {
    std::string out;
    if (cp < 0x80) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    return out;
}

constexpr FileIconDescriptor make(FileIcon kind, const char* name, char32_t codepoint, char ascii, std::uint8_t r,
                                  std::uint8_t g, std::uint8_t b) {
    return FileIconDescriptor{
        kind, name, codepoint, ascii, Rgb{r, g, b},
    };
}

// Codepoints and colors follow nvim-web-devicons defaults (Nerd Fonts v3).
constexpr std::array<FileIconDescriptor, 44> kDescriptors{{
    make(FileIcon::Default, "default", 0xF0F6, '*', 0x6D, 0x80, 0x86),
    make(FileIcon::Folder, "folder", 0xF024B, '/', 0xDC, 0xA9, 0x62),
    make(FileIcon::FolderOpen, "folder_open", 0xF0256, 'v', 0xDC, 0xA9, 0x62),

    make(FileIcon::C, "c", 0xE61E, 'c', 0x51, 0x9A, 0xBA),
    make(FileIcon::Cpp, "cpp", 0xE61D, 'C', 0x51, 0x9A, 0xBA),
    make(FileIcon::Header, "header", 0xF0FD, 'h', 0xA0, 0x74, 0xC4),
    make(FileIcon::CSharp, "cs", 0xF031B, '#', 0x51, 0x9A, 0xBA),
    make(FileIcon::Go, "go", 0xE627, 'g', 0x00, 0xAC, 0xD7),
    make(FileIcon::Java, "java", 0xE738, 'J', 0xCC, 0x3E, 0x44),
    make(FileIcon::JavaScript, "js", 0xE60C, 'j', 0xCB, 0xCB, 0x41),
    make(FileIcon::Jsx, "jsx", 0xE625, 'x', 0x20, 0xA5, 0xDA),
    make(FileIcon::Kotlin, "kt", 0xE634, 'k', 0xA9, 0x79, 0xBE),
    make(FileIcon::Lua, "lua", 0xE620, 'l', 0x51, 0xA0, 0xCF),
    make(FileIcon::Php, "php", 0xE608, 'p', 0x89, 0x9B, 0xD9),
    make(FileIcon::Python, "py", 0xE606, 'y', 0xFF, 0xBC, 0x03),
    make(FileIcon::Ruby, "rb", 0xE791, 'r', 0xCC, 0x3E, 0x44),
    make(FileIcon::Rust, "rs", 0xE68B, 'R', 0xDE, 0xA5, 0x84),
    make(FileIcon::Shell, "sh", 0xE795, '$', 0x42, 0x88, 0x50),
    make(FileIcon::Swift, "swift", 0xE755, 's', 0xF0, 0x51, 0x38),
    make(FileIcon::TypeScript, "ts", 0xE628, 't', 0x31, 0x7C, 0xC8),
    make(FileIcon::Tsx, "tsx", 0xE7BA, 'X', 0x20, 0xA5, 0xDA),
    make(FileIcon::Vim, "vim", 0xE62B, 'v', 0x01, 0x98, 0x33),
    make(FileIcon::Zig, "zig", 0xE6A9, 'z', 0xF7, 0xA4, 0x1D),

    make(FileIcon::Css, "css", 0xE6B8, '%', 0x42, 0xA5, 0xD5),
    make(FileIcon::Html, "html", 0xE736, 'H', 0xE4, 0x4D, 0x26),
    make(FileIcon::Json, "json", 0xE60B, '{', 0xCB, 0xCB, 0x41),
    make(FileIcon::Markdown, "md", 0xF48A, 'm', 0xDD, 0xDD, 0xDD),
    make(FileIcon::Scss, "scss", 0xE603, 's', 0xC6, 0x53, 0x8C),
    make(FileIcon::Sql, "sql", 0xE706, 'q', 0xE3, 0x8B, 0xA1),
    make(FileIcon::Toml, "toml", 0xE6B2, 'T', 0x9C, 0x42, 0x21),
    make(FileIcon::Yaml, "yaml", 0xE8EB, 'y', 0xCB, 0x17, 0x16),

    make(FileIcon::Archive, "zip", 0xF410, 'z', 0xEC, 0xA4, 0x07),
    make(FileIcon::CMake, "cmake", 0xE794, 'M', 0xDC, 0xE3, 0xEB),
    make(FileIcon::Docker, "docker", 0xF0868, 'D', 0x45, 0x8E, 0xE6),
    make(FileIcon::Executable, "exe", 0xEAE8, '!', 0x9E, 0xCE, 0x6A),
    make(FileIcon::Git, "git", 0xE702, 'g', 0xF1, 0x50, 0x2F),
    make(FileIcon::GitBranch, "git_branch", 0xF063C, 'Y', 0xBB, 0x9A, 0xF7),
    make(FileIcon::Image, "image", 0xE60D, 'i', 0xA0, 0x74, 0xC4),
    make(FileIcon::Lock, "lock", 0xE672, 'L', 0xBC, 0xA3, 0xA4),
    make(FileIcon::Makefile, "make", 0xE779, 'M', 0x6D, 0x80, 0x86),
    make(FileIcon::PackageJson, "npm", 0xE71E, 'n', 0xE8, 0x27, 0x4B),
    make(FileIcon::Pdf, "pdf", 0xEAEB, 'P', 0xEC, 0x1C, 0x24),
    make(FileIcon::Svg, "svg", 0xF0721, 'S', 0xFF, 0xB3, 0x00),
    make(FileIcon::Text, "txt", 0xF0219, 't', 0x89, 0xD1, 0xFF),
}};

static_assert(kDescriptors.size() == 44, "descriptor table out of sync with FileIcon enum");

std::string to_lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

std::string basename(std::string_view path) {
    if (path.empty()) {
        return {};
    }

    std::size_t end = path.size();
    while (end > 0 && (path[end - 1] == '/' || path[end - 1] == '\\')) {
        --end;
    }

    std::size_t start = end;
    while (start > 0) {
        const char ch = path[start - 1];
        if (ch == '/' || ch == '\\') {
            break;
        }
        --start;
    }

    return std::string(path.substr(start, end - start));
}

std::string extension_of(std::string_view filename) {
    if (filename.empty() || (filename.front() == '.' && filename.find('.', 1) == std::string_view::npos)) {
        return {};
    }

    const std::size_t dot = filename.rfind('.');
    if (dot == std::string_view::npos || dot + 1 >= filename.size()) {
        return {};
    }

    return std::string(filename.substr(dot + 1));
}

const FileIconDescriptor& descriptor_or_default(FileIcon icon) {
    const std::size_t index = static_cast<std::size_t>(icon);
    if (index < kDescriptors.size() && kDescriptors[index].kind == icon) {
        return kDescriptors[index];
    }
    return kDescriptors[0];
}

FileIcon icon_for_extension_lower(std::string_view extension) {
    if (extension.empty()) {
        return FileIcon::Default;
    }

    const std::string ext = to_lower(std::string(extension));

    if (ext == "c") {
        return FileIcon::C;
    }
    if (ext == "cc" || ext == "cpp" || ext == "cxx" || ext == "cppm") {
        return FileIcon::Cpp;
    }
    if (ext == "h" || ext == "hpp" || ext == "hh" || ext == "hxx") {
        return FileIcon::Header;
    }
    if (ext == "cs") {
        return FileIcon::CSharp;
    }
    if (ext == "go") {
        return FileIcon::Go;
    }
    if (ext == "java") {
        return FileIcon::Java;
    }
    if (ext == "js" || ext == "mjs" || ext == "cjs") {
        return FileIcon::JavaScript;
    }
    if (ext == "jsx") {
        return FileIcon::Jsx;
    }
    if (ext == "kt" || ext == "kts") {
        return FileIcon::Kotlin;
    }
    if (ext == "lua" || ext == "luac" || ext == "luau") {
        return FileIcon::Lua;
    }
    if (ext == "php") {
        return FileIcon::Php;
    }
    if (ext == "py" || ext == "pyw" || ext == "pyi") {
        return FileIcon::Python;
    }
    if (ext == "rb") {
        return FileIcon::Ruby;
    }
    if (ext == "rs") {
        return FileIcon::Rust;
    }
    if (ext == "sh" || ext == "bash" || ext == "zsh" || ext == "fish") {
        return FileIcon::Shell;
    }
    if (ext == "swift") {
        return FileIcon::Swift;
    }
    if (ext == "ts") {
        return FileIcon::TypeScript;
    }
    if (ext == "tsx") {
        return FileIcon::Tsx;
    }
    if (ext == "vim") {
        return FileIcon::Vim;
    }
    if (ext == "zig") {
        return FileIcon::Zig;
    }
    if (ext == "css") {
        return FileIcon::Css;
    }
    if (ext == "html" || ext == "htm") {
        return FileIcon::Html;
    }
    if (ext == "json" || ext == "jsonc" || ext == "json5") {
        return FileIcon::Json;
    }
    if (ext == "md" || ext == "markdown") {
        return FileIcon::Markdown;
    }
    if (ext == "scss" || ext == "sass") {
        return FileIcon::Scss;
    }
    if (ext == "sql") {
        return FileIcon::Sql;
    }
    if (ext == "toml") {
        return FileIcon::Toml;
    }
    if (ext == "yaml" || ext == "yml") {
        return FileIcon::Yaml;
    }
    if (ext == "zip" || ext == "tar" || ext == "gz" || ext == "bz2" || ext == "xz" || ext == "7z" || ext == "rar") {
        return FileIcon::Archive;
    }
    if (ext == "cmake") {
        return FileIcon::CMake;
    }
    if (ext == "dockerfile") {
        return FileIcon::Docker;
    }
    if (ext == "exe" || ext == "dll" || ext == "so" || ext == "dylib") {
        return FileIcon::Executable;
    }
    if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "gif" || ext == "webp" || ext == "bmp") {
        return FileIcon::Image;
    }
    if (ext == "lock") {
        return FileIcon::Lock;
    }
    if (ext == "pdf") {
        return FileIcon::Pdf;
    }
    if (ext == "svg") {
        return FileIcon::Svg;
    }
    if (ext == "txt") {
        return FileIcon::Text;
    }

    return FileIcon::Default;
}

} // namespace

const FileIconDescriptor& file_icon_descriptor(FileIcon icon) { return descriptor_or_default(icon); }

std::string file_icon_glyph(FileIcon icon, GlyphSet glyphs) {
    const FileIconDescriptor& descriptor = file_icon_descriptor(icon);

    if (glyphs == GlyphSet::Auto) {
        glyphs = detect_file_icon_glyph_set();
    }

    if (glyphs == GlyphSet::Ascii) {
        return std::string(1, descriptor.ascii_fallback);
    }

    return utf8_from(descriptor.nerd_codepoint);
}

Rgb file_icon_color(FileIcon icon) { return file_icon_descriptor(icon).color; }

Style file_icon_style(FileIcon icon, GlyphSet glyphs) {
    (void)glyphs;
    return style_fg(file_icon_color(icon));
}

FileIcon file_icon_for_extension(std::string_view extension) { return icon_for_extension_lower(extension); }

FileIcon file_icon_for_filename(std::string_view filename) {
    const std::string lower = to_lower(std::string(filename));

    if (lower == ".gitignore" || lower == ".gitattributes" || lower == ".gitmodules") {
        return FileIcon::Git;
    }
    if (lower == "cmakelists.txt") {
        return FileIcon::CMake;
    }
    if (lower == "dockerfile") {
        return FileIcon::Docker;
    }
    if (lower == "makefile" || lower == "gnumakefile") {
        return FileIcon::Makefile;
    }
    if (lower == "package.json" || lower == "package-lock.json") {
        return FileIcon::PackageJson;
    }
    if (lower == "cargo.toml" || lower == "cargo.lock") {
        return lower.find("lock") != std::string::npos ? FileIcon::Lock : FileIcon::Rust;
    }
    if (lower == "readme.md" || lower == "readme") {
        return FileIcon::Markdown;
    }

    return icon_for_extension_lower(extension_of(lower));
}

FileIcon file_icon_for_path(std::string_view path, bool is_directory, bool folder_open) {
    if (is_directory) {
        return folder_open ? FileIcon::FolderOpen : FileIcon::Folder;
    }

    return file_icon_for_filename(basename(path));
}

std::string file_icon_glyph_for_path(std::string_view path, GlyphSet glyphs, bool is_directory, bool folder_open) {
    return file_icon_glyph(file_icon_for_path(path, is_directory, folder_open), glyphs);
}

} // namespace tuinator
