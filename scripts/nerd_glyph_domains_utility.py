#!/usr/bin/env python3
"""Semantic domain routing for Nerd Fonts Powerline / small utility sets."""

from __future__ import annotations

from nerd_font_lib import kebab_case, pascal_case

# --- Powerline (i_ple.sh) ---

PLE_CLASSIC_SUFFIXES = {"branch", "line_number", "hostname", "column_number"}

PLE_DOMAIN_ORDER = ["Powerline", "PowerlineExtra", "PowerSymbol", "Progress", "Pomodoro"]

PLE_DOMAIN_PATH_PREFIX = {
    "Powerline": "powerline",
    "PowerlineExtra": "powerline-extra",
    "PowerSymbol": "power",
    "Progress": "progress",
    "Pomodoro": "pomodoro",
}

PLE_DOMAIN_CPP_STEM = {
    "Powerline": "powerline_icon",
    "PowerlineExtra": "powerline_extra_icon",
    "PowerSymbol": "power_symbol_icon",
    "Progress": "progress_icon",
    "Pomodoro": "pomodoro_icon",
}

PLE_DOMAIN_ENUM = {
    "Powerline": "PowerlineIcon",
    "PowerlineExtra": "PowerlineExtraIcon",
    "PowerSymbol": "PowerSymbolIcon",
    "Progress": "ProgressIcon",
    "Pomodoro": "PomodoroIcon",
}

PLE_DOMAIN_CATEGORY_ENUM = {
    "Powerline": "PowerlineIconCategory",
    "PowerlineExtra": "PowerlineExtraIconCategory",
    "PowerSymbol": "PowerSymbolIconCategory",
    "Progress": "ProgressIconCategory",
    "Pomodoro": "PomodoroIconCategory",
}

POWERLINE_CATEGORY_ORDER = ["Segment"]
POWERLINE_EXTRA_CATEGORY_ORDER = [
    "Divider",
    "Curve",
    "Separator",
    "Pattern",
    "Inverse",
]

PLE_EXTRA_DIVIDER = {
    "left_hard_divider",
    "left_soft_divider",
    "right_hard_divider",
    "right_soft_divider",
    "right_hard_divider_inverse",
    "left_hard_divider_inverse",
}
PLE_EXTRA_CURVE = {
    "right_half_circle_thick",
    "right_half_circle_thin",
    "left_half_circle_thick",
    "left_half_circle_thin",
}
PLE_EXTRA_SEPARATOR = {
    "lower_left_triangle",
    "backslash_separator",
    "lower_right_triangle",
    "forwardslash_separator",
    "upper_left_triangle",
    "forwardslash_separator_redundant",
    "upper_right_triangle",
    "backslash_separator_redundant",
}
PLE_EXTRA_PATTERN = {
    "flame_thick",
    "flame_thin",
    "flame_thick_mirrored",
    "flame_thin_mirrored",
    "pixelated_squares_small",
    "pixelated_squares_small_mirrored",
    "pixelated_squares_big",
    "pixelated_squares_big_mirrored",
    "ice_waveform",
    "ice_waveform_mirrored",
    "honeycomb",
    "honeycomb_outline",
    "lego_separator",
    "lego_separator_thin",
    "lego_block_facing",
    "lego_block_sideways",
    "trapezoid_top_bottom",
    "trapezoid_top_bottom_mirrored",
}


def ple_domain_for(suffix: str) -> str:
    if suffix in PLE_CLASSIC_SUFFIXES:
        return "Powerline"
    return "PowerlineExtra"


def ple_category_for(domain: str, suffix: str) -> str:
    if domain == "Powerline":
        return "Segment"
    if suffix in PLE_EXTRA_DIVIDER:
        return "Divider"
    if suffix in PLE_EXTRA_CURVE:
        return "Curve"
    if suffix in PLE_EXTRA_SEPARATOR:
        return "Separator"
    if suffix in PLE_EXTRA_PATTERN:
        return "Pattern"
    if "inverse" in suffix:
        return "Inverse"
    return "Pattern"


def ple_path(domain: str, suffix: str) -> str:
    return f"{PLE_DOMAIN_PATH_PREFIX[domain]}-{kebab_case(suffix)}"


def ple_ascii(suffix: str) -> str:
    if suffix in ("branch",):
        return "Y"
    if "number" in suffix or "column" in suffix:
        return "#"
    if suffix == "hostname":
        return "H"
    if "divider" in suffix:
        return ">" if "left" in suffix else "<"
    if "circle" in suffix:
        return "(" if "left" in suffix else ")"
    if "triangle" in suffix or "separator" in suffix or "trapezoid" in suffix:
        return "/"
    if "flame" in suffix or "waveform" in suffix:
        return "~"
    if "lego" in suffix and "separator" in suffix:
        return "-"
    return "#"


def ple_row(suffix: str, codepoint: int) -> dict[str, str]:
    domain = ple_domain_for(suffix)
    category = ple_category_for(domain, suffix)
    return {
        "name": pascal_case(suffix),
        "domain": domain,
        "category": category,
        "nerd_suffix": suffix,
        "path": ple_path(domain, suffix),
        "codepoint": f"{codepoint:04X}",
        "ascii": ple_ascii(suffix),
        "source_set": "ple",
    }


# --- IEC power symbols ---

IEC_DOMAIN = "PowerSymbol"
IEC_CATEGORY_ORDER = ["Power"]


def iec_row(suffix: str, codepoint: int) -> dict[str, str]:
    return {
        "name": pascal_case(suffix),
        "domain": IEC_DOMAIN,
        "category": "Power",
        "nerd_suffix": suffix,
        "path": f"power-{kebab_case(suffix)}",
        "codepoint": f"{codepoint:04X}",
        "ascii": "O",
        "source_set": "iec",
    }


# --- Extra progress spinners ---

PROGRESS_DOMAIN = "Progress"
PROGRESS_CATEGORY_ORDER = ["Spinner", "Bar"]


def progress_row(suffix: str, codepoint: int) -> dict[str, str]:
    category = "Spinner" if suffix.startswith("spinner") else "Bar"
    return {
        "name": pascal_case(suffix),
        "domain": PROGRESS_DOMAIN,
        "category": category,
        "nerd_suffix": suffix,
        "path": f"progress-{kebab_case(suffix)}",
        "codepoint": f"{codepoint:04X}",
        "ascii": "|",
        "source_set": "extra",
    }


# --- Pomodoro ---

POMODORO_DOMAIN = "Pomodoro"
POMODORO_CATEGORY_ORDER = ["Session", "Pause", "Interruption", "Focus"]


def pomodoro_row(suffix: str, codepoint: int) -> dict[str, str]:
    if suffix.startswith("done") or suffix.startswith("estimated") or suffix.startswith("ticking") or suffix.startswith("squashed"):
        category = "Session"
    elif "pause" in suffix:
        category = "Pause"
    elif "interruption" in suffix:
        category = "Interruption"
    else:
        category = "Focus"
    return {
        "name": pascal_case(suffix),
        "domain": POMODORO_DOMAIN,
        "category": category,
        "nerd_suffix": suffix,
        "path": f"pomodoro-{kebab_case(suffix)}",
        "codepoint": f"{codepoint:04X}",
        "ascii": "o",
        "source_set": "pom",
    }


def ple_category_order_for(domain: str) -> list[str]:
    if domain == "Powerline":
        return POWERLINE_CATEGORY_ORDER
    if domain == "PowerlineExtra":
        return POWERLINE_EXTRA_CATEGORY_ORDER
    if domain == "PowerSymbol":
        return IEC_CATEGORY_ORDER
    if domain == "Progress":
        return PROGRESS_CATEGORY_ORDER
    if domain == "Pomodoro":
        return POMODORO_CATEGORY_ORDER
    return []


def ple_category_path_for(domain: str, category: str) -> str:
    prefix = PLE_DOMAIN_PATH_PREFIX[domain]
    if category in ("Segment", "Power", "Session", "Pause", "Interruption", "Focus"):
        return prefix
    return f"{prefix}-{kebab_case(category)}"


PLE_REGISTRY = {
    "order": PLE_DOMAIN_ORDER,
    "path_prefix": PLE_DOMAIN_PATH_PREFIX,
    "cpp_stem": PLE_DOMAIN_CPP_STEM,
    "enum": PLE_DOMAIN_ENUM,
    "category_enum": PLE_DOMAIN_CATEGORY_ENUM,
    "category_order": ple_category_order_for,
    "category_path": ple_category_path_for,
}
