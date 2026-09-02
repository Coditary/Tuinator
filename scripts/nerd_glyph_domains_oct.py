#!/usr/bin/env python3
"""Semantic domain routing for Nerd Fonts Octicons (i_oct.sh, v3.4.0)."""

from __future__ import annotations

from nerd_font_lib import kebab_case, pascal_case
from nerd_glyph_domains_editor_shared import (
    SHARED_EDITOR_CATEGORY_ENUM,
    SHARED_EDITOR_CPP_STEM,
    SHARED_EDITOR_DOMAINS,
    SHARED_EDITOR_ENUM,
    SHARED_EDITOR_PATH_PREFIX,
    shared_editor_domain,
)

OCT_DOMAIN_ORDER = [
    "GitHubPlatform",
    "DevMisc",
    "SourceControl",
    "EditorLayout",
    "EditorFile",
    "EditorTerminal",
    "EditorNavigation",
    "EditorStatus",
    "EditorChrome",
    "EditorEdit",
    "EditorFormat",
    "EditorComment",
    "EditorTesting",
    "EditorRemote",
    *SHARED_EDITOR_DOMAINS,
    "EditorMisc",
]

OCT_DOMAIN_PATH_PREFIX = {
    "GitHubPlatform": "github-platform",
    "DevMisc": "dev-misc",
    "SourceControl": "source-control",
    "EditorLayout": "editor-layout",
    "EditorFile": "editor-file",
    "EditorTerminal": "editor-terminal",
    "EditorNavigation": "editor-navigation",
    "EditorStatus": "editor-status",
    "EditorChrome": "editor-chrome",
    "EditorEdit": "editor-edit",
    "EditorFormat": "editor-format",
    "EditorComment": "editor-comment",
    "EditorTesting": "editor-testing",
    "EditorRemote": "editor-remote",
    **SHARED_EDITOR_PATH_PREFIX,
    "EditorMisc": "editor-misc",
}

OCT_DOMAIN_CPP_STEM = {
    "GitHubPlatform": "github_platform_icon",
    "DevMisc": "dev_misc_icon",
    "SourceControl": "source_control_icon",
    "EditorLayout": "editor_layout_icon",
    "EditorFile": "editor_file_icon",
    "EditorTerminal": "editor_terminal_icon",
    "EditorNavigation": "editor_navigation_icon",
    "EditorStatus": "editor_status_icon",
    "EditorChrome": "editor_chrome_icon",
    "EditorEdit": "editor_edit_icon",
    "EditorFormat": "editor_format_icon",
    "EditorComment": "editor_comment_icon",
    "EditorTesting": "editor_testing_icon",
    "EditorRemote": "editor_remote_icon",
    **SHARED_EDITOR_CPP_STEM,
    "EditorMisc": "editor_misc_icon",
}

OCT_DOMAIN_ENUM = {
    "GitHubPlatform": "GitHubPlatformIcon",
    "DevMisc": "DevMiscIcon",
    "SourceControl": "SourceControlIcon",
    "EditorLayout": "EditorLayoutIcon",
    "EditorFile": "EditorFileIcon",
    "EditorTerminal": "EditorTerminalIcon",
    "EditorNavigation": "EditorNavigationIcon",
    "EditorStatus": "EditorStatusIcon",
    "EditorChrome": "EditorChromeIcon",
    "EditorEdit": "EditorEditIcon",
    "EditorFormat": "EditorFormatIcon",
    "EditorComment": "EditorCommentIcon",
    "EditorTesting": "EditorTestingIcon",
    "EditorRemote": "EditorRemoteIcon",
    **SHARED_EDITOR_ENUM,
    "EditorMisc": "EditorMiscIcon",
}

OCT_DOMAIN_CATEGORY_ENUM = {
    "GitHubPlatform": "GitHubPlatformIconCategory",
    "DevMisc": "DevMiscIconCategory",
    "SourceControl": "SourceControlIconCategory",
    "EditorLayout": "EditorLayoutIconCategory",
    "EditorFile": "EditorFileIconCategory",
    "EditorTerminal": "EditorTerminalIconCategory",
    "EditorNavigation": "EditorNavigationIconCategory",
    "EditorStatus": "EditorStatusIconCategory",
    "EditorChrome": "EditorChromeIconCategory",
    "EditorEdit": "EditorEditIconCategory",
    "EditorFormat": "EditorFormatIconCategory",
    "EditorComment": "EditorCommentIconCategory",
    "EditorTesting": "EditorTestingIconCategory",
    "EditorRemote": "EditorRemoteIconCategory",
    **SHARED_EDITOR_CATEGORY_ENUM,
    "EditorMisc": "EditorMiscIconCategory",
}

SOURCE_CONTROL_GIT = {
    "git_branch",
    "git_commit",
    "git_compare",
    "git_merge",
    "git_merge_queue",
    "git_pull_request",
    "git_pull_request_closed",
    "git_pull_request_draft",
    "commit",
}
SOURCE_CONTROL_DIFF = {
    "diff",
    "diff_added",
    "diff_ignored",
    "diff_modified",
    "diff_removed",
    "diff_renamed",
    "file_diff",
}
SOURCE_CONTROL_REPO = {
    "repo",
    "repo_clone",
    "repo_deleted",
    "repo_forked",
    "repo_locked",
    "repo_pull",
    "repo_push",
    "repo_template",
    "feed_forked",
    "feed_merged",
    "mirror",
    "tag",
}
SOURCE_CONTROL_REVIEW = {
    "code_review",
    "codescan",
    "codescan_checkmark",
    "cross_reference",
    "rel_file_path",
}
SOURCE_CONTROL_SYNC = {
    "sync",
    "versions",
    "history",
}

GITHUB_PLATFORM_ISSUE = {
    "issue_closed",
    "issue_draft",
    "issue_opened",
    "issue_reopened",
    "issue_tracked_by",
    "issue_tracks",
}
GITHUB_PLATFORM_DISCUSSION = {
    "discussion_closed",
    "discussion_duplicate",
    "discussion_outdated",
    "comment_discussion",
}
GITHUB_PLATFORM_FEED = {
    "feed_discussion",
    "feed_repo",
    "feed_person",
    "feed_rocket",
    "feed_star",
    "feed_tag",
    "feed_trophy",
}
GITHUB_PLATFORM_COPILOT = {
    "copilot",
    "copilot_error",
    "copilot_warning",
}

DEV_MISC_PROJECT = {
    "project",
    "project_roadmap",
    "project_symlink",
    "project_template",
    "goal",
    "north_star",
    "milestone",
}
DEV_MISC_PEOPLE = {
    "organization",
    "people",
    "person",
    "person_add",
    "person_fill",
    "id_badge",
}
DEV_MISC_BRAND = {
    "logo_github",
    "mark_github",
    "logo_gist",
    "hubot",
    "squirrel",
    "ruby",
}
DEV_MISC_WORKFLOW = {
    "workflow",
    "webhook",
}
DEV_MISC_SPONSOR = {
    "fiscal_host",
    "sponsor_tiers",
    "gift",
    "credit_card",
}
DEV_MISC_POLICY = {
    "code_of_conduct",
    "law",
}
DEV_MISC_TOOLING = {
    "codespaces",
    "dependabot",
}

EDITOR_EDIT = {
    "copy",
    "paste",
    "duplicate",
    "trash",
    "pencil",
    "plus",
    "plus_circle",
    "dash",
    "paperclip",
}
EDITOR_FORMAT_TEXT = {
    "bold",
    "italic",
    "strikethrough",
    "heading",
    "horizontal_rule",
    "markdown",
    "quote",
    "typography",
    "hash",
    "number",
}
EDITOR_FORMAT_LIST = {
    "list_ordered",
    "list_unordered",
}
EDITOR_FORMAT_CODE = {
    "code",
    "code_square",
}
EDITOR_COMMENT = {
    "comment",
    "reply",
}
EDITOR_TESTING = {
    "beaker",
    "bug",
}
EDITOR_REMOTE = {
    "cloud",
    "cloud_offline",
    "download",
    "upload",
    "desktop_download",
    "server",
    "database",
    "package",
    "package_dependencies",
    "package_dependents",
    "container",
    "cpu",
    "plug",
}

EDITOR_CHROME_SETTINGS = {
    "gear",
    "sliders",
    "paintbrush",
}
EDITOR_CHROME_UI = {
    "ellipsis",
    "kebab_horizontal",
    "three_bars",
    "grabber",
    "columns",
    "rows",
    "multi_select",
    "single_select",
}
EDITOR_CHROME_SEARCH = {
    "search",
    "filter",
    "command_palette",
}
EDITOR_CHROME_BOOKMARK = {
    "bookmark",
    "bookmark_fill",
    "bookmark_slash",
    "bookmark_slash_fill",
}
EDITOR_CHROME_SECURITY = {
    "lock",
    "unlock",
    "key",
    "key_asterisk",
    "shield",
    "shield_check",
    "shield_lock",
    "shield_slash",
    "shield_x",
}
EDITOR_CHROME_PREVIEW = {
    "eye",
    "eye_closed",
    "read",
    "sparkle_fill",
    "telescope",
    "telescope_fill",
}
EDITOR_CHROME_MEDIA = {
    "play",
    "stop",
    "video",
    "broadcast",
    "mute",
    "unmute",
    "image",
    "device_camera",
    "device_camera_video",
    "device_desktop",
    "device_mobile",
}
EDITOR_CHROME_USER = {
    "sign_in",
    "sign_out",
}

SOCIAL_HEART = {"heart", "heart_fill", "feed_heart"}
SOCIAL_THUMB = {"thumbsup", "thumbsdown"}
SOCIAL_STAR = {"star", "star_fill"}
SOCIAL_FACE = {"smiley", "mention"}

SOURCE_CONTROL_CATEGORY_ORDER = ["Git", "Diff", "Repo", "Review", "Sync"]
GITHUB_PLATFORM_CATEGORY_ORDER = ["Issue", "Discussion", "Feed", "Copilot"]
DEV_MISC_CATEGORY_ORDER = [
    "Project",
    "People",
    "Brand",
    "Workflow",
    "Sponsor",
    "Policy",
    "Tooling",
    "Science",
    "Chart",
    "Shape",
    "Accessibility",
    "Misc",
]
EDITOR_LAYOUT_CATEGORY_ORDER = ["Window"]
EDITOR_FILE_CATEGORY_ORDER = ["File", "Folder"]
EDITOR_TERMINAL_CATEGORY_ORDER = ["Shell"]
EDITOR_NAVIGATION_CATEGORY_ORDER = [
    "Arrow",
    "Chevron",
    "Triangle",
    "Move",
    "Fold",
    "Tab",
    "Sidebar",
    "Sort",
    "Location",
]
EDITOR_STATUS_CATEGORY_ORDER = [
    "Alert",
    "Check",
    "Info",
    "Bell",
    "Block",
    "Close",
    "Verified",
]
EDITOR_CHROME_CATEGORY_ORDER = [
    "Window",
    "Settings",
    "User",
    "Security",
    "Search",
    "UI",
    "Bookmark",
    "Pin",
    "Preview",
    "Media",
    "Zoom",
]
EDITOR_EDIT_CATEGORY_ORDER = ["Edit", "Transform"]
EDITOR_FORMAT_CATEGORY_ORDER = ["Style", "List", "Language"]
EDITOR_COMMENT_CATEGORY_ORDER = ["Comment", "Reply"]
EDITOR_TESTING_CATEGORY_ORDER = ["Lab", "Debug"]
EDITOR_REMOTE_CATEGORY_ORDER = ["Cloud", "Server", "Download", "Package", "Device"]
EDITOR_MISC_CATEGORY_ORDER = [
    "Communication",
    "Time",
    "Home",
    "Link",
    "Tool",
    "Misc",
]
SOCIAL_REACTION_CATEGORY_ORDER = ["Heart", "Thumb", "Star", "Face", "Mention"]


def oct_domain_for(suffix: str) -> str:
    shared = shared_editor_domain(suffix)
    if shared is not None:
        return shared

    if suffix in SOCIAL_HEART or suffix in SOCIAL_THUMB or suffix in SOCIAL_STAR or suffix in SOCIAL_FACE:
        return "EditorSocial"

    if (
        suffix.startswith("git_")
        or suffix in SOURCE_CONTROL_GIT
        or suffix in SOURCE_CONTROL_DIFF
        or suffix in SOURCE_CONTROL_REPO
        or suffix in SOURCE_CONTROL_REVIEW
        or suffix in SOURCE_CONTROL_SYNC
    ):
        return "SourceControl"

    if (
        suffix.startswith("issue_")
        or suffix in GITHUB_PLATFORM_ISSUE
        or suffix.startswith("discussion_")
        or suffix in GITHUB_PLATFORM_DISCUSSION
        or suffix in GITHUB_PLATFORM_FEED
        or suffix in GITHUB_PLATFORM_COPILOT
    ):
        return "GitHubPlatform"

    if (
        suffix in DEV_MISC_PROJECT
        or suffix in DEV_MISC_PEOPLE
        or suffix in DEV_MISC_BRAND
        or suffix in DEV_MISC_WORKFLOW
        or suffix in DEV_MISC_SPONSOR
        or suffix in DEV_MISC_POLICY
        or suffix in DEV_MISC_TOOLING
        or suffix.startswith("accessibility")
    ):
        return "DevMisc"

    if suffix.startswith("screen_"):
        return "EditorLayout"

    if suffix.startswith("file"):
        return "EditorFile"

    if suffix == "terminal":
        return "EditorTerminal"

    if suffix in EDITOR_EDIT:
        return "EditorEdit"

    if suffix in EDITOR_FORMAT_TEXT or suffix in EDITOR_FORMAT_LIST or suffix in EDITOR_FORMAT_CODE:
        return "EditorFormat"

    if suffix in EDITOR_COMMENT or suffix.startswith("comment"):
        return "EditorComment"

    if suffix in EDITOR_TESTING:
        return "EditorTesting"

    if suffix in EDITOR_REMOTE or suffix.startswith("package") or suffix.startswith("cloud"):
        return "EditorRemote"

    if suffix.startswith("arrow_") or suffix.startswith("chevron_") or suffix.startswith("triangle_"):
        return "EditorNavigation"
    if suffix.startswith("move_to_"):
        return "EditorNavigation"
    if suffix in {"fold", "fold_down", "fold_up", "unfold", "location", "sort_asc", "sort_desc", "skip", "skip_fill"}:
        return "EditorNavigation"
    if suffix.startswith("sidebar_") or suffix.startswith("tab"):
        return "EditorNavigation"

    if (
        suffix.startswith("alert")
        or suffix.startswith("check")
        or suffix == "checkbox"
        or suffix in {"info", "question", "report", "verified", "unverified", "passkey_fill"}
        or suffix in {"blocked", "no_entry"}
        or suffix.startswith("bell")
        or suffix == "unread"
        or suffix.startswith("dot")
        or suffix.startswith("x")
    ):
        return "EditorStatus"

    if suffix in EDITOR_CHROME_USER or suffix.startswith("sign_"):
        return "EditorChrome"
    if suffix in EDITOR_CHROME_SETTINGS or suffix.startswith("settings"):
        return "EditorChrome"
    if suffix in EDITOR_CHROME_SEARCH:
        return "EditorChrome"
    if suffix in EDITOR_CHROME_BOOKMARK or suffix.startswith("bookmark"):
        return "EditorChrome"
    if suffix in EDITOR_CHROME_SECURITY or suffix.startswith("lock") or suffix.startswith("key") or suffix.startswith("shield"):
        return "EditorChrome"
    if suffix in EDITOR_CHROME_UI:
        return "EditorChrome"
    if suffix in EDITOR_CHROME_PREVIEW or suffix.startswith("eye"):
        return "EditorChrome"
    if suffix in EDITOR_CHROME_MEDIA or suffix.startswith("device_"):
        return "EditorChrome"
    if suffix == "pin" or suffix.startswith("pin"):
        return "EditorChrome"
    if suffix.startswith("zoom_"):
        return "EditorChrome"

    if (
        suffix.startswith("link")
        or suffix == "unlink"
        or suffix.startswith("calendar")
        or suffix.startswith("clock")
        or suffix in {"stopwatch", "hourglass"}
        or suffix.startswith("home")
        or suffix in {"inbox", "mail"}
        or suffix in {"share", "share_android", "rss", "paper_airplane"}
        or suffix in {"table", "tasklist", "checklist"}
        or suffix == "tools"
        or suffix in {"briefcase", "rocket", "zap", "flame", "pulse", "megaphone", "light_bulb"}
        or suffix in {"archive", "book", "mortar_board", "diamond", "trophy"}
        or suffix.startswith("square")
        or suffix.startswith("circle")
        or suffix == "globe"
        or suffix == "cache"
        or suffix == "log"
        or suffix == "iterations"
        or suffix == "graph"
        or suffix in {"moon", "sun", "infinity", "tag", "note"}
        or suffix in {"apps", "browser"}
        or suffix == "stack"
    ):
        return "EditorMisc"

    return "EditorMisc"


def oct_category_for(domain: str, suffix: str) -> str:
    if domain == "SourceControl":
        if suffix.startswith("git_") or suffix in SOURCE_CONTROL_GIT:
            return "Git"
        if suffix in SOURCE_CONTROL_DIFF:
            return "Diff"
        if suffix in SOURCE_CONTROL_REPO:
            return "Repo"
        if suffix in SOURCE_CONTROL_REVIEW:
            return "Review"
        return "Sync"

    if domain == "GitHubPlatform":
        if suffix in GITHUB_PLATFORM_ISSUE:
            return "Issue"
        if suffix in GITHUB_PLATFORM_DISCUSSION:
            return "Discussion"
        if suffix in GITHUB_PLATFORM_FEED:
            return "Feed"
        return "Copilot"

    if domain == "DevMisc":
        if suffix in DEV_MISC_PROJECT:
            return "Project"
        if suffix in DEV_MISC_PEOPLE:
            return "People"
        if suffix in DEV_MISC_BRAND:
            return "Brand"
        if suffix in DEV_MISC_WORKFLOW:
            return "Workflow"
        if suffix in DEV_MISC_SPONSOR:
            return "Sponsor"
        if suffix in DEV_MISC_POLICY:
            return "Policy"
        if suffix in DEV_MISC_TOOLING:
            return "Tooling"
        if suffix in {"beaker", "bug"}:
            return "Science"
        if suffix == "graph":
            return "Chart"
        if suffix.startswith("square") or suffix.startswith("circle") or suffix == "diamond":
            return "Shape"
        if suffix.startswith("accessibility"):
            return "Accessibility"
        return "Misc"

    if domain == "EditorSocial":
        if suffix.startswith("star_"):
            return "Star"
        if "heart" in suffix:
            return "Heart"
        if suffix.startswith("thumbs"):
            return "Thumb"
        return "Reaction"
    if domain == "EditorCommunication":
        if suffix.startswith("mail") or suffix == "inbox":
            return "Mail"
        return "Feed"
    if domain == "EditorShape":
        return "Shape"
    if domain == "EditorMedia":
        return "Device"
    if domain == "EditorChart":
        return "Chart"

    if domain == "EditorMisc":
        if suffix in SOCIAL_HEART:
            return "Heart"
        if suffix in SOCIAL_THUMB:
            return "Thumb"
        if suffix in SOCIAL_STAR:
            return "Star"
        if suffix in SOCIAL_FACE:
            return "Mention" if suffix == "mention" else "Face"
        return "Face"

    if domain == "EditorLayout":
        return "Window"

    if domain == "EditorFile":
        if suffix.startswith("file_directory"):
            return "Folder"
        return "File"

    if domain == "EditorTerminal":
        return "Shell"

    if domain == "EditorNavigation":
        if suffix.startswith("arrow_"):
            return "Arrow"
        if suffix.startswith("chevron_"):
            return "Chevron"
        if suffix.startswith("triangle_"):
            return "Triangle"
        if suffix.startswith("move_to_"):
            return "Move"
        if suffix in {"fold", "fold_down", "fold_up", "unfold"}:
            return "Fold"
        if suffix.startswith("tab"):
            return "Tab"
        if suffix.startswith("sidebar_"):
            return "Sidebar"
        if suffix in {"sort_asc", "sort_desc", "skip", "skip_fill"}:
            return "Sort"
        return "Location"

    if domain == "EditorStatus":
        if suffix.startswith("alert"):
            return "Alert"
        if suffix.startswith("check") or suffix == "checkbox":
            return "Check"
        if suffix in {"info", "question", "report"}:
            return "Info"
        if suffix.startswith("bell"):
            return "Bell"
        if suffix in {"blocked", "no_entry", "unread"} or suffix.startswith("dot"):
            return "Block"
        if suffix.startswith("x"):
            return "Close"
        if suffix in {"verified", "unverified", "passkey_fill"}:
            return "Verified"
        return "Alert"

    if domain == "EditorChrome":
        if suffix in EDITOR_CHROME_SETTINGS:
            return "Settings"
        if suffix in EDITOR_CHROME_USER or suffix.startswith("sign_"):
            return "User"
        if suffix in EDITOR_CHROME_SEARCH:
            return "Search"
        if suffix in EDITOR_CHROME_BOOKMARK:
            return "Bookmark"
        if suffix in EDITOR_CHROME_SECURITY or suffix.startswith("lock") or suffix.startswith("key") or suffix.startswith("shield"):
            return "Security"
        if suffix in EDITOR_CHROME_UI:
            return "UI"
        if suffix in EDITOR_CHROME_PREVIEW or suffix.startswith("eye"):
            return "Preview"
        if suffix in EDITOR_CHROME_MEDIA or suffix.startswith("device_"):
            return "Media"
        if suffix == "pin":
            return "Pin"
        if suffix.startswith("zoom_"):
            return "Zoom"
        return "UI"

    if domain == "EditorEdit":
        if suffix in {"copy", "paste", "duplicate", "trash", "pencil", "plus", "plus_circle", "dash"}:
            return "Edit"
        return "Transform"

    if domain == "EditorFormat":
        if suffix in EDITOR_FORMAT_LIST:
            return "List"
        if suffix in EDITOR_FORMAT_CODE:
            return "Language"
        return "Style"

    if domain == "EditorComment":
        if suffix == "reply":
            return "Reply"
        return "Comment"

    if domain == "EditorTesting":
        if suffix == "bug":
            return "Debug"
        return "Lab"

    if domain == "EditorRemote":
        if suffix.startswith("cloud"):
            return "Cloud"
        if suffix.startswith("server"):
            return "Server"
        if suffix in {"download", "upload", "desktop_download"}:
            return "Download"
        if suffix.startswith("package") or suffix in {"database", "container", "cpu"}:
            return "Package"
        if suffix == "plug":
            return "Device"
        return "Cloud"

    if suffix.startswith("link") or suffix == "unlink":
        return "Link"
    if suffix.startswith("clock") or suffix in {"stopwatch", "hourglass"} or suffix.startswith("calendar"):
        return "Time"
    if suffix.startswith("home"):
        return "Home"
    if suffix in {"inbox", "mail", "share", "share_android", "rss", "paper_airplane"}:
        return "Communication"
    if suffix == "tools":
        return "Tool"
    return "Misc"


def oct_path(domain: str, suffix: str) -> str:
    return f"{OCT_DOMAIN_PATH_PREFIX[domain]}-{kebab_case(suffix)}"


def oct_ascii(suffix: str) -> str:
    if suffix.startswith("git_") or suffix in {"commit", "repo", "sync", "tag"}:
        return "G"
    if suffix.startswith("diff") or suffix == "file_diff":
        return "D"
    if suffix.startswith("issue_"):
        return "I"
    if suffix.startswith("discussion") or suffix == "comment_discussion":
        return "d"
    if suffix.startswith("feed_"):
        return "f"
    if suffix.startswith("arrow_") or suffix.startswith("chevron_") or suffix.startswith("triangle_"):
        return "^"
    if suffix.startswith("check") or suffix == "verified":
        return "+"
    if suffix.startswith("alert") or suffix.startswith("x"):
        return "!"
    if suffix.startswith("bell"):
        return "B"
    if suffix in {"heart", "heart_fill", "feed_heart"}:
        return "<3"
    if suffix in {"thumbsup", "thumbsdown"}:
        return "T"
    if suffix in {"star", "star_fill"}:
        return "*"
    if suffix.startswith("file"):
        return "F"
    if suffix.startswith("repo") or suffix.startswith("project"):
        return "R"
    if suffix in {"logo_github", "mark_github"}:
        return "#"
    if suffix.startswith("copilot"):
        return "C"
    if suffix == "search":
        return "?"
    if suffix == "terminal":
        return "$"
    if suffix.startswith("eye"):
        return "o"
    if suffix.startswith("lock") or suffix == "unlock":
        return "#"
    if suffix in EDITOR_EDIT:
        return "e"
    return "."


def oct_row(suffix: str, codepoint: int) -> dict[str, str]:
    domain = oct_domain_for(suffix)
    category = oct_category_for(domain, suffix)
    return {
        "name": pascal_case(suffix),
        "domain": domain,
        "category": category,
        "nerd_suffix": suffix,
        "path": oct_path(domain, suffix),
        "codepoint": f"{codepoint:04X}",
        "ascii": oct_ascii(suffix),
        "source_set": "oct",
    }


def oct_category_order_for(domain: str) -> list[str]:
    return {
        "GitHubPlatform": GITHUB_PLATFORM_CATEGORY_ORDER,
        "DevMisc": DEV_MISC_CATEGORY_ORDER,
        "SourceControl": SOURCE_CONTROL_CATEGORY_ORDER,
        "EditorLayout": EDITOR_LAYOUT_CATEGORY_ORDER,
        "EditorFile": EDITOR_FILE_CATEGORY_ORDER,
        "EditorTerminal": EDITOR_TERMINAL_CATEGORY_ORDER,
        "EditorNavigation": EDITOR_NAVIGATION_CATEGORY_ORDER,
        "EditorStatus": EDITOR_STATUS_CATEGORY_ORDER,
        "EditorChrome": EDITOR_CHROME_CATEGORY_ORDER,
        "EditorEdit": EDITOR_EDIT_CATEGORY_ORDER,
        "EditorFormat": EDITOR_FORMAT_CATEGORY_ORDER,
        "EditorComment": EDITOR_COMMENT_CATEGORY_ORDER,
        "EditorTesting": EDITOR_TESTING_CATEGORY_ORDER,
        "EditorRemote": EDITOR_REMOTE_CATEGORY_ORDER,
        "EditorSocial": ["Star", "Heart", "Thumb", "Reaction"],
        "EditorCommunication": ["Mail", "Feed"],
        "EditorShape": ["Shape"],
        "EditorMedia": ["Device"],
        "EditorChart": ["Chart"],
        "EditorMisc": EDITOR_MISC_CATEGORY_ORDER,
    }.get(domain, [])


def oct_category_path_for(domain: str, category: str) -> str:
    prefix = OCT_DOMAIN_PATH_PREFIX[domain]
    flat = {
        "Git",
        "Diff",
        "Repo",
        "Review",
        "Sync",
        "Issue",
        "Discussion",
        "Feed",
        "Copilot",
        "Project",
        "People",
        "Brand",
        "Workflow",
        "Sponsor",
        "Policy",
        "Tooling",
        "Window",
        "File",
        "Folder",
        "Shell",
        "Arrow",
        "Chevron",
        "Triangle",
        "Move",
        "Fold",
        "Tab",
        "Sidebar",
        "Sort",
        "Location",
        "Alert",
        "Check",
        "Info",
        "Bell",
        "Block",
        "Close",
        "Verified",
        "Settings",
        "User",
        "Security",
        "Search",
        "UI",
        "Bookmark",
        "Pin",
        "Preview",
        "Media",
        "Zoom",
        "Edit",
        "Transform",
        "Style",
        "List",
        "Language",
        "Comment",
        "Reply",
        "Lab",
        "Debug",
        "Cloud",
        "Server",
        "Download",
        "Package",
        "Device",
        "Communication",
        "Time",
        "Home",
        "Link",
        "Tool",
        "Heart",
        "Thumb",
        "Star",
        "Face",
        "Mention",
        "Science",
        "Chart",
        "Shape",
        "Accessibility",
        "Misc",
    }
    if category in flat:
        if category in {
            "Git",
            "Issue",
            "Arrow",
            "Alert",
            "Window",
            "File",
            "Shell",
            "Heart",
            "Science",
            "Misc",
        }:
            return prefix
        return f"{prefix}-{kebab_case(category)}"
    return prefix


OCT_REGISTRY = {
    "order": OCT_DOMAIN_ORDER,
    "path_prefix": OCT_DOMAIN_PATH_PREFIX,
    "cpp_stem": OCT_DOMAIN_CPP_STEM,
    "enum": OCT_DOMAIN_ENUM,
    "category_enum": OCT_DOMAIN_CATEGORY_ENUM,
    "category_order": oct_category_order_for,
    "category_path": oct_category_path_for,
}
