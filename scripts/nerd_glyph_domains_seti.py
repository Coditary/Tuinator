#!/usr/bin/env python3
"""Semantic domain routing for Nerd Fonts Seti-UI (i_seti.sh, v3.4.0)."""

from __future__ import annotations

from nerd_font_lib import kebab_case, pascal_case

SETI_DOMAIN_ORDER = [
    "FileLanguage",
    "FileMarkup",
    "FileConfig",
    "FileData",
    "FileMedia",
    "FileFolder",
    "FileProject",
]

SETI_DOMAIN_PATH_PREFIX = {
    "FileLanguage": "file-language",
    "FileMarkup": "file-markup",
    "FileConfig": "file-config",
    "FileData": "file-data",
    "FileMedia": "file-media",
    "FileFolder": "file-folder",
    "FileProject": "file-project",
}

SETI_DOMAIN_CPP_STEM = {
    "FileLanguage": "file_language_icon",
    "FileMarkup": "file_markup_icon",
    "FileConfig": "file_config_icon",
    "FileData": "file_data_icon",
    "FileMedia": "file_media_icon",
    "FileFolder": "file_folder_icon",
    "FileProject": "file_project_icon",
}

SETI_DOMAIN_ENUM = {
    "FileLanguage": "FileLanguageIcon",
    "FileMarkup": "FileMarkupIcon",
    "FileConfig": "FileConfigIcon",
    "FileData": "FileDataIcon",
    "FileMedia": "FileMediaIcon",
    "FileFolder": "FileFolderIcon",
    "FileProject": "FileProjectIcon",
}

SETI_DOMAIN_CATEGORY_ENUM = {
    "FileLanguage": "FileLanguageIconCategory",
    "FileMarkup": "FileMarkupIconCategory",
    "FileConfig": "FileConfigIconCategory",
    "FileData": "FileDataIconCategory",
    "FileMedia": "FileMediaIconCategory",
    "FileFolder": "FileFolderIconCategory",
    "FileProject": "FileProjectIconCategory",
}

FILE_LANGUAGE = {
    "css",
    "powershell",
    "puppet",
    "sublime",
    "apple",
    "asm",
    "babel",
    "bicep",
    "bsl",
    "c",
    "c_sharp",
    "cjsx",
    "clojure",
    "coffee",
    "coldfusion",
    "cpp",
    "crystal",
    "crystal_embedded",
    "cu",
    "d",
    "dart",
    "elixir",
    "elixir_script",
    "elm",
    "f_sharp",
    "go",
    "go2",
    "godot",
    "graphql",
    "hacklang",
    "haskell",
    "haxe",
    "java",
    "javascript",
    "julia",
    "kotlin",
    "livescript",
    "lua",
    "nim",
    "ocaml",
    "perl",
    "php",
    "prolog",
    "purescript",
    "python",
    "r",
    "rails",
    "react",
    "reasonml",
    "rescript",
    "ruby",
    "rust",
    "scala",
    "shell",
    "swift",
    "typescript",
    "vala",
    "vue",
    "wasm",
    "wat",
    "zig",
    "ada",
    "astro",
    "chuck",
    "common_lisp",
    "electron",
    "emacs",
    "fennel",
    "ionic",
    "msdos",
    "neovim",
    "orgmode",
    "scheme",
    "svelte",
    "v_lang",
    "vim",
    "vitruvian",
    "windows",
    "custom_asm",
    "custom_c",
    "custom_cpp",
    "custom_css",
    "custom_go",
}

FILE_MARKUP = {
    "ejs",
    "haml",
    "html",
    "jade",
    "jinja",
    "liquid",
    "markdown",
    "mustache",
    "nunjucks",
    "pug",
    "slim",
    "smarty",
    "svg",
    "twig",
    "xml",
}

FILE_CONFIG = {
    "json",
    "bazel",
    "bower",
    "config",
    "docker",
    "editorconfig",
    "eslint",
    "firebase",
    "gradle",
    "grunt",
    "gulp",
    "heroku",
    "jenkins",
    "karma",
    "less",
    "makefile",
    "maven",
    "npm",
    "npm_ignored",
    "pipeline",
    "plan",
    "platformio",
    "prettier",
    "prisma",
    "rollup",
    "sass",
    "sbt",
    "settings",
    "spring",
    "stylelint",
    "stylus",
    "terraform",
    "toml",
    "tsconfig",
    "webpack",
    "wgt",
    "yarn",
    "yml",
}

FILE_DATA = {
    "argdown",
    "cake",
    "cake_php",
    "code_climate",
    "csv",
    "db",
    "deprecation_cop",
    "error",
    "ethereum",
    "grails",
    "hex",
    "info",
    "license",
    "lock",
    "odata",
    "pddl",
    "pdf",
    "salesforce",
    "search",
    "tex",
    "time_cop",
    "todo",
    "word",
    "xls",
}

FILE_MEDIA = {
    "audio",
    "favicon",
    "firefox",
    "font",
    "illustrator",
    "image",
    "photoshop",
    "play_arrow",
    "video",
}

FILE_FOLDER = {
    "folder",
    "folder_config",
    "folder_git",
    "folder_git_branch",
    "folder_github",
    "folder_npm",
    "folder_oct",
    "folder_open",
    "git_folder",
    "home",
    "custom_folder",
}

FILE_PROJECT = {
    "checkbox",
    "checkbox_unchecked",
    "clock",
    "code_search",
    "custom_default",
    "default",
    "git",
    "git_ignore",
    "github",
    "gitlab",
    "happenings",
    "ignored",
    "indent_line",
    "mdo",
    "new_file",
    "notebook",
    "project",
    "text",
    "zip",
}

FILE_LANGUAGE_CATEGORY_ORDER = ["Language"]
FILE_MARKUP_CATEGORY_ORDER = ["Markup"]
FILE_CONFIG_CATEGORY_ORDER = ["Config"]
FILE_DATA_CATEGORY_ORDER = ["Data"]
FILE_MEDIA_CATEGORY_ORDER = ["Media"]
FILE_FOLDER_CATEGORY_ORDER = ["Folder"]
FILE_PROJECT_CATEGORY_ORDER = ["Project"]


def seti_domain_for(suffix: str) -> str:
    if suffix in FILE_LANGUAGE:
        return "FileLanguage"
    if suffix in FILE_MARKUP:
        return "FileMarkup"
    if suffix in FILE_CONFIG:
        return "FileConfig"
    if suffix in FILE_DATA:
        return "FileData"
    if suffix in FILE_MEDIA:
        return "FileMedia"
    if suffix in FILE_FOLDER:
        return "FileFolder"
    if suffix in FILE_PROJECT:
        return "FileProject"

    if suffix.endswith("_folder") or suffix in {"folder", "home"}:
        return "FileFolder"
    if suffix in {"json", "yaml"}:
        return "FileConfig"
    if suffix in {"powershell", "sublime"}:
        return "FileLanguage"
    if suffix in {"babel", "rollup", "webpack", "gulp", "grunt", "karma", "jenkins"}:
        return "FileConfig"
    if suffix in {"docker", "terraform", "makefile", "gradle", "maven", "sbt", "bazel"}:
        return "FileConfig"
    if suffix in {"github", "gitlab", "git", "npm", "yarn", "bower"}:
        return "FileProject"
    if suffix in {"image", "audio", "video", "font"}:
        return "FileMedia"
    if suffix == "pdf":
        return "FileData"
    if suffix in {"db", "csv", "xls", "word", "tex", "odata"}:
        return "FileData"
    if suffix in {"html", "markdown", "xml", "svg"}:
        return "FileMarkup"
    if suffix in {"python", "javascript", "typescript", "ruby", "rust", "go"}:
        return "FileLanguage"
    if suffix.startswith("custom_"):
        return "FileLanguage"

    return "FileProject"


def seti_category_for(domain: str, suffix: str) -> str:
    return {
        "FileLanguage": "Language",
        "FileMarkup": "Markup",
        "FileConfig": "Config",
        "FileData": "Data",
        "FileMedia": "Media",
        "FileFolder": "Folder",
        "FileProject": "Project",
    }[domain]


def seti_path(domain: str, suffix: str) -> str:
    prefix = SETI_DOMAIN_PATH_PREFIX[domain]
    body = suffix
    domain_tail = prefix.rsplit("-", 1)[-1]
    if body == domain_tail:
        return prefix
    if body.startswith(f"{domain_tail}_"):
        body = body[len(domain_tail) + 1 :]
    return f"{prefix}-{kebab_case(body)}"


def seti_ascii(domain: str, suffix: str) -> str:
    return {
        "FileLanguage": "L",
        "FileMarkup": "<",
        "FileConfig": "C",
        "FileData": "D",
        "FileMedia": "M",
        "FileFolder": "/",
        "FileProject": "P",
    }[domain]


def seti_row(suffix: str, codepoint: int) -> dict[str, str]:
    domain = seti_domain_for(suffix)
    category = seti_category_for(domain, suffix)
    return {
        "name": pascal_case(suffix),
        "domain": domain,
        "category": category,
        "nerd_suffix": suffix,
        "path": seti_path(domain, suffix),
        "codepoint": f"{codepoint:04X}",
        "ascii": seti_ascii(domain, suffix),
        "source_set": "seti",
    }


def seti_category_order_for(domain: str) -> list[str]:
    return {
        "FileLanguage": FILE_LANGUAGE_CATEGORY_ORDER,
        "FileMarkup": FILE_MARKUP_CATEGORY_ORDER,
        "FileConfig": FILE_CONFIG_CATEGORY_ORDER,
        "FileData": FILE_DATA_CATEGORY_ORDER,
        "FileMedia": FILE_MEDIA_CATEGORY_ORDER,
        "FileFolder": FILE_FOLDER_CATEGORY_ORDER,
        "FileProject": FILE_PROJECT_CATEGORY_ORDER,
    }.get(domain, [])


def seti_category_path_for(domain: str, category: str) -> str:
    return SETI_DOMAIN_PATH_PREFIX[domain]


SETI_REGISTRY = {
    "order": SETI_DOMAIN_ORDER,
    "path_prefix": SETI_DOMAIN_PATH_PREFIX,
    "cpp_stem": SETI_DOMAIN_CPP_STEM,
    "enum": SETI_DOMAIN_ENUM,
    "category_enum": SETI_DOMAIN_CATEGORY_ENUM,
    "category_order": seti_category_order_for,
    "category_path": seti_category_path_for,
}
