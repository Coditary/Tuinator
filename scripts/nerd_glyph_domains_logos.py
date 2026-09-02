#!/usr/bin/env python3
"""Semantic domain routing for Nerd Fonts Font Logos (i_logos.sh, 130 icons)."""

from __future__ import annotations

from nerd_font_lib import kebab_case, pascal_case

LOGOS_DOMAIN_ORDER = [
    "LinuxDistro",
    "DesktopEnvironment",
    "TechSaas",
    "TechCreative",
    "TechCommunity",
]

LOGOS_DOMAIN_PATH_PREFIX = {
    "LinuxDistro": "linux-distro",
    "DesktopEnvironment": "desktop-environment",
    "TechSaas": "tech-saas",
    "TechCreative": "tech-creative",
    "TechCommunity": "tech-community",
}

LOGOS_DOMAIN_CPP_STEM = {
    "LinuxDistro": "linux_distro_icon",
    "DesktopEnvironment": "desktop_environment_icon",
    "TechSaas": "tech_saas_icon",
    "TechCreative": "tech_creative_icon",
    "TechCommunity": "tech_community_icon",
}

LOGOS_DOMAIN_ENUM = {
    "LinuxDistro": "LinuxDistroIcon",
    "DesktopEnvironment": "DesktopEnvironmentIcon",
    "TechSaas": "TechSaasIcon",
    "TechCreative": "TechCreativeIcon",
    "TechCommunity": "TechCommunityIcon",
}

LOGOS_DOMAIN_CATEGORY_ENUM = {
    "LinuxDistro": "LinuxDistroIconCategory",
    "DesktopEnvironment": "DesktopEnvironmentIconCategory",
    "TechSaas": "TechSaasIconCategory",
    "TechCreative": "TechCreativeIconCategory",
    "TechCommunity": "TechCommunityIconCategory",
}

LINUX_DISTRO_BSD = {
    "freebsd",
    "openbsd",
}
LINUX_DISTRO_VARIANT = {
    "fedora_inverse",
    "linuxmint_inverse",
    "slackware_inverse",
    "ubuntu_inverse",
}

LINUX_DISTRO = {
    "almalinux",
    "alpine",
    "aosc",
    "archcraft",
    "archlabs",
    "archlinux",
    "arcolinux",
    "artix",
    "biglinux",
    "centos",
    "coreos",
    "crystal",
    "debian",
    "deepin",
    "devuan",
    "elementary",
    "endeavour",
    "fedora",
    "garuda",
    "gentoo",
    "gnu_guix",
    "hyperbola",
    "illumos",
    "kali_linux",
    "leap",
    "linuxmint",
    "locos",
    "mageia",
    "mandriva",
    "manjaro",
    "mxlinux",
    "nixos",
    "nobara",
    "opensuse",
    "parabola",
    "parrot",
    "pop_os",
    "postmarketos",
    "puppy",
    "qubesos",
    "raspberry_pi",
    "redhat",
    "rocky_linux",
    "sabayon",
    "slackware",
    "solus",
    "tails",
    "trisquel",
    "tumbleweed",
    "ubuntu",
    "void",
    "xerolinux",
    "zorin",
}

DESKTOP_WINDOW_MANAGER = {
    "awesome",
    "bspwm",
    "dwm",
    "enlightenment",
    "fluxbox",
    "hyprland",
    "i3",
    "jwm",
    "qtile",
    "river",
    "sway",
    "xmonad",
}

DESKTOP_ENVIRONMENT = {
    "budgie",
    "cinnamon",
    "gnome",
    "kde",
    "kde_neon",
    "kde_plasma",
    "kubuntu",
    "kubuntu_inverse",
    "lxde",
    "lxle",
    "lxqt",
    "mate",
    "vanilla",
    "xfce",
}

DESKTOP_PLATFORM = {
    "freedesktop",
    "gtk",
    "qt",
    "wayland",
    "xorg",
}

TECH_SAAS_DEVELOPMENT = {
    "arduino",
    "kicad",
    "neovim",
    "octoprint",
    "openscad",
    "osh",
    "oshwa",
    "prusaslicer",
    "reprap",
    "riscv",
    "typst",
    "vscodium",
}

TECH_SAAS_COMMUNICATION = {
    "thunderbird",
    "tor",
}

TECH_SAAS_MEDIA = {
    "mpv",
}

TECH_SAAS_OFFICE = {
    "libreoffice",
    "libreofficebase",
    "libreofficecalc",
    "libreofficedraw",
    "libreofficeimpress",
    "libreofficemath",
    "libreofficewriter",
}

TECH_SAAS_CLOUD = {
    "docker",
    "flathub",
    "snappy",
}

TECH_SAAS_BRAND = {
    "apple",
}

TECH_CREATIVE = {
    "freecad",
    "gimp",
    "inkscape",
    "kdenlive",
    "krita",
}

TECH_COMMUNITY_ORG = {
    "fdroid",
    "fosdem",
    "osi",
    "wikimedia",
}

TECH_COMMUNITY_GIT = {
    "codeberg",
    "forgejo",
    "gitea",
}

TECH_COMMUNITY_MASCOT = {
    "ferris",
    "tux",
}

LINUX_DISTRO_CATEGORY_ORDER = ["Distro", "BSD", "Variant"]
DESKTOP_ENVIRONMENT_CATEGORY_ORDER = ["WindowManager", "Desktop", "Platform"]
TECH_SAAS_CATEGORY_ORDER = [
    "Development",
    "Office",
    "Cloud",
    "Communication",
    "Media",
    "Brand",
]
TECH_CREATIVE_CATEGORY_ORDER = ["Creative"]
TECH_COMMUNITY_CATEGORY_ORDER = ["Organization", "Git", "Mascot"]


def logos_domain_for(suffix: str) -> str:
    if suffix in LINUX_DISTRO or suffix in LINUX_DISTRO_BSD or suffix in LINUX_DISTRO_VARIANT:
        return "LinuxDistro"
    if (
        suffix in DESKTOP_WINDOW_MANAGER
        or suffix in DESKTOP_ENVIRONMENT
        or suffix in DESKTOP_PLATFORM
    ):
        return "DesktopEnvironment"
    if suffix in TECH_CREATIVE:
        return "TechCreative"
    if (
        suffix in TECH_COMMUNITY_ORG
        or suffix in TECH_COMMUNITY_GIT
        or suffix in TECH_COMMUNITY_MASCOT
    ):
        return "TechCommunity"
    return "TechSaas"


def logos_category_for(domain: str, suffix: str) -> str:
    if domain == "LinuxDistro":
        if suffix in LINUX_DISTRO_BSD:
            return "BSD"
        if suffix in LINUX_DISTRO_VARIANT:
            return "Variant"
        return "Distro"

    if domain == "DesktopEnvironment":
        if suffix in DESKTOP_WINDOW_MANAGER:
            return "WindowManager"
        if suffix in DESKTOP_PLATFORM:
            return "Platform"
        return "Desktop"

    if domain == "TechCreative":
        return "Creative"

    if domain == "TechCommunity":
        if suffix in TECH_COMMUNITY_GIT:
            return "Git"
        if suffix in TECH_COMMUNITY_MASCOT:
            return "Mascot"
        return "Organization"

    if suffix in TECH_SAAS_DEVELOPMENT:
        return "Development"
    if suffix in TECH_SAAS_OFFICE:
        return "Office"
    if suffix in TECH_SAAS_CLOUD:
        return "Cloud"
    if suffix in TECH_SAAS_COMMUNICATION:
        return "Communication"
    if suffix in TECH_SAAS_MEDIA:
        return "Media"
    if suffix in TECH_SAAS_BRAND:
        return "Brand"
    return "Development"


def logos_path(domain: str, suffix: str) -> str:
    return f"{LOGOS_DOMAIN_PATH_PREFIX[domain]}-{kebab_case(suffix)}"


def logos_ascii(domain: str, suffix: str) -> str:
    if domain == "LinuxDistro":
        if suffix in LINUX_DISTRO_BSD:
            return "B"
        return "L"
    if domain == "DesktopEnvironment":
        if suffix in DESKTOP_WINDOW_MANAGER:
            return "W"
        if suffix in DESKTOP_PLATFORM:
            return "P"
        return "D"
    if domain == "TechCreative":
        return "C"
    if domain == "TechCommunity":
        if suffix in TECH_COMMUNITY_GIT:
            return "G"
        if suffix in TECH_COMMUNITY_MASCOT:
            return "@"
        return "O"
    if suffix in TECH_SAAS_CLOUD:
        return "C"
    if suffix in TECH_SAAS_BRAND:
        return "A"
    if suffix in TECH_SAAS_OFFICE:
        return "O"
    if suffix in TECH_SAAS_COMMUNICATION:
        return "M"
    return "*"


def logos_row(suffix: str, codepoint: int) -> dict[str, str]:
    domain = logos_domain_for(suffix)
    category = logos_category_for(domain, suffix)
    return {
        "name": pascal_case(suffix),
        "domain": domain,
        "category": category,
        "nerd_suffix": suffix,
        "path": logos_path(domain, suffix),
        "codepoint": f"{codepoint:04X}",
        "ascii": logos_ascii(domain, suffix),
        "source_set": "logos",
    }


def logos_category_order_for(domain: str) -> list[str]:
    return {
        "LinuxDistro": LINUX_DISTRO_CATEGORY_ORDER,
        "DesktopEnvironment": DESKTOP_ENVIRONMENT_CATEGORY_ORDER,
        "TechSaas": TECH_SAAS_CATEGORY_ORDER,
        "TechCreative": TECH_CREATIVE_CATEGORY_ORDER,
        "TechCommunity": TECH_COMMUNITY_CATEGORY_ORDER,
    }.get(domain, [])


def logos_category_path_for(domain: str, category: str) -> str:
    prefix = LOGOS_DOMAIN_PATH_PREFIX[domain]
    if category in {"Distro", "Desktop", "Creative", "Brand", "Media", "Organization", "Mascot"}:
        return prefix
    return f"{prefix}-{kebab_case(category)}"


LOGOS_REGISTRY = {
    "order": LOGOS_DOMAIN_ORDER,
    "path_prefix": LOGOS_DOMAIN_PATH_PREFIX,
    "cpp_stem": LOGOS_DOMAIN_CPP_STEM,
    "enum": LOGOS_DOMAIN_ENUM,
    "category_enum": LOGOS_DOMAIN_CATEGORY_ENUM,
    "category_order": logos_category_order_for,
    "category_path": logos_category_path_for,
}
