#!/usr/bin/env python3
"""Shared naming + categorization for Nerd Fonts weather icons (v3.4.0)."""

from __future__ import annotations

import re
from typing import Callable

NERD_FONTS_TAG = "v3.4.0"

CATEGORY_ORDER = [
    "Day",
    "NightAlt",
    "Night",
    "Moon",
    "Wind",
    "Storm",
    "Cloud",
    "Precipitation",
    "Time",
    "Direction",
    "Temperature",
    "Pressure",
    "Sun",
    "Beach",
    "Hazard",
    "Marine",
    "Astronomy",
    "Navigation",
    "Other",
]


def pascal_case(snake: str) -> str:
    parts = snake.split("_")
    return "".join(p[:1].upper() + p[1:] for p in parts if p)


def category_kebab(category: str) -> str:
    mapping = {
        "NightAlt": "night-alt",
    }
    if category in mapping:
        return mapping[category]
    return category.lower()


def category_path(category: str) -> str:
    return f"weather-{category_kebab(category)}"


def nested_weather_path(suffix: str) -> str:
    """Nested cheat-sheet path, e.g. time_1 -> weather-time-1."""
    return "weather-" + suffix.replace("_", "-")


def legacy_weather_name(suffix: str) -> str:
    """Old underscore form kept for lookup compatibility."""
    return f"weather-{suffix}"


def ascii_for(name: str) -> str:
    if "rain" in name or "shower" in name:
        return "r"
    if "snow" in name or "sleet" in name:
        return "*"
    if "sun" in name or "day" in name:
        return "o"
    if "night" in name or "moon" in name:
        return "n"
    if "wind" in name or "tornado" in name:
        return "~"
    if "cloud" in name or "fog" in name:
        return "c"
    if "storm" in name or "lightning" in name or "thunder" in name:
        return "!"
    if "direction" in name:
        return ">"
    if name.startswith("time_"):
        return "t"
    if "celsius" in name or "fahrenheit" in name or "thermo" in name:
        return "T"
    if "umbrella" in name:
        return "u"
    return "*"


def categorize(name: str) -> str:
    rules: list[tuple[str, Callable[[str], bool]]] = [
        ("Day", lambda n: n.startswith("day_")),
        ("NightAlt", lambda n: n.startswith("night_alt_")),
        ("Night", lambda n: n.startswith("night_")),
        ("Moon", lambda n: n.startswith("moon_")),
        ("Time", lambda n: n.startswith("time_")),
        ("Direction", lambda n: n.startswith("direction_")),
        ("Wind", lambda n: n.startswith("wind_") or n in ("windy", "strong_wind")),
        (
            "Storm",
            lambda n: n
            in (
                "tornado",
                "hurricane",
                "hurricane_warning",
                "thunderstorm",
                "lightning",
                "storm_showers",
            )
            or "storm" in n,
        ),
        (
            "Cloud",
            lambda n: (n.startswith("cloud") or n in ("cloudy", "cloudy_gusts", "cloudy_windy", "fog", "hail"))
            and not n.startswith(("day_", "night")),
        ),
        (
            "Precipitation",
            lambda n: (
                any(tok in ("rain", "snow", "sleet", "sprinkle", "showers") for tok in n.split("_"))
                and not n.startswith(("day_", "night"))
            ),
        ),
        (
            "Temperature",
            lambda n: any(x in n for x in ("celsius", "fahrenheit", "degrees", "thermometer", "hot", "cold")),
        ),
        ("Pressure", lambda n: "barometer" in n or "humidity" in n),
        ("Sun", lambda n: n.startswith("sun") or n == "solar_eclipse"),
        ("Other", lambda n: n in ("na", "train", "umbrella")),
        (
            "Hazard",
            lambda n: n
            in (
                "volcano",
                "earthquake",
                "fire",
                "flood",
                "tsunami",
                "meteor",
            ),
        ),
        ("AirQuality", lambda n: n in ("dust", "smoke", "smog")),
        ("Marine", lambda n: n in ("small_craft_advisory", "gale_warning")),
        (
            "Astronomy",
            lambda n: n in ("stars", "lunar_eclipse", "moonrise", "moonset", "aliens", "alien"),
        ),
        ("Navigation", lambda n: n.startswith("horizon") or n.startswith("refresh")),
    ]
    for cat, pred in rules:
        if pred(name):
            return cat
    return "Other"


def parse_i_weather_sh(text: str) -> list[tuple[str, int]]:
    icons: list[tuple[str, int]] = []
    for line in text.splitlines():
        match = re.search(r"i='(.)' i_weather_(\w+)=\$i", line)
        if match:
            icons.append((match.group(2), ord(match.group(1))))
    return icons


def rows_for_suffixes(suffixes: list[str], icons: dict[str, int]) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    for suffix in suffixes:
        if suffix not in icons:
            raise KeyError(f"missing i_weather_{suffix}")
        category = categorize(suffix)
        rows.append(
            {
                "name": pascal_case(suffix),
                "category": category,
                "nerd_suffix": suffix,
                "path": nested_weather_path(suffix),
                "codepoint": f"{icons[suffix]:04X}",
                "ascii": ascii_for(suffix),
            }
        )
    return rows


def write_tsv(path, rows: list[dict[str, str]]) -> None:
    lines = ["name\tcategory\tnerd_suffix\tpath\tcodepoint\tascii"]
    for row in rows:
        lines.append(
            "\t".join(
                [
                    row["name"],
                    row["category"],
                    row["nerd_suffix"],
                    row["path"],
                    row["codepoint"],
                    row["ascii"],
                ]
            )
        )
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
