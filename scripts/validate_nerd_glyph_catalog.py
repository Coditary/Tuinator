#!/usr/bin/env python3
"""Validate Nerd Font glyph catalog exports."""

from __future__ import annotations

import sys
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from export_nerd_glyph_catalog import collect_routed_set, collect_utility, collect_weather, merge  # noqa: E402
from nerd_font_lib import ALL_SETS, load_set  # noqa: E402

ROUTED = ("cod", "dev", "oct", "seti", "logos", "fae", "fa", "md")
MISC_LIMIT = 200


def main() -> int:
    expected = {s: len(load_set(s)) for s in ALL_SETS}
    expected_total = sum(expected.values())

    glyphs: dict[str, list] = defaultdict(list)
    merge(glyphs, collect_utility())
    for set_name in ROUTED:
        merge(glyphs, collect_routed_set(set_name))
    weather = collect_weather()

    routed_total = sum(len(v) for v in glyphs.values())
    weather_total = sum(len(v) for v in weather.values())
    actual_total = routed_total + weather_total

    print(f"Expected: {expected_total}  Routed+weather: {actual_total}")
    if actual_total != expected_total:
        print("ERROR: count mismatch", expected_total - actual_total)
        return 1

    domain_counts = Counter()
    for rows in glyphs.values():
        for r in rows:
            domain_counts[r["domain"]] += 1
    for rows in weather.values():
        for r in rows:
            domain_counts[r["domain"]] += 1

    misc = [(d, n) for d, n in domain_counts.items() if "misc" in d.lower() or d in ("Misc",)]
    misc.sort(key=lambda x: -x[1])
    print(f"Domains: {len(domain_counts)}")
    print("Misc buckets:")
    for d, n in misc:
        flag = " !" if n > MISC_LIMIT else ""
        print(f"  {d}: {n}{flag}")

    over = [x for x in misc if x[1] > MISC_LIMIT]
    if over:
        print(f"WARN: {len(over)} misc domain(s) over {MISC_LIMIT}")
    print("OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
