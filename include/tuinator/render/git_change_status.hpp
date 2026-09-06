#pragma once

#include <tuinator/render/color.hpp>

#include <optional>

namespace tuinator {

/// Git porcelain short status codes (e.g. `git status --porcelain`).
enum class GitChangeStatus {
    Modified,
    Added,
    Deleted,
    Renamed,
    Copied,
    Untracked,
    Ignored,
    Unmerged,
    Unknown,
};

/// Default colors aligned with SourceControlPanelStyle.
struct GitChangeStatusColorDefaults {
    Rgb modified{0xe0, 0xaf, 0x68};
    Rgb added{0x9e, 0xce, 0x6a};
    Rgb deleted{0xf7, 0x76, 0x8e};
    Rgb text{0xc0, 0xca, 0xf5};
    Rgb muted{0x56, 0x5f, 0x89};
};

char git_change_status_char(GitChangeStatus status);

std::optional<GitChangeStatus> git_change_status_from_char(char ch);

const char* git_change_status_label(GitChangeStatus status);

Rgb git_change_status_color(GitChangeStatus status, GitChangeStatusColorDefaults defaults = {});

} // namespace tuinator
