#pragma once

#include <tuinator/render/color.hpp>
#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/style.hpp>

#include <optional>
#include <string>
#include <string_view>

namespace tuinator {

/// Nerd Fonts glyph set (see nerd-fonts bin/scripts/lib/i_*.sh).
enum class UiIconSet {
    Codicon,
    Devicon,
};

/// UI / tool / OS symbols from Nerd Fonts (v3.4.0).
enum class UiIcon {
    DiffAdded,
    DiffModified,
    DiffRemoved,
    DiffRenamed,
    DiffIgnored,
    Diff,
    GitCommit,
    GitCompare,
    GitMerge,
    GitPullRequest,
    GitPullRequestClosed,
    GitFetch,
    Git,
    GitBranch,
    DevGitCommit,
    DevGitMerge,
    DevGitCompare,
    DevGitPullRequest,
    Github,
    Gitlab,
    Close,
    ChromeClose,
    Warning,
    Info,
    Error,
    Check,
    Bell,
    Search,
    Settings,
    Home,
    Refresh,
    Sync,
    Account,
    Play,
    StopCircle,
    File,
    Files,
    FileCode,
    SymbolNumeric,
    Terminal,
    TerminalLinux,
    TerminalBash,
    Linux,
    MacOS,
    Windows,
    Windows11,
    Ubuntu,
    Fedora,
    Archlinux,
    Debian,
    Docker,
    Neovim,
    Vim,
    Nodejs,
    Python,
    Rust,
};

struct UiIconDescriptor {
    UiIcon kind = UiIcon::DiffAdded;
    UiIconSet set = UiIconSet::Codicon;
    const char* nerd_name = "";
    char32_t codepoint = 0;
    char ascii_fallback = '?';
    Rgb color{0xC0, 0xCA, 0xF5};
};

const UiIconDescriptor& ui_icon_descriptor(UiIcon icon);

/// Cheat-sheet name, e.g. `cod-diff_added` or `dev-linux`.
const char* ui_icon_nerd_name(UiIcon icon);

UiIconSet ui_icon_set(UiIcon icon);

std::optional<UiIcon> ui_icon_from_nerd_name(std::string_view nerd_name);

std::string ui_icon_glyph(UiIcon icon, GlyphSet glyphs = GlyphSet::Auto);

Rgb ui_icon_color(UiIcon icon);

Style ui_icon_style(UiIcon icon, GlyphSet glyphs = GlyphSet::Auto);

} // namespace tuinator
