#!/usr/bin/env python3
"""Generate C++ icon tables from data/nerd_icons/<domain>.tsv files."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from nerd_icon_domains import (  # noqa: E402
    DOMAIN_CATEGORY_ENUM,
    DOMAIN_CPP_STEM,
    DOMAIN_ENUM,
    DOMAIN_ORDER,
    DOMAIN_PATH_PREFIX,
    category_order_for_domain,
    category_path_for,
)

DATA_DIR = ROOT / "data" / "nerd_icons"
NERD_FONTS_TAG = "v3.4.0"

CATEGORY_LABELS = {
    "Day": "Day",
    "NightAlt": "Night Alt",
    "Night": "Night",
    "Moon": "Moon",
    "MoonAlt": "Moon Alt",
    "Wind": "Wind",
    "WindDirection": "Wind Direction",
    "Storm": "Storm",
    "Cloud": "Cloud",
    "Precipitation": "Precipitation",
    "Direction": "Direction",
    "Sun": "Sun",
    "Clock": "Clock",
    "Measurement": "Measurement",
    "Astronomy": "Astronomy",
    "AirQuality": "Air Quality",
    "Hazard": "Hazard",
    "MarineAlert": "Marine Alert",
    "Action": "Action",
    "Equipment": "Equipment",
    "Misc": "Misc",
    "Unavailable": "Unavailable",
    "Celestial": "Celestial",
    "Eclipse": "Eclipse",
    "Sky": "Sky",
    "Misc": "Misc",
}


def cpp_char_literal(ch: str) -> str:
    if ch == "\\":
        return "'\\\\'"
    if ch == "'":
        return "'\\''"
    return f"'{ch}'"


def api_prefix(stem: str) -> str:
    return stem


def all_icons_fn(stem: str) -> str:
    base = stem[: -len("_icon")] if stem.endswith("_icon") else stem
    return f"all_{base}_icons"


def icons_in_category_fn(stem: str) -> str:
    base = stem[: -len("_icon")] if stem.endswith("_icon") else stem
    return f"{base}_icons_in_category"


def load_domain_rows(domain: str) -> list[dict[str, str]]:
    path = DATA_DIR / f"{domain.lower()}.tsv"
    if not path.exists():
        raise SystemExit(f"Missing {path}")
    rows: list[dict[str, str]] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line or line.startswith("name\t"):
            continue
        parts = line.split("\t")
        if len(parts) == 7:
            name, row_domain, category, nerd_suffix, path_value, codepoint, ascii_ch = parts
        elif len(parts) >= 8:
            name, row_domain, category, nerd_suffix, path_value, codepoint, ascii_ch, *_rest = parts
        else:
            raise SystemExit(f"Bad row in {path}: {line!r}")
        if row_domain != domain:
            raise SystemExit(f"Domain mismatch in {path}: {row_domain} != {domain}")
        rows.append(
            {
                "name": name,
                "category": category,
                "nerd_suffix": nerd_suffix,
                "path": path_value,
                "codepoint": codepoint,
                "ascii": ascii_ch,
            }
        )
    cat_rank = {c: i for i, c in enumerate(category_order_for_domain(domain))}
    rows.sort(key=lambda r: (cat_rank.get(r["category"], 999), r["name"]))
    return rows


def fetch_codepoint(suffix: str) -> int:
    import urllib.request

    text = urllib.request.urlopen(
        f"https://raw.githubusercontent.com/ryanoasis/nerd-fonts/{NERD_FONTS_TAG}/bin/scripts/lib/i_weather.sh"
    ).read().decode("utf-8")
    var = f"i_weather_{suffix}"
    pattern = re.compile(rf"i='(.)' {re.escape(var)}=\$i")
    for line in text.splitlines():
        match = pattern.search(line)
        if match:
            return ord(match.group(1))
    raise KeyError(suffix)


def verify_rows(rows: list[dict[str, str]]) -> None:
    for row in rows:
        expected = int(row["codepoint"], 16)
        actual = fetch_codepoint(row["nerd_suffix"])
        if actual != expected:
            raise SystemExit(
                f"{row['name']}: TSV has U+{expected:04X}, nerd-fonts has U+{actual:04X}"
            )


def write_domain(domain: str, rows: list[dict[str, str]]) -> None:
    stem = DOMAIN_CPP_STEM[domain]
    enum_name = DOMAIN_ENUM[domain]
    category_enum = DOMAIN_CATEGORY_ENUM[domain]
    prefix = api_prefix(stem)
    categories = category_order_for_domain(domain)
    category_enum_body = "\n".join(f"    {c}," for c in categories)

    enum_lines: list[str] = []
    current_cat = None
    for row in rows:
        if row["category"] != current_cat:
            current_cat = row["category"]
            enum_lines.append(f"    // {current_cat}")
        enum_lines.append(f"    {row['name']},")
    enum_body = "\n".join(enum_lines)

    header = ROOT / "include" / "tuinator/render" / f"{stem}.hpp"
    cpp = ROOT / "src" / "render" / f"{stem}.cpp"

    header.write_text(
        f"""#pragma once

#include <tuinator/render/glyphs.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {{

enum class {category_enum} {{
{category_enum_body}
}};

/// {len(rows)} icons from Nerd Fonts weather set ({domain}, {NERD_FONTS_TAG}).
enum class {enum_name} {{
{enum_body}
}};

struct {enum_name}Descriptor {{
    {enum_name} kind = {enum_name}::{rows[0]["name"]};
    {category_enum} category = {category_enum}::{rows[0]["category"]};
    const char* path = "";
    const char* nerd_suffix = "";
    char32_t codepoint = 0;
    char ascii_fallback = '?';
}};

const {enum_name}Descriptor& {prefix}_descriptor({enum_name} icon);
{category_enum} {prefix}_category({enum_name} icon);
const char* {prefix}_path({enum_name} icon);
const char* {prefix}_nerd_suffix({enum_name} icon);
const char* {prefix}_category_path({category_enum} category);
const char* {prefix}_category_label({category_enum} category);
std::vector<std::string_view> {prefix}_path_segments({enum_name} icon);
std::optional<{enum_name}> {prefix}_from_path(std::string_view path);
std::vector<{enum_name}> {icons_in_category_fn(stem)}({category_enum} category);
std::vector<{category_enum}> all_{prefix}_categories();
std::vector<{enum_name}> {all_icons_fn(stem)}();
std::string {prefix}_glyph({enum_name} icon, GlyphSet glyphs = GlyphSet::Auto);

}} // namespace tuinator
""",
        encoding="utf-8",
    )

    make_lines = []
    for row in rows:
        make_lines.append(
            f'    make({enum_name}::{row["name"]}, {category_enum}::{row["category"]}, '
            f'"{row["path"]}", "{row["nerd_suffix"]}", 0x{row["codepoint"]}, '
            f"{cpp_char_literal(row['ascii'])}),"
        )

    category_switch = f"const char* {prefix}_category_label({category_enum} category) {{\n    switch (category) {{\n"
    for cat in categories:
        label = CATEGORY_LABELS.get(cat, cat)
        category_switch += f"    case {category_enum}::{cat}:\n        return \"{label}\";\n"
    category_switch += '    }\n    return "Other";\n}\n'

    category_path_switch = (
        f"const char* {prefix}_category_path({category_enum} category) {{\n    switch (category) {{\n"
    )
    for cat in categories:
        cat_path = category_path_for(domain, cat)
        category_path_switch += f'    case {category_enum}::{cat}:\n        return "{cat_path}";\n'
    category_path_switch += f'    }}\n    return "{DOMAIN_PATH_PREFIX[domain]}";\n}}\n'

    category_push = "".join(f"    categories.push_back({category_enum}::{cat});\n" for cat in categories)

    cpp.write_text(
        f"""#include <tuinator/render/{stem}.hpp>

#include <tuinator/render/glyphs.hpp>

#include <array>
#include <string>

namespace tuinator {{

namespace {{

std::string utf8_from(char32_t cp) {{
    std::string out;
    if (cp < 0x80) {{
        out.push_back(static_cast<char>(cp));
    }} else if (cp < 0x800) {{
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }} else if (cp < 0x10000) {{
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }} else {{
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }}
    return out;
}}

constexpr {enum_name}Descriptor make(
    {enum_name} kind,
    {category_enum} category,
    const char* path,
    const char* nerd_suffix,
    char32_t codepoint,
    char ascii) {{
    return {enum_name}Descriptor{{kind, category, path, nerd_suffix, codepoint, ascii}};
}}

constexpr std::array<{enum_name}Descriptor, {len(rows)}> kDescriptors{{{{
{chr(10).join(make_lines)}
}}}};

static_assert(kDescriptors.size() == {len(rows)}, "descriptor table out of sync");

const {enum_name}Descriptor& descriptor_or_default({enum_name} icon) {{
    const auto index = static_cast<std::size_t>(icon);
    if (index < kDescriptors.size()) {{
        return kDescriptors[index];
    }}
    return kDescriptors[0];
}}

}} // namespace

const {enum_name}Descriptor& {prefix}_descriptor({enum_name} icon) {{
    return descriptor_or_default(icon);
}}

{category_enum} {prefix}_category({enum_name} icon) {{
    return {prefix}_descriptor(icon).category;
}}

const char* {prefix}_path({enum_name} icon) {{
    return {prefix}_descriptor(icon).path;
}}

const char* {prefix}_nerd_suffix({enum_name} icon) {{
    return {prefix}_descriptor(icon).nerd_suffix;
}}

{category_path_switch}
{category_switch}
std::vector<std::string_view> {prefix}_path_segments({enum_name} icon) {{
    const char* path = {prefix}_path(icon);
    std::vector<std::string_view> segments;
    if (path == nullptr || path[0] == '\\0') {{
        return segments;
    }}
    const char* start = path;
    for (const char* cursor = path; *cursor != '\\0'; ++cursor) {{
        if (*cursor != '-') {{
            continue;
        }}
        segments.emplace_back(start, static_cast<std::size_t>(cursor - start));
        start = cursor + 1;
    }}
    segments.emplace_back(start);
    return segments;
}}

std::optional<{enum_name}> {prefix}_from_path(std::string_view query) {{
    for (const {enum_name}Descriptor& descriptor : kDescriptors) {{
        if (query == descriptor.path) {{
            return descriptor.kind;
        }}
    }}
    for (const {enum_name}Descriptor& descriptor : kDescriptors) {{
        const std::string legacy = std::string("{DOMAIN_PATH_PREFIX[domain]}-") + descriptor.nerd_suffix;
        if (query == legacy) {{
            return descriptor.kind;
        }}
    }}
    for (const {enum_name}Descriptor& descriptor : kDescriptors) {{
        if (query == descriptor.nerd_suffix) {{
            return descriptor.kind;
        }}
    }}
    return std::nullopt;
}}

std::vector<{enum_name}> {icons_in_category_fn(stem)}({category_enum} category) {{
    std::vector<{enum_name}> icons;
    icons.reserve(kDescriptors.size());
    for (const {enum_name}Descriptor& descriptor : kDescriptors) {{
        if (descriptor.category == category) {{
            icons.push_back(descriptor.kind);
        }}
    }}
    return icons;
}}

std::vector<{category_enum}> all_{prefix}_categories() {{
    std::vector<{category_enum}> categories;
    categories.reserve({len(categories)});
{category_push}    return categories;
}}

std::vector<{enum_name}> {all_icons_fn(stem)}() {{
    std::vector<{enum_name}> icons;
    icons.reserve(kDescriptors.size());
    for (const {enum_name}Descriptor& descriptor : kDescriptors) {{
        icons.push_back(descriptor.kind);
    }}
    return icons;
}}

std::string {prefix}_glyph({enum_name} icon, GlyphSet glyphs) {{
    const {enum_name}Descriptor& descriptor = {prefix}_descriptor(icon);
    if (glyphs == GlyphSet::Auto) {{
        glyphs = detect_file_icon_glyph_set();
    }}
    if (glyphs == GlyphSet::Ascii) {{
        return std::string(1, descriptor.ascii_fallback);
    }}
    return utf8_from(descriptor.codepoint);
}}

}} // namespace tuinator
""",
        encoding="utf-8",
    )

    print(f"Wrote {stem}.hpp/.cpp ({len(rows)} icons)")


def write_nerd_icon_set_header() -> None:
    cases = "\n".join(f"    {domain}," for domain in DOMAIN_ORDER)
    path = ROOT / "include" / "tuinator/render/nerd_icon_set.hpp"
    path.write_text(
        f"""#pragma once

namespace tuinator {{

/// Semantic domains for glyphs sourced from Nerd Fonts `i_weather.sh` ({NERD_FONTS_TAG}).
enum class NerdIconSet {{
{cases}
}};

const char* nerd_icon_set_label(NerdIconSet set);
const char* nerd_icon_set_path_prefix(NerdIconSet set);

}} // namespace tuinator
""",
        encoding="utf-8",
    )

    cpp = ROOT / "src" / "render/nerd_icon_set.cpp"
    label_switch = "const char* nerd_icon_set_label(NerdIconSet set) {\n    switch (set) {\n"
    prefix_switch = "const char* nerd_icon_set_path_prefix(NerdIconSet set) {\n    switch (set) {\n"
    for domain in DOMAIN_ORDER:
        label_switch += f'    case NerdIconSet::{domain}:\n        return "{domain}";\n'
        prefix_switch += (
            f'    case NerdIconSet::{domain}:\n        return "{DOMAIN_PATH_PREFIX[domain]}";\n'
        )
    label_switch += '    }\n    return "Unknown";\n}\n'
    prefix_switch += '    }\n    return "";\n}\n'
    cpp.write_text(
        f"""#include <tuinator/render/nerd_icon_set.hpp>

namespace tuinator {{

{label_switch}
{prefix_switch}
}} // namespace tuinator
""",
        encoding="utf-8",
    )


def main() -> int:
    verify = "--verify" in sys.argv
    domains = sys.argv[1:] if len(sys.argv) > 1 and not sys.argv[1].startswith("-") else DOMAIN_ORDER

    if not DATA_DIR.exists():
        raise SystemExit("Run export_nerd_icon_domains.py first")

    total = 0
    for domain in domains:
        rows = load_domain_rows(domain)
        if verify:
            verify_rows(rows)
        write_domain(domain, rows)
        total += len(rows)

    if domains == DOMAIN_ORDER:
        write_nerd_icon_set_header()
        print(f"Total icons generated: {total}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
