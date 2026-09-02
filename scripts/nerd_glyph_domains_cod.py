#!/usr/bin/env python3
"""Semantic domain routing for Nerd Fonts Codicons (i_cod.sh, v3.4.0)."""

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

COD_DOMAIN_ORDER = [
    "SourceControl",
    "EditorDebug",
    "EditorLayout",
    "EditorFile",
    "EditorTerminal",
    "EditorStatus",
    "EditorNavigation",
    "EditorChrome",
    "EditorEdit",
    "EditorFormat",
    "EditorSymbol",
    "EditorComment",
    "EditorTesting",
    "EditorRemote",
    *SHARED_EDITOR_DOMAINS,
    "EditorMisc",
]

COD_DOMAIN_PATH_PREFIX = {
    "SourceControl": "source-control",
    "EditorDebug": "editor-debug",
    "EditorLayout": "editor-layout",
    "EditorFile": "editor-file",
    "EditorTerminal": "editor-terminal",
    "EditorStatus": "editor-status",
    "EditorNavigation": "editor-navigation",
    "EditorChrome": "editor-chrome",
    "EditorEdit": "editor-edit",
    "EditorFormat": "editor-format",
    "EditorSymbol": "editor-symbol",
    "EditorComment": "editor-comment",
    "EditorTesting": "editor-testing",
    "EditorRemote": "editor-remote",
    **SHARED_EDITOR_PATH_PREFIX,
    "EditorMisc": "editor-misc",
}

COD_DOMAIN_CPP_STEM = {
    "SourceControl": "source_control_icon",
    "EditorDebug": "editor_debug_icon",
    "EditorLayout": "editor_layout_icon",
    "EditorFile": "editor_file_icon",
    "EditorTerminal": "editor_terminal_icon",
    "EditorStatus": "editor_status_icon",
    "EditorNavigation": "editor_navigation_icon",
    "EditorChrome": "editor_chrome_icon",
    "EditorEdit": "editor_edit_icon",
    "EditorFormat": "editor_format_icon",
    "EditorSymbol": "editor_symbol_icon",
    "EditorComment": "editor_comment_icon",
    "EditorTesting": "editor_testing_icon",
    "EditorRemote": "editor_remote_icon",
    **SHARED_EDITOR_CPP_STEM,
    "EditorMisc": "editor_misc_icon",
}

COD_DOMAIN_ENUM = {
    "SourceControl": "SourceControlIcon",
    "EditorDebug": "EditorDebugIcon",
    "EditorLayout": "EditorLayoutIcon",
    "EditorFile": "EditorFileIcon",
    "EditorTerminal": "EditorTerminalIcon",
    "EditorStatus": "EditorStatusIcon",
    "EditorNavigation": "EditorNavigationIcon",
    "EditorChrome": "EditorChromeIcon",
    "EditorEdit": "EditorEditIcon",
    "EditorFormat": "EditorFormatIcon",
    "EditorSymbol": "EditorSymbolIcon",
    "EditorComment": "EditorCommentIcon",
    "EditorTesting": "EditorTestingIcon",
    "EditorRemote": "EditorRemoteIcon",
    **SHARED_EDITOR_ENUM,
    "EditorMisc": "EditorMiscIcon",
}

COD_DOMAIN_CATEGORY_ENUM = {
    "SourceControl": "SourceControlIconCategory",
    "EditorDebug": "EditorDebugIconCategory",
    "EditorLayout": "EditorLayoutIconCategory",
    "EditorFile": "EditorFileIconCategory",
    "EditorTerminal": "EditorTerminalIconCategory",
    "EditorStatus": "EditorStatusIconCategory",
    "EditorNavigation": "EditorNavigationIconCategory",
    "EditorChrome": "EditorChromeIconCategory",
    "EditorEdit": "EditorEditIconCategory",
    "EditorFormat": "EditorFormatIconCategory",
    "EditorSymbol": "EditorSymbolIconCategory",
    "EditorComment": "EditorCommentIconCategory",
    "EditorTesting": "EditorTestingIconCategory",
    "EditorRemote": "EditorRemoteIconCategory",
    **SHARED_EDITOR_CATEGORY_ENUM,
    "EditorMisc": "EditorMiscIconCategory",
}

SOURCE_CONTROL_GIT = {
    "merge",
    "source_control",
}
SOURCE_CONTROL_REVIEW = {
    "request_changes",
}
SOURCE_CONTROL_REMOTE = {
    "discard",
}

EDITOR_DEBUG_RUN = {
    "stop_circle",
    "play",
    "play_circle",
    "activate_breakpoints",
}
EDITOR_DEBUG_WATCH = {
    "watch",
    "variable_group",
}
EDITOR_DEBUG_TEST = {
    "bug",
    "inspect",
}
EDITOR_DEBUG_GENERAL = {
    "debug",
    "debug_all",
    "debug_alt",
    "debug_alt_small",
}

EDITOR_LAYOUT_WINDOW = {
    "collapse_all",
    "expand_all",
    "empty_window",
    "window",
    "multiple_windows",
}
EDITOR_LAYOUT_VIEW = {
    "editor_layout",
}

EDITOR_FILE_NEW = {
    "new_file",
    "new_folder",
}
EDITOR_FILE_SAVE = {
    "export",
}
EDITOR_FILE_DELETE = {
    "trash",
}

EDITOR_STATUS_INFO = {
    "info",
    "feedback",
    "question",
}
EDITOR_STATUS_WARNING = {
    "warning",
}
EDITOR_STATUS_ERROR = {
    "report",
    "bracket_error",
}

EDITOR_NAV_LOCATION = {
    "go_to_file",
    "location",
    "move",
}

EDITOR_CHROME_WINDOW = {
    "close_all",
}
EDITOR_CHROME_SETTINGS = {
    "gear",
}
EDITOR_CHROME_USER = {
    "account",
    "organization",
}
EDITOR_CHROME_UI = {
    "menu",
    "three_bars",
    "ellipsis",
    "kebab_vertical",
    "gripper",
    "grabber",
}
EDITOR_CHROME_SEARCH = {
    "bookmark",
}
EDITOR_CHROME_PREVIEW = {
    "preview",
    "open_preview",
}
EDITOR_CHROME_EXTENSIONS = {
    "extensions",
    "color_mode",
    "dashboard",
}
EDITOR_CHROME_SECURITY = {
    "shield",
}
EDITOR_CHROME_AUDIO = {
    "mute",
    "unmute",
}

EDITOR_EDIT = {
    "add",
    "remove",
    "edit",
    "copy",
    "replace",
    "replace_all",
    "clear_all",
    "redo",
    "insert",
    "exclude",
    "combine",
    "clippy",
}
EDITOR_FORMAT_TEXT = {
    "bold",
    "italic",
    "text_size",
    "whole_word",
    "whitespace",
    "word_wrap",
    "preserve_case",
    "case_sensitive",
    "newline",
    "no_newline",
    "indent",
    "horizontal_rule",
    "quote",
    "markdown",
    "json",
    "code",
    "regex",
}
EDITOR_FORMAT_FOLD = {
    "fold",
    "fold_down",
    "fold_up",
    "unfold",
}
EDITOR_FORMAT_LIST = {
    "list_ordered",
    "list_unordered",
    "list_filter",
    "list_flat",
    "list_selection",
    "list_tree",
    "sort_precedence",
}
EDITOR_SYMBOL = {
    "type_hierarchy",
    "type_hierarchy_sub",
    "type_hierarchy_super",
    "group_by_ref_type",
    "ungroup_by_ref_type",
    "references",
}
EDITOR_COMMENT = {
    "comment",
    "comment_discussion",
    "comment_draft",
    "comment_unresolved",
    "reply",
}
EDITOR_TESTING = {
    "beaker",
    "beaker_stop",
}
EDITOR_REMOTE = {
    "cloud",
    "cloud_download",
    "cloud_upload",
    "azure",
    "azure_devops",
    "server",
    "server_environment",
    "server_process",
    "database",
    "package",
    "desktop_download",
}

SOURCE_CONTROL_CATEGORY_ORDER = ["Git", "Diff", "Remote", "Review"]
EDITOR_DEBUG_CATEGORY_ORDER = [
    "Breakpoint",
    "Run",
    "Control",
    "Stack",
    "Console",
    "Coverage",
    "Watch",
    "Test",
    "General",
]
EDITOR_LAYOUT_CATEGORY_ORDER = ["Layout", "Panel", "Sidebar", "Split", "Window", "View"]
EDITOR_FILE_CATEGORY_ORDER = ["File", "Folder", "New", "Save", "Notebook", "Delete"]
EDITOR_TERMINAL_CATEGORY_ORDER = ["Shell"]
EDITOR_STATUS_CATEGORY_ORDER = ["Info", "Warning", "Error", "Success", "Alert"]
EDITOR_NAVIGATION_CATEGORY_ORDER = ["Arrow", "Chevron", "Triangle", "Location"]
EDITOR_CHROME_CATEGORY_ORDER = [
    "Window",
    "Settings",
    "User",
    "Visibility",
    "Security",
    "UI",
    "Search",
    "Filter",
    "Pin",
    "Preview",
    "Extensions",
    "Theme",
    "Dashboard",
    "Audio",
    "Zoom",
]
EDITOR_EDIT_CATEGORY_ORDER = ["Edit", "Transform"]
EDITOR_FORMAT_CATEGORY_ORDER = ["Style", "Fold", "List", "Language"]
EDITOR_SYMBOL_CATEGORY_ORDER = ["Hierarchy", "Group", "Reference"]
EDITOR_COMMENT_CATEGORY_ORDER = ["Comment", "Reply"]
EDITOR_TESTING_CATEGORY_ORDER = ["Lab"]
EDITOR_REMOTE_CATEGORY_ORDER = ["Cloud", "Server", "Download", "Package"]
EDITOR_MISC_CATEGORY_ORDER = [
    "Communication",
    "Media",
    "Collaboration",
    "Issue",
    "History",
    "Hint",
    "Social",
    "Tool",
    "VM",
    "Bot",
    "Project",
    "Layer",
    "Shape",
    "Link",
    "Chart",
    "Misc",
]


def cod_domain_for(suffix: str) -> str:
    if suffix.startswith("git_") or suffix in SOURCE_CONTROL_GIT:
        return "SourceControl"
    if suffix.startswith("diff"):
        return "SourceControl"
    if suffix.startswith("repo"):
        return "SourceControl"
    if suffix in SOURCE_CONTROL_REVIEW or suffix in SOURCE_CONTROL_REMOTE:
        return "SourceControl"
    if suffix.startswith("github"):
        return "SourceControl"
    if suffix.startswith("remote"):
        return "SourceControl"
    if suffix.startswith("sync"):
        return "SourceControl"

    if suffix in EDITOR_TESTING:
        return "EditorTesting"
    if suffix.startswith("symbol_") or suffix == "bracket_dot":
        return "EditorSymbol"

    if suffix.startswith("debug"):
        return "EditorDebug"
    if suffix.startswith("run_") or suffix in EDITOR_DEBUG_RUN:
        return "EditorDebug"
    if suffix in EDITOR_DEBUG_WATCH:
        return "EditorDebug"
    if suffix in EDITOR_DEBUG_TEST:
        return "EditorDebug"

    if suffix.startswith("terminal"):
        return "EditorTerminal"

    if suffix.startswith("layout") or suffix in EDITOR_LAYOUT_WINDOW or suffix in EDITOR_LAYOUT_VIEW:
        return "EditorLayout"
    if suffix.startswith("split_"):
        return "EditorLayout"
    if suffix.startswith("screen_"):
        return "EditorLayout"

    if suffix.startswith("file") or suffix == "files":
        return "EditorFile"
    if suffix.startswith("folder") or suffix.startswith("root_folder"):
        return "EditorFile"
    if suffix in EDITOR_FILE_NEW or suffix.startswith("save") or suffix.startswith("notebook"):
        return "EditorFile"
    if suffix in EDITOR_FILE_SAVE or suffix in EDITOR_FILE_DELETE:
        return "EditorFile"

    if suffix in EDITOR_STATUS_INFO or suffix in EDITOR_STATUS_WARNING:
        return "EditorStatus"
    if suffix.startswith("error") or suffix in EDITOR_STATUS_ERROR:
        return "EditorStatus"
    if suffix.startswith("check"):
        return "EditorStatus"
    if suffix.startswith("verified") or suffix == "unverified":
        return "EditorStatus"
    if suffix.startswith("bell"):
        return "EditorStatus"
    if suffix.startswith("pass"):
        return "EditorStatus"

    if suffix.startswith("arrow_"):
        return "EditorNavigation"
    if suffix.startswith("chevron_"):
        return "EditorNavigation"
    if suffix.startswith("triangle_"):
        return "EditorNavigation"
    if suffix in EDITOR_NAV_LOCATION:
        return "EditorNavigation"

    if suffix.startswith("close") or suffix.startswith("chrome_") or suffix in EDITOR_CHROME_WINDOW:
        return "EditorChrome"
    if suffix.startswith("settings") or suffix in EDITOR_CHROME_SETTINGS:
        return "EditorChrome"
    if suffix in EDITOR_CHROME_USER or suffix.startswith("person"):
        return "EditorChrome"
    if suffix.startswith("sign_"):
        return "EditorChrome"
    if suffix.startswith("eye"):
        return "EditorChrome"
    if suffix.startswith("lock") or suffix == "unlock" or suffix in {"key", "record_keys"}:
        return "EditorChrome"
    if suffix.startswith("workspace_") or suffix in EDITOR_CHROME_SECURITY:
        return "EditorChrome"
    if suffix in EDITOR_CHROME_UI:
        return "EditorChrome"
    if suffix.startswith("zoom_"):
        return "EditorChrome"
    if suffix.startswith("search") or suffix in EDITOR_CHROME_SEARCH:
        return "EditorChrome"
    if suffix.startswith("filter"):
        return "EditorChrome"
    if suffix.startswith("pin"):
        return "EditorChrome"
    if suffix in EDITOR_CHROME_PREVIEW:
        return "EditorChrome"
    if suffix in EDITOR_CHROME_EXTENSIONS:
        return "EditorChrome"
    if suffix in EDITOR_CHROME_AUDIO:
        return "EditorChrome"

    if suffix in EDITOR_EDIT:
        return "EditorEdit"
    if suffix in EDITOR_FORMAT_TEXT or suffix in EDITOR_FORMAT_FOLD or suffix in EDITOR_FORMAT_LIST:
        return "EditorFormat"
    if suffix.startswith("fold"):
        return "EditorFormat"
    if suffix in EDITOR_SYMBOL:
        return "EditorSymbol"
    if suffix in EDITOR_COMMENT or suffix.startswith("comment"):
        return "EditorComment"
    if suffix in EDITOR_REMOTE:
        return "EditorRemote"
    if (
        suffix.startswith("cloud")
        or suffix.startswith("azure")
        or suffix.startswith("server")
        or suffix == "database"
        or suffix == "package"
    ):
        return "EditorRemote"

    shared = shared_editor_domain(suffix)
    if shared is not None:
        return shared

    return "EditorMisc"


def cod_category_for(domain: str, suffix: str) -> str:
    if domain == "SourceControl":
        if suffix.startswith("git_") or suffix in SOURCE_CONTROL_GIT:
            return "Git"
        if suffix.startswith("diff"):
            return "Diff"
        if suffix in SOURCE_CONTROL_REVIEW:
            return "Review"
        return "Remote"

    if domain == "EditorDebug":
        if suffix.startswith("debug_breakpoint"):
            return "Breakpoint"
        if suffix.startswith("run_") or suffix in EDITOR_DEBUG_RUN or suffix in {"debug_start", "debug_stop"}:
            return "Run"
        if suffix in {"debug_console"}:
            return "Console"
        if suffix in {"debug_coverage"}:
            return "Coverage"
        if suffix.startswith("debug_stackframe"):
            return "Stack"
        if suffix in EDITOR_DEBUG_GENERAL:
            return "General"
        if suffix.startswith("debug_"):
            return "Control"
        if suffix in EDITOR_DEBUG_WATCH:
            return "Watch"
        if suffix in EDITOR_DEBUG_TEST:
            return "Test"
        return "General"

    if domain == "EditorLayout":
        if suffix.startswith("layout_panel"):
            return "Panel"
        if suffix.startswith("layout_sidebar"):
            return "Sidebar"
        if suffix.startswith("split_"):
            return "Split"
        if suffix in EDITOR_LAYOUT_WINDOW or suffix.startswith("screen_"):
            return "Window"
        if suffix in EDITOR_LAYOUT_VIEW or suffix in {"collapse_all", "expand_all"}:
            return "View"
        return "Layout"

    if domain == "EditorFile":
        if suffix in EDITOR_FILE_NEW:
            return "New"
        if suffix.startswith("save"):
            return "Save"
        if suffix.startswith("notebook"):
            return "Notebook"
        if suffix in EDITOR_FILE_DELETE:
            return "Delete"
        if suffix.startswith("folder") or suffix.startswith("root_folder"):
            return "Folder"
        return "File"

    if domain == "EditorTerminal":
        return "Shell"

    if domain == "EditorStatus":
        if suffix in EDITOR_STATUS_INFO:
            return "Info"
        if suffix in EDITOR_STATUS_WARNING:
            return "Warning"
        if suffix.startswith("error") or suffix in EDITOR_STATUS_ERROR:
            return "Error"
        if suffix.startswith("bell"):
            return "Alert"
        return "Success"

    if domain == "EditorNavigation":
        if suffix.startswith("arrow_"):
            return "Arrow"
        if suffix.startswith("chevron_"):
            return "Chevron"
        if suffix.startswith("triangle_"):
            return "Triangle"
        return "Location"

    if domain == "EditorChrome":
        if suffix.startswith("close") or suffix.startswith("chrome_") or suffix in EDITOR_CHROME_WINDOW:
            return "Window"
        if suffix.startswith("settings") or suffix in EDITOR_CHROME_SETTINGS:
            return "Settings"
        if suffix in EDITOR_CHROME_USER or suffix.startswith("person") or suffix.startswith("sign_"):
            return "User"
        if suffix.startswith("eye"):
            return "Visibility"
        if (
            suffix.startswith("lock")
            or suffix == "unlock"
            or suffix in {"key", "record_keys"}
            or suffix.startswith("workspace_")
            or suffix in EDITOR_CHROME_SECURITY
        ):
            return "Security"
        if suffix in EDITOR_CHROME_UI:
            return "UI"
        if suffix.startswith("search") or suffix in EDITOR_CHROME_SEARCH:
            return "Search"
        if suffix.startswith("filter"):
            return "Filter"
        if suffix.startswith("pin"):
            return "Pin"
        if suffix in EDITOR_CHROME_PREVIEW:
            return "Preview"
        if suffix in {"extensions"}:
            return "Extensions"
        if suffix == "color_mode":
            return "Theme"
        if suffix == "dashboard":
            return "Dashboard"
        if suffix in EDITOR_CHROME_AUDIO:
            return "Audio"
        if suffix.startswith("zoom_"):
            return "Zoom"
        return "UI"

    if domain == "EditorEdit":
        if suffix in {"add", "remove", "edit", "copy", "replace", "replace_all", "clear_all", "redo", "insert"}:
            return "Edit"
        return "Transform"

    if domain == "EditorFormat":
        if suffix in EDITOR_FORMAT_FOLD or suffix.startswith("fold"):
            return "Fold"
        if suffix in EDITOR_FORMAT_LIST or suffix == "sort_precedence":
            return "List"
        if suffix in {"json", "code", "regex", "markdown"}:
            return "Language"
        return "Style"

    if domain == "EditorSymbol":
        if suffix.startswith("type_hierarchy"):
            return "Hierarchy"
        if suffix.startswith("group_") or suffix.startswith("ungroup_"):
            return "Group"
        if suffix.startswith("symbol_") or suffix == "bracket_dot":
            return "Reference"
        return "Reference"

    if domain == "EditorComment":
        if suffix == "reply":
            return "Reply"
        return "Comment"

    if domain == "EditorTesting":
        return "Lab"

    if domain == "EditorRemote":
        if suffix.startswith("cloud") or suffix.startswith("azure"):
            return "Cloud"
        if suffix.startswith("server"):
            return "Server"
        if suffix == "desktop_download":
            return "Download"
        if suffix in {"database", "package"}:
            return "Package"
        return "Cloud"

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
        if suffix.startswith("call_"):
            return "Call"
        return "Feed"
    if domain == "EditorShape":
        return "Shape"
    if domain == "EditorMedia":
        if "camera" in suffix:
            return "Camera"
        if suffix.startswith("mic"):
            return "Audio"
        return "Device"
    if domain == "EditorChart":
        return "Chart"

    if domain != "EditorMisc":
        return "General"

    if suffix.startswith("link"):
        return "Link"
    if suffix.startswith("graph") or suffix in {"pie_chart", "table"}:
        return "Chart"
    if suffix.startswith("mail") or suffix in {"send", "call_incoming", "call_outgoing"}:
        return "Communication"
    if suffix.startswith("device_") or suffix.startswith("mic") or suffix.startswith("record"):
        return "Media"
    if suffix == "live_share":
        return "Collaboration"
    if suffix.startswith("issue") or suffix in {"issues", "milestone", "tasklist"}:
        return "Issue"
    if suffix in {"history", "refresh", "versions"}:
        return "History"
    if suffix.startswith("lightbulb") or suffix in {"wand", "sparkle"}:
        return "Hint"
    if (
        suffix.startswith("heart")
        or suffix.startswith("star_")
        or suffix.startswith("thumbs")
        or suffix in {"smiley", "reactions", "mention", "twitter"}
    ):
        return "Social"
    if suffix == "tools":
        return "Tool"
    if suffix.startswith("vm"):
        return "VM"
    if suffix in {"hubot", "squirrel", "octoface", "copilot"}:
        return "Bot"
    if suffix == "project":
        return "Project"
    if suffix.startswith("layers"):
        return "Layer"
    if suffix.startswith("circle") or suffix == "primitive_square":
        return "Shape"
    return "Misc"


def cod_path(domain: str, suffix: str) -> str:
    return f"{COD_DOMAIN_PATH_PREFIX[domain]}-{kebab_case(suffix)}"


def cod_ascii(suffix: str) -> str:
    if suffix.startswith("git_") or suffix in {"merge", "source_control", "repo"}:
        return "G"
    if suffix.startswith("diff"):
        return "D"
    if suffix.startswith("repo") or suffix.startswith("remote") or suffix.startswith("github"):
        return "R"
    if suffix.startswith("debug"):
        return "b"
    if suffix.startswith("run_") or suffix in {"play", "play_circle", "stop_circle"}:
        return ">"
    if suffix.startswith("symbol_"):
        return "S"
    if suffix.startswith("terminal"):
        return "$"
    if suffix.startswith("layout") or suffix.startswith("split_"):
        return "L"
    if suffix.startswith("file") or suffix.startswith("folder") or suffix.startswith("save"):
        return "F"
    if suffix in {"info", "question", "feedback"}:
        return "i"
    if suffix in {"warning"}:
        return "!"
    if suffix.startswith("error") or suffix == "report":
        return "X"
    if suffix.startswith("check") or suffix.startswith("verified") or suffix.startswith("pass"):
        return "+"
    if suffix.startswith("bell"):
        return "B"
    if suffix.startswith("arrow_") or suffix.startswith("chevron_") or suffix.startswith("triangle_"):
        return "^"
    if suffix in {"go_to_file", "location"}:
        return "@"
    if suffix.startswith("close") or suffix.startswith("chrome_"):
        return "x"
    if suffix.startswith("settings") or suffix == "gear":
        return "*"
    if suffix.startswith("search"):
        return "?"
    if suffix.startswith("eye"):
        return "o"
    if suffix.startswith("lock") or suffix == "unlock" or suffix == "key":
        return "#"
    if suffix.startswith("comment"):
        return "c"
    if suffix.startswith("cloud") or suffix.startswith("server"):
        return "C"
    if suffix in EDITOR_EDIT:
        return "e"
    return "."


def cod_row(suffix: str, codepoint: int) -> dict[str, str]:
    domain = cod_domain_for(suffix)
    category = cod_category_for(domain, suffix)
    return {
        "name": pascal_case(suffix),
        "domain": domain,
        "category": category,
        "nerd_suffix": suffix,
        "path": cod_path(domain, suffix),
        "codepoint": f"{codepoint:04X}",
        "ascii": cod_ascii(suffix),
        "source_set": "cod",
    }


def cod_category_order_for(domain: str) -> list[str]:
    return {
        "SourceControl": SOURCE_CONTROL_CATEGORY_ORDER,
        "EditorDebug": EDITOR_DEBUG_CATEGORY_ORDER,
        "EditorLayout": EDITOR_LAYOUT_CATEGORY_ORDER,
        "EditorFile": EDITOR_FILE_CATEGORY_ORDER,
        "EditorTerminal": EDITOR_TERMINAL_CATEGORY_ORDER,
        "EditorStatus": EDITOR_STATUS_CATEGORY_ORDER,
        "EditorNavigation": EDITOR_NAVIGATION_CATEGORY_ORDER,
        "EditorChrome": EDITOR_CHROME_CATEGORY_ORDER,
        "EditorEdit": EDITOR_EDIT_CATEGORY_ORDER,
        "EditorFormat": EDITOR_FORMAT_CATEGORY_ORDER,
        "EditorSymbol": EDITOR_SYMBOL_CATEGORY_ORDER,
        "EditorComment": EDITOR_COMMENT_CATEGORY_ORDER,
        "EditorTesting": EDITOR_TESTING_CATEGORY_ORDER,
        "EditorRemote": EDITOR_REMOTE_CATEGORY_ORDER,
        "EditorSocial": ["Star", "Heart", "Thumb", "Reaction"],
        "EditorCommunication": ["Mail", "Call", "Feed"],
        "EditorShape": ["Shape"],
        "EditorMedia": ["Camera", "Audio", "Device"],
        "EditorChart": ["Chart"],
        "EditorMisc": EDITOR_MISC_CATEGORY_ORDER,
    }.get(domain, [])


def cod_category_path_for(domain: str, category: str) -> str:
    prefix = COD_DOMAIN_PATH_PREFIX[domain]
    if category in {
        "Git",
        "Diff",
        "Remote",
        "Review",
        "Shell",
        "Info",
        "Warning",
        "Error",
        "Success",
        "Alert",
        "Arrow",
        "Chevron",
        "Triangle",
        "Location",
        "File",
        "Folder",
        "New",
        "Save",
        "Notebook",
        "Delete",
        "Layout",
        "Window",
        "View",
        "Breakpoint",
        "Run",
        "Control",
        "Stack",
        "Console",
        "Coverage",
        "Watch",
        "Test",
        "General",
        "Settings",
        "User",
        "Visibility",
        "Security",
        "Search",
        "Filter",
        "Pin",
        "Preview",
        "Extensions",
        "Theme",
        "Dashboard",
        "Audio",
        "Zoom",
        "Edit",
        "Transform",
        "Style",
        "Fold",
        "List",
        "Language",
        "Hierarchy",
        "Group",
        "Reference",
        "Comment",
        "Reply",
        "Lab",
        "Cloud",
        "Server",
        "Download",
        "Package",
        "Communication",
        "Media",
        "Collaboration",
        "Issue",
        "History",
        "Hint",
        "Social",
        "Tool",
        "VM",
        "Bot",
        "Project",
        "Layer",
        "Shape",
        "Link",
        "Chart",
        "Misc",
    }:
        if category in {"Shell", "Layout", "File", "Settings", "Search", "Filter", "Preview", "Dashboard"}:
            return prefix
        return f"{prefix}-{kebab_case(category)}"
    if category in {"Panel", "Sidebar", "Split"}:
        return f"{prefix}-{kebab_case(category)}"
    return prefix


COD_REGISTRY = {
    "order": COD_DOMAIN_ORDER,
    "path_prefix": COD_DOMAIN_PATH_PREFIX,
    "cpp_stem": COD_DOMAIN_CPP_STEM,
    "enum": COD_DOMAIN_ENUM,
    "category_enum": COD_DOMAIN_CATEGORY_ENUM,
    "category_order": cod_category_order_for,
    "category_path": cod_category_path_for,
}
