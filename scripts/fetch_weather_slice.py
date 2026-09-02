#!/usr/bin/env python3
"""Fetch a weather icon slice from nerd-fonts and write data/weather/<output>.tsv"""

from __future__ import annotations

import sys
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from weather_icon_naming import (  # noqa: E402
    NERD_FONTS_TAG,
    categorize,
    nested_weather_path,
    parse_i_weather_sh,
    pascal_case,
    ascii_for,
    write_tsv,
)

SLICES: dict[str, list[str]] = {
    "slice1_day_night.tsv": ["Day", "NightAlt", "Night"],
    "slice2_moon.tsv": ["Moon"],
    "slice3_wind_storm_cloud_precip.tsv": ["Wind", "Storm", "Cloud", "Precipitation"],
    "slice4_misc.tsv": [
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
    ],
}


def main() -> int:
    if len(sys.argv) != 2 or sys.argv[1] not in SLICES:
        print(f"Usage: {sys.argv[0]} <{'|'.join(SLICES)}>", file=sys.stderr)
        return 1

    output = sys.argv[1]
    categories = set(SLICES[output])

    text = urllib.request.urlopen(
        f"https://raw.githubusercontent.com/ryanoasis/nerd-fonts/{NERD_FONTS_TAG}/bin/scripts/lib/i_weather.sh"
    ).read().decode("utf-8")
    icons = dict(parse_i_weather_sh(text))

    rows = []
    for suffix, cp in sorted(icons.items(), key=lambda x: x[0]):
        cat = categorize(suffix)
        if cat not in categories:
            continue
        rows.append(
            {
                "name": pascal_case(suffix),
                "category": cat,
                "nerd_suffix": suffix,
                "path": nested_weather_path(suffix),
                "codepoint": f"{cp:04X}",
                "ascii": ascii_for(suffix),
            }
        )

    out_path = ROOT / "data" / "weather" / output
    write_tsv(out_path, rows)
    print(f"Wrote {out_path} ({len(rows)} icons, categories: {sorted(categories)})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
