#include <tuinator/render/git_change_status.hpp>

#include <cstring>

#include "test_harness.hpp"

TUINATOR_TEST(git_change_status_char_maps_porcelain_codes) {
    TUINATOR_CHECK_EQ(tuinator::git_change_status_char(tuinator::GitChangeStatus::Modified), 'M');
    TUINATOR_CHECK_EQ(tuinator::git_change_status_char(tuinator::GitChangeStatus::Added), 'A');
    TUINATOR_CHECK_EQ(tuinator::git_change_status_char(tuinator::GitChangeStatus::Deleted), 'D');
    TUINATOR_CHECK_EQ(tuinator::git_change_status_char(tuinator::GitChangeStatus::Renamed), 'R');
    TUINATOR_CHECK_EQ(tuinator::git_change_status_char(tuinator::GitChangeStatus::Copied), 'C');
    TUINATOR_CHECK_EQ(tuinator::git_change_status_char(tuinator::GitChangeStatus::Untracked), '?');
    TUINATOR_CHECK_EQ(tuinator::git_change_status_char(tuinator::GitChangeStatus::Ignored), '!');
    TUINATOR_CHECK_EQ(tuinator::git_change_status_char(tuinator::GitChangeStatus::Unmerged), 'U');
}

TUINATOR_TEST(git_change_status_from_char_round_trip) {
    TUINATOR_CHECK_EQ(tuinator::git_change_status_from_char('M'), tuinator::GitChangeStatus::Modified);
    TUINATOR_CHECK_EQ(tuinator::git_change_status_from_char('a'), tuinator::GitChangeStatus::Added);
    TUINATOR_CHECK_EQ(tuinator::git_change_status_from_char('?'), tuinator::GitChangeStatus::Untracked);
    TUINATOR_CHECK(!tuinator::git_change_status_from_char('x').has_value());
}

TUINATOR_TEST(git_change_status_label) {
    TUINATOR_CHECK(std::strcmp(tuinator::git_change_status_label(tuinator::GitChangeStatus::Modified), "Modified") ==
                   0);
    TUINATOR_CHECK(std::strcmp(tuinator::git_change_status_label(tuinator::GitChangeStatus::Untracked), "Untracked") ==
                   0);
}

TUINATOR_TEST(git_change_status_color_uses_defaults) {
    const tuinator::Rgb modified = tuinator::git_change_status_color(tuinator::GitChangeStatus::Modified);
    TUINATOR_CHECK_EQ(modified.r, 0xe0);
    TUINATOR_CHECK_EQ(modified.g, 0xaf);
    TUINATOR_CHECK_EQ(modified.b, 0x68);

    const tuinator::Rgb added = tuinator::git_change_status_color(tuinator::GitChangeStatus::Added);
    TUINATOR_CHECK_EQ(added.r, 0x9e);
    TUINATOR_CHECK_EQ(added.g, 0xce);
    TUINATOR_CHECK_EQ(added.b, 0x6a);

    const tuinator::Rgb deleted = tuinator::git_change_status_color(tuinator::GitChangeStatus::Deleted);
    TUINATOR_CHECK_EQ(deleted.r, 0xf7);
    TUINATOR_CHECK_EQ(deleted.g, 0x76);
    TUINATOR_CHECK_EQ(deleted.b, 0x8e);
}

TUINATOR_TEST(git_change_status_color_respects_overrides) {
    tuinator::GitChangeStatusColorDefaults colors;
    colors.modified = tuinator::Rgb{1, 2, 3};
    const tuinator::Rgb modified = tuinator::git_change_status_color(tuinator::GitChangeStatus::Modified, colors);
    TUINATOR_CHECK_EQ(modified.r, 1);
    TUINATOR_CHECK_EQ(modified.g, 2);
    TUINATOR_CHECK_EQ(modified.b, 3);
}
