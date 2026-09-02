#!/usr/bin/env python3
"""Export all Nerd Font glyphs into semantic domain TSV files."""

from __future__ import annotations

import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from nerd_font_lib import NERD_FONTS_TAG, load_set  # noqa: E402
from nerd_glyph_domains_utility import (  # noqa: E402
    PLE_DOMAIN_ORDER,
    iec_row,
    ple_row,
    pomodoro_row,
    progress_row,
)
from nerd_icon_domains import DOMAIN_ORDER, row_from_suffix  # noqa: E402
from weather_icon_naming import categorize, parse_i_weather_sh  # noqa: E402

OUT_WEATHER = ROOT / "data" / "nerd_icons"
OUT_GLYPHS = ROOT / "data" / "nerd_glyphs"

ROUTERS = {
    "cod": ("nerd_glyph_domains_cod", "cod_row"),
    "dev": ("nerd_glyph_domains_dev", "dev_row"),
    "oct": ("nerd_glyph_domains_oct", "oct_row"),
    "seti": ("nerd_glyph_domains_seti", "seti_row"),
    "logos": ("nerd_glyph_domains_logos", "logos_row"),
    "fae": ("nerd_glyph_domains_fae", "fae_row"),
    "fa": ("nerd_glyph_domains_fa", "fa_row"),
    "md": ("nerd_glyph_domains_md", "md_row"),
}


def write_domain_tsv(path: Path, rows: list[dict[str, str]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    lines = ["name\tdomain\tcategory\tnerd_suffix\tpath\tcodepoint\tascii\tsource_set"]
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
                    row.get("source_set", ""),
                ]
            )
        )
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def flush_domains(by_domain: dict[str, list[dict[str, str]]], out_dir: Path) -> int:
    total = 0
    for domain in sorted(by_domain):
        rows = sorted(by_domain[domain], key=lambda r: (r["category"], r["name"]))
        path = out_dir / f"{domain.lower()}.tsv"
        write_domain_tsv(path, rows)
        print(f"Wrote {path.relative_to(ROOT)} ({len(rows)} icons)")
        total += len(rows)
    return total


def collect_weather() -> dict[str, list[dict[str, str]]]:
    text = __import__("urllib.request").request.urlopen(
        f"https://raw.githubusercontent.com/ryanoasis/nerd-fonts/{NERD_FONTS_TAG}/bin/scripts/lib/i_weather.sh"
    ).read().decode()
    icons = dict(parse_i_weather_sh(text))
    by_domain: dict[str, list[dict[str, str]]] = {d: [] for d in DOMAIN_ORDER}
    for suffix, cp in sorted(icons.items()):
        row = row_from_suffix(suffix, cp, categorize(suffix))
        row["source_set"] = "weather"
        by_domain[row["domain"]].append(row)
    return by_domain


def collect_utility() -> dict[str, list[dict[str, str]]]:
    by_domain: dict[str, list[dict[str, str]]] = {d: [] for d in PLE_DOMAIN_ORDER}
    for suffix, cp in sorted(load_set("ple").items()):
        by_domain[ple_row(suffix, cp)["domain"]].append(ple_row(suffix, cp))
    for suffix, cp in sorted(load_set("iec").items()):
        by_domain[iec_row(suffix, cp)["domain"]].append(iec_row(suffix, cp))
    for suffix, cp in sorted(load_set("extra").items()):
        by_domain[progress_row(suffix, cp)["domain"]].append(progress_row(suffix, cp))
    for suffix, cp in sorted(load_set("pom").items()):
        by_domain[pomodoro_row(suffix, cp)["domain"]].append(pomodoro_row(suffix, cp))
    return by_domain


def collect_routed_set(set_name: str) -> dict[str, list[dict[str, str]]]:
    mod_name, row_fn_name = ROUTERS[set_name]
    mod = __import__(mod_name, fromlist=[row_fn_name])
    row_fn = getattr(mod, row_fn_name)
    by_domain: dict[str, list[dict[str, str]]] = defaultdict(list)
    for suffix, cp in sorted(load_set(set_name).items()):
        row = row_fn(suffix, cp)
        by_domain[row["domain"]].append(row)
    return by_domain


def merge(into: dict[str, list[dict[str, str]]], src: dict[str, list[dict[str, str]]]) -> None:
    for domain, rows in src.items():
        into.setdefault(domain, []).extend(rows)


def main() -> int:
    sets = sys.argv[1:] if len(sys.argv) > 1 else ["all"]
    if "all" in sets:
        sets = ["weather", "utility", *ROUTERS.keys()]

    weather_domains: dict[str, list[dict[str, str]]] = {}
    glyph_domains: dict[str, list[dict[str, str]]] = defaultdict(list)

    if "weather" in sets:
        weather_domains = collect_weather()
        flush_domains(weather_domains, OUT_WEATHER)

    if "utility" in sets:
        merge(glyph_domains, collect_utility())

    for set_name in sets:
        if set_name in ROUTERS:
            merge(glyph_domains, collect_routed_set(set_name))

    if glyph_domains:
        flush_domains(glyph_domains, OUT_GLYPHS)

    weather_total = sum(len(v) for v in weather_domains.values())
    glyph_total = sum(len(v) for v in glyph_domains.values())
    print(f"Total exported rows: {weather_total + glyph_total}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
