#include <tuinator/render/git_change_status.hpp>

#include <array>
#include <cctype>

namespace tuinator {

namespace {

struct GitChangeStatusDescriptor {
    GitChangeStatus status;
    char ch;
    const char* label;
};

constexpr std::array<GitChangeStatusDescriptor, 9> kDescriptors{{
    {GitChangeStatus::Modified, 'M', "Modified"},
    {GitChangeStatus::Added, 'A', "Added"},
    {GitChangeStatus::Deleted, 'D', "Deleted"},
    {GitChangeStatus::Renamed, 'R', "Renamed"},
    {GitChangeStatus::Copied, 'C', "Copied"},
    {GitChangeStatus::Untracked, '?', "Untracked"},
    {GitChangeStatus::Ignored, '!', "Ignored"},
    {GitChangeStatus::Unmerged, 'U', "Unmerged"},
    {GitChangeStatus::Unknown, ' ', "Unknown"},
}};

const GitChangeStatusDescriptor& descriptor(GitChangeStatus status) {
    const auto index = static_cast<std::size_t>(status);
    return kDescriptors[index < kDescriptors.size() ? index : kDescriptors.size() - 1];
}

} // namespace

char git_change_status_char(GitChangeStatus status) {
    return descriptor(status).ch;
}

std::optional<GitChangeStatus> git_change_status_from_char(char ch) {
    const char normalized = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    for (const GitChangeStatusDescriptor& entry : kDescriptors) {
        if (entry.ch == normalized) {
            return entry.status;
        }
    }
    return std::nullopt;
}

const char* git_change_status_label(GitChangeStatus status) {
    return descriptor(status).label;
}

Rgb git_change_status_color(GitChangeStatus status, GitChangeStatusColorDefaults defaults) {
    switch (status) {
    case GitChangeStatus::Modified:
        return defaults.modified;
    case GitChangeStatus::Added:
    case GitChangeStatus::Copied:
        return defaults.added;
    case GitChangeStatus::Deleted:
        return defaults.deleted;
    case GitChangeStatus::Renamed:
        return defaults.text;
    case GitChangeStatus::Untracked:
    case GitChangeStatus::Unmerged:
        return defaults.text;
    case GitChangeStatus::Ignored:
        return defaults.muted;
    case GitChangeStatus::Unknown:
        return defaults.muted;
    }
    return defaults.muted;
}

} // namespace tuinator
