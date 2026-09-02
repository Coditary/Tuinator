#!/usr/bin/env python3
"""Parse Nerd Fonts i_*.sh glyph definition scripts."""

from __future__ import annotations

import re
import urllib.request
from pathlib import Path

NERD_FONTS_TAG = "v3.4.0"
NERD_FONTS_LIB_BASE = (
    f"https://raw.githubusercontent.com/ryanoasis/nerd-fonts/{NERD_FONTS_TAG}/bin/scripts/lib"
)

# Sets loaded by i_all.sh @ v3.4.0
ALL_SETS = ("cod", "dev", "extra", "fae", "fa", "iec", "logos", "oct", "ple", "pom", "seti", "weather", "md")

SET_VAR_PREFIX = {
    "cod": "cod",
    "dev": "dev",
    "extra": "extra",
    "fae": "fae",
    "fa": "fa",
    "iec": "iec",
    "logos": "linux",
    "oct": "oct",
    "ple": ("pl", "ple"),  # i_pl_* and i_ple_*
    "pom": "pom",
    "seti": ("seti", "custom"),
    "weather": "weather",
    "md": "md",
}


def fetch_set_script(set_name: str) -> str:
    url = f"{NERD_FONTS_LIB_BASE}/i_{set_name}.sh"
    return urllib.request.urlopen(url).read().decode("utf-8")


def parse_glyph_script(text: str, set_name: str) -> dict[str, int]:
    """Return mapping nerd_suffix -> codepoint for one i_<set>.sh file."""
    if set_name == "seti":
        return parse_seti_script(text)

    icons: dict[str, int] = {}
    prefix = SET_VAR_PREFIX.get(set_name, set_name)
    prefixes = (prefix,) if isinstance(prefix, str) else prefix

    for line in text.splitlines():
        match = re.search(r"i='(.)'\s+i_(\w+)=\$i", line)
        if not match:
            continue
        ch, var = match.group(1), match.group(2)
        for pfx in prefixes:
            needle = f"{pfx}_"
            if var.startswith(needle):
                suffix = var[len(needle) :]
                icons[suffix] = ord(ch)
                break
    return icons


def parse_seti_script(text: str) -> dict[str, int]:
    """Parse i_seti.sh: seti + custom icons, aliases, 191 unique codepoints."""
    var_cp: dict[str, int] = {}
    aliases: dict[str, str] = {}

    for line in text.splitlines():
        match = re.search(r"i='(.)'\s+i_(\w+)=\$i", line)
        if match:
            var_cp[match.group(2)] = ord(match.group(1))
            continue
        match = re.search(r"i_(\w+)=\$i_(\w+)", line)
        if match:
            aliases[match.group(1)] = match.group(2)

    def resolve(var: str, seen: set[str] | None = None) -> int | None:
        if seen is None:
            seen = set()
        if var in var_cp:
            return var_cp[var]
        if var in aliases:
            if var in seen:
                return None
            seen.add(var)
            return resolve(aliases[var], seen)
        return None

    def suffix_of(var: str) -> str:
        if var.startswith("seti_"):
            return var[5:]
        if var.startswith("custom_"):
            return var[7:]
        return var

    relevant = {
        var
        for var in set(var_cp) | set(aliases)
        if var.startswith(("seti_", "custom_")) or var.startswith("indent")
    }

    cp_to_vars: dict[int, list[str]] = {}
    for var in relevant:
        cp = resolve(var)
        if cp is not None:
            cp_to_vars.setdefault(cp, []).append(var)

    def cluster_key(item: tuple[int, list[str]]) -> tuple[int, int]:
        _cp, vars = item
        has_seti = any(v.startswith("seti_") and v in var_cp for v in vars)
        has_custom = any(v.startswith("custom_") and v in var_cp for v in vars)
        return (0 if has_seti else 1 if has_custom else 2, _cp)

    icons: dict[str, int] = {}
    for cp, vars in sorted(cp_to_vars.items(), key=cluster_key):
        direct = [v for v in vars if v in var_cp]
        pick_from = (
            [v for v in direct if v.startswith("seti_")]
            or [v for v in direct if v.startswith("custom_")]
            or direct
            or sorted(vars)
        )
        var = pick_from[0]
        suffix = suffix_of(var)
        if suffix in icons and icons[suffix] != cp:
            suffix = f"custom_{suffix_of(var)}"
        icons[suffix] = cp
    return icons


def load_set(set_name: str, text: str | None = None) -> dict[str, int]:
    if text is None:
        text = fetch_set_script(set_name)
    return parse_glyph_script(text, set_name)


def pascal_case(snake: str) -> str:
    parts = snake.split("_")
    return "".join(p[:1].upper() + p[1:] for p in parts if p)


def kebab_case(snake: str) -> str:
    return snake.replace("_", "-")
