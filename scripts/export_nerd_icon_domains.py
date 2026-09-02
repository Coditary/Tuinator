#!/usr/bin/env python3
"""Export i_weather.sh glyphs into per-domain TSV files under data/nerd_icons/."""

from __future__ import annotations

import sys
import urllib.request
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from nerd_icon_domains import DOMAIN_ORDER, domain_for  # noqa: E402
from weather_icon_naming import NERD_FONTS_TAG, categorize, parse_i_weather_sh  # noqa: E402

OUT_DIR = ROOT / "data" / "nerd_icons"


def main() -> int:
    text = urllib.request.urlopen(
        f"https://raw.githubusercontent.com/ryanoasis/nerd-fonts/{NERD_FONTS_TAG}/bin/scripts/lib/i_weather.sh"
    ).read().decode("utf-8")
    icons = dict(parse_i_weather_sh(text))

    from nerd_icon_domains import row_from_suffix

    by_domain: dict[str, list[dict[str, str]]] = {d: [] for d in DOMAIN_ORDER}
    for suffix, cp in sorted(icons.items()):
        weather_category = categorize(suffix)
        row = row_from_suffix(suffix, cp, weather_category)
        by_domain[row["domain"]].append(row)

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    for domain in DOMAIN_ORDER:
        rows = by_domain[domain]
        path = OUT_DIR / f"{domain.lower()}.tsv"
        lines = ["name\tdomain\tcategory\tnerd_suffix\tpath\tcodepoint\tascii"]
        for row in rows:
            lines.append(
                "\t".join(
                    [
                        row["name"],
                        row["domain"],
                        row["category"],
                        row["nerd_suffix"],
                        row["path"],
                        row["codepoint"],
                        row["ascii"],
                    ]
                )
            )
        path.write_text("\n".join(lines) + "\n", encoding="utf-8")
        print(f"Wrote {path.name} ({len(rows)} icons)")

    counts = Counter(domain_for(s, categorize(s)) for s in icons)
    total = sum(counts[d] for d in DOMAIN_ORDER)
    print(f"Total exported: {total}")
    if total != 228:
        raise SystemExit(f"Expected 228 icons, got {total}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
