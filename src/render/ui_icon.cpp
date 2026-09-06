#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/ui_icon.hpp>

#include <array>
#include <cstring>
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

constexpr UiIconDescriptor make(UiIcon kind, UiIconSet set, const char* nerd_name, char32_t codepoint, char ascii,
                                std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    return UiIconDescriptor{
        kind, set, nerd_name, codepoint, ascii, Rgb{r, g, b},
    };
}

// Codepoints from Nerd Fonts v3.4.0 (data/nerd_ui_icons.tsv).
constexpr std::array<UiIconDescriptor, 56> kDescriptors{{
    make(UiIcon::DiffAdded, UiIconSet::Codicon, "cod-diff_added", 0xEADC, '+', 0x9E, 0xCE, 0x6A),
    make(UiIcon::DiffModified, UiIconSet::Codicon, "cod-diff_modified", 0xEADE, '~', 0x7D, 0xCF, 0xFF),
    make(UiIcon::DiffRemoved, UiIconSet::Codicon, "cod-diff_removed", 0xEADF, '-', 0xF7, 0x76, 0x8E),
    make(UiIcon::DiffRenamed, UiIconSet::Codicon, "cod-diff_renamed", 0xEAE0, '>', 0x7D, 0xCF, 0xFF),
    make(UiIcon::DiffIgnored, UiIconSet::Codicon, "cod-diff_ignored", 0xEADD, ':', 0x56, 0x5F, 0x89),
    make(UiIcon::Diff, UiIconSet::Codicon, "cod-diff", 0xEAE1, 'd', 0x7D, 0xCF, 0xFF),
    make(UiIcon::GitCommit, UiIconSet::Codicon, "cod-git_commit", 0xEAFC, 'c', 0xBB, 0x9A, 0xF7),
    make(UiIcon::GitCompare, UiIconSet::Codicon, "cod-git_compare", 0xEAFD, 'c', 0xBB, 0x9A, 0xF7),
    make(UiIcon::GitMerge, UiIconSet::Codicon, "cod-git_merge", 0xEAFE, 'm', 0xBB, 0x9A, 0xF7),
    make(UiIcon::GitPullRequest, UiIconSet::Codicon, "cod-git_pull_request", 0xEA64, 'p', 0xBB, 0x9A, 0xF7),
    make(UiIcon::GitPullRequestClosed, UiIconSet::Codicon, "cod-git_pull_request_closed", 0xEBDA, 'p', 0xBB, 0x9A,
         0xF7),
    make(UiIcon::GitFetch, UiIconSet::Codicon, "cod-git_fetch", 0xEC1D, 'f', 0xBB, 0x9A, 0xF7),
    make(UiIcon::Git, UiIconSet::Devicon, "dev-git", 0xE702, 'g', 0xF1, 0x50, 0x2F),
    make(UiIcon::GitBranch, UiIconSet::Devicon, "dev-git_branch", 0xE725, 'b', 0xBB, 0x9A, 0xF7),
    make(UiIcon::DevGitCommit, UiIconSet::Devicon, "dev-git_commit", 0xE729, 'c', 0xBB, 0x9A, 0xF7),
    make(UiIcon::DevGitMerge, UiIconSet::Devicon, "dev-git_merge", 0xE727, 'm', 0xBB, 0x9A, 0xF7),
    make(UiIcon::DevGitCompare, UiIconSet::Devicon, "dev-git_compare", 0xE728, 'c', 0xBB, 0x9A, 0xF7),
    make(UiIcon::DevGitPullRequest, UiIconSet::Devicon, "dev-git_pull_request", 0xE726, 'p', 0xBB, 0x9A, 0xF7),
    make(UiIcon::Github, UiIconSet::Devicon, "dev-github", 0xE709, 'h', 0xC0, 0xCA, 0xF5),
    make(UiIcon::Gitlab, UiIconSet::Devicon, "dev-gitlab", 0xE7EB, 'l', 0xE2, 0x43, 0x29),
    make(UiIcon::Close, UiIconSet::Codicon, "cod-close", 0xEA76, 'x', 0x56, 0x5F, 0x89),
    make(UiIcon::ChromeClose, UiIconSet::Codicon, "cod-chrome_close", 0xEAB8, 'x', 0x56, 0x5F, 0x89),
    make(UiIcon::Warning, UiIconSet::Codicon, "cod-warning", 0xEA6C, '!', 0xE0, 0xAF, 0x68),
    make(UiIcon::Info, UiIconSet::Codicon, "cod-info", 0xEA74, 'i', 0x7D, 0xCF, 0xFF),
    make(UiIcon::Error, UiIconSet::Codicon, "cod-error", 0xEA87, '!', 0xF7, 0x76, 0x8E),
    make(UiIcon::Check, UiIconSet::Codicon, "cod-check", 0xEAB2, '+', 0x9E, 0xCE, 0x6A),
    make(UiIcon::Bell, UiIconSet::Codicon, "cod-bell", 0xEAA2, 'b', 0xE0, 0xAF, 0x68),
    make(UiIcon::Search, UiIconSet::Codicon, "cod-search", 0xEA6D, '?', 0x7D, 0xCF, 0xFF),
    make(UiIcon::Settings, UiIconSet::Codicon, "cod-settings", 0xEB52, '*', 0xC0, 0xCA, 0xF5),
    make(UiIcon::Home, UiIconSet::Codicon, "cod-home", 0xEB06, 'h', 0xC0, 0xCA, 0xF5),
    make(UiIcon::Refresh, UiIconSet::Codicon, "cod-refresh", 0xEB37, 'r', 0x7D, 0xCF, 0xFF),
    make(UiIcon::Sync, UiIconSet::Codicon, "cod-sync", 0xEA77, 's', 0x7D, 0xCF, 0xFF),
    make(UiIcon::Account, UiIconSet::Codicon, "cod-account", 0xEB99, '@', 0xC0, 0xCA, 0xF5),
    make(UiIcon::Play, UiIconSet::Codicon, "cod-play", 0xEB2C, '>', 0x9E, 0xCE, 0x6A),
    make(UiIcon::StopCircle, UiIconSet::Codicon, "cod-stop_circle", 0xEBA5, 's', 0xF7, 0x76, 0x8E),
    make(UiIcon::File, UiIconSet::Codicon, "cod-file", 0xEA7B, 'f', 0x7D, 0xCF, 0xFF),
    make(UiIcon::Files, UiIconSet::Codicon, "cod-files", 0xEAF0, 'f', 0x7D, 0xCF, 0xFF),
    make(UiIcon::FileCode, UiIconSet::Codicon, "cod-file_code", 0xEAE9, 'f', 0x7D, 0xCF, 0xFF),
    make(UiIcon::SymbolNumeric, UiIconSet::Codicon, "cod-symbol_numeric", 0xEA90, '#', 0xE0, 0xAF, 0x68),
    make(UiIcon::Terminal, UiIconSet::Codicon, "cod-terminal", 0xEA85, '>', 0x9E, 0xCE, 0x6A),
    make(UiIcon::TerminalLinux, UiIconSet::Codicon, "cod-terminal_linux", 0xEBC6, '>', 0xFF, 0xFF, 0xFF),
    make(UiIcon::TerminalBash, UiIconSet::Codicon, "cod-terminal_bash", 0xEBCA, '$', 0x9E, 0xCE, 0x6A),
    make(UiIcon::Linux, UiIconSet::Devicon, "dev-linux", 0xE712, 'L', 0xFF, 0xFF, 0xFF),
    make(UiIcon::MacOS, UiIconSet::Devicon, "dev-apple", 0xE711, 'M', 0xC0, 0xCA, 0xF5),
    make(UiIcon::Windows, UiIconSet::Devicon, "dev-windows", 0xE70F, 'W', 0x00, 0xA4, 0xEF),
    make(UiIcon::Windows11, UiIconSet::Devicon, "dev-windows11", 0xE8E5, 'W', 0x00, 0xA4, 0xEF),
    make(UiIcon::Ubuntu, UiIconSet::Devicon, "dev-ubuntu", 0xE73A, 'U', 0xE9, 0x54, 0x20),
    make(UiIcon::Fedora, UiIconSet::Devicon, "dev-fedora", 0xE7D9, 'F', 0x29, 0x4E, 0x9D),
    make(UiIcon::Archlinux, UiIconSet::Devicon, "dev-archlinux", 0xE732, 'A', 0x17, 0x9C, 0xD3),
    make(UiIcon::Debian, UiIconSet::Devicon, "dev-debian", 0xE77D, 'D', 0xD7, 0x0A, 0x53),
    make(UiIcon::Docker, UiIconSet::Devicon, "dev-docker", 0xE7B0, 'D', 0x45, 0x8E, 0xE6),
    make(UiIcon::Neovim, UiIconSet::Devicon, "dev-neovim", 0xE83A, 'N', 0x57, 0xA1, 0x43),
    make(UiIcon::Vim, UiIconSet::Devicon, "dev-vim", 0xE7C5, 'V', 0x01, 0x98, 0x33),
    make(UiIcon::Nodejs, UiIconSet::Devicon, "dev-nodejs", 0xE719, 'n', 0x3C, 0x87, 0x3A),
    make(UiIcon::Python, UiIconSet::Devicon, "dev-python", 0xE73C, 'y', 0xFF, 0xBC, 0x03),
    make(UiIcon::Rust, UiIconSet::Devicon, "dev-rust", 0xE7A8, 'R', 0xDE, 0xA5, 0x84),
}};

static_assert(kDescriptors.size() == 56, "descriptor table out of sync with UiIcon enum");

const UiIconDescriptor& descriptor_or_default(UiIcon icon) {
    const auto index = static_cast<std::size_t>(icon);
    if (index < kDescriptors.size()) {
        return kDescriptors[index];
    }
    return kDescriptors[static_cast<std::size_t>(UiIcon::Info)];
}

} // namespace

const UiIconDescriptor& ui_icon_descriptor(UiIcon icon) { return descriptor_or_default(icon); }

const char* ui_icon_nerd_name(UiIcon icon) { return ui_icon_descriptor(icon).nerd_name; }

UiIconSet ui_icon_set(UiIcon icon) { return ui_icon_descriptor(icon).set; }

std::optional<UiIcon> ui_icon_from_nerd_name(std::string_view nerd_name) {
    for (const UiIconDescriptor& descriptor : kDescriptors) {
        if (nerd_name == descriptor.nerd_name) {
            return descriptor.kind;
        }
    }
    return std::nullopt;
}

std::string ui_icon_glyph(UiIcon icon, GlyphSet glyphs) {
    const UiIconDescriptor& descriptor = ui_icon_descriptor(icon);

    if (glyphs == GlyphSet::Auto) {
        glyphs = detect_file_icon_glyph_set();
    }

    if (glyphs == GlyphSet::Ascii) {
        return std::string(1, descriptor.ascii_fallback);
    }

    return utf8_from(descriptor.codepoint);
}

Rgb ui_icon_color(UiIcon icon) { return ui_icon_descriptor(icon).color; }

Style ui_icon_style(UiIcon icon, GlyphSet glyphs) {
    (void)glyphs;
    return style_fg(ui_icon_color(icon));
}

} // namespace tuinator
