#!/usr/bin/env python3
"""Regenerate line_icon table from data/nerd_line_icons.tsv (Nerd Fonts v3.4.0 PLE + Unicode)."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TSV = ROOT / "data" / "nerd_line_icons.tsv"
HEADER = ROOT / "include" / "tuinator/render/line_icon.hpp"
CPP = ROOT / "src/render/line_icon.cpp"

NERD_FONTS_TAG = "v3.4.0"

CATEGORY_ENUM = {
    "Powerline": "Powerline",
    "PowerlineExtra": "PowerlineExtra",
    "BoxDrawing": "BoxDrawing",
    "BlockElement": "BlockElement",
    "Diff": "Diff",
}

SET_ENUM = {
    "ple": "LineIconSet::Powerline",
    "unicode": "LineIconSet::Unicode",
}


def load_tsv() -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    for line in TSV.read_text(encoding="utf-8").splitlines():
        if not line or line.startswith("name\t"):
            continue
        name, category, set_name, icon, codepoint, ascii_ch = line.split("\t")
        rows.append(
            {
                "name": name,
                "category": category,
                "set": set_name,
                "icon": icon,
                "codepoint": codepoint,
                "ascii": ascii_ch,
            }
        )
    return rows


def fetch_ple_codepoint(icon: str) -> int:
    import urllib.request

    text = urllib.request.urlopen(
        f"https://raw.githubusercontent.com/ryanoasis/nerd-fonts/{NERD_FONTS_TAG}/bin/scripts/lib/i_ple.sh"
    ).read().decode("utf-8")
    for prefix in ("ple", "pl"):
        var = f"i_{prefix}_{icon}"
        pattern = re.compile(rf"i='(.)' {re.escape(var)}=\$i")
        for line in text.splitlines():
            match = pattern.search(line)
            if match:
                return ord(match.group(1))
    raise KeyError(f"ple-{icon}")


def verify_rows(rows: list[dict[str, str]]) -> None:
    for row in rows:
        expected = int(row["codepoint"], 16)
        if row["set"] == "ple":
            actual = fetch_ple_codepoint(row["icon"])
            if actual != expected:
                raise SystemExit(
                    f"{row['name']}: TSV has U+{expected:04X}, nerd-fonts has U+{actual:04X}"
                )


def nerd_name_for(row: dict[str, str]) -> str:
    if row["set"] == "ple":
        return f"ple-{row['icon']}"
    return f"uni-{row['icon']}"


def write_header(rows: list[dict[str, str]]) -> None:
    categories = list(dict.fromkeys(row["category"] for row in rows))
    category_enum = "\n".join(f"    {CATEGORY_ENUM[c]}," for c in categories)

    sections: dict[str, list[str]] = {c: [] for c in categories}
    for row in rows:
        sections[row["category"]].append(f"    {row['name']},")

    enum_lines = []
    for category in categories:
        enum_lines.append(f"    // {category}")
        enum_lines.extend(sections[category])
    enum_body = "\n".join(enum_lines)

    content = f"""#pragma once

#include <tuinator/render/glyphs.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {{

/// Glyph source for LineIcon.
enum class LineIconSet {{
    Powerline,
    Unicode,
}};

/// Semantic category for line / box / block / diff glyphs.
enum class LineIconCategory {{
{category_enum}
}};

/// Typed line glyphs: Powerline ({sum(1 for r in rows if r['category'] == 'Powerline')} Nerd Fonts PLE),
/// box drawing, block shades, and diff helpers ({NERD_FONTS_TAG} + Unicode).
enum class LineIcon {{
{enum_body}
}};

struct LineIconDescriptor {{
    LineIcon kind = LineIcon::{rows[0]['name']};
    LineIconCategory category = LineIconCategory::{CATEGORY_ENUM[rows[0]['category']]};
    LineIconSet set = LineIconSet::Powerline;
    const char* nerd_name = "";
    char32_t codepoint = 0;
    char ascii_fallback = '?';
}};

const LineIconDescriptor& line_icon_descriptor(LineIcon icon);

LineIconCategory line_icon_category(LineIcon icon);

LineIconSet line_icon_set(LineIcon icon);

/// Cheat-sheet name, e.g. `ple-right_half_circle_thin` or `uni-horizontal`.
const char* line_icon_nerd_name(LineIcon icon);

std::optional<LineIcon> line_icon_from_nerd_name(std::string_view nerd_name);

/// All icons in a category (enum order within the category).
std::vector<LineIcon> line_icons_in_category(LineIconCategory category);

std::string line_icon_glyph(LineIcon icon, GlyphSet glyphs = GlyphSet::Auto);

}} // namespace tuinator
"""
    HEADER.write_text(content, encoding="utf-8")


def cpp_char_literal(ch: str) -> str:
    if ch == "\\":
        return "'\\\\'"
    if ch == "'":
        return "'\\''"
    return f"'{ch}'"


def write_cpp(rows: list[dict[str, str]]) -> None:
    make_lines = []
    for row in rows:
        set_enum = SET_ENUM[row["set"]]
        category_enum = f"LineIconCategory::{CATEGORY_ENUM[row['category']]}"
        nerd = nerd_name_for(row)
        make_lines.append(
            "    make(LineIcon::{name}, {category}, {set}, \"{nerd}\", 0x{cp}, {ascii}),".format(
                name=row["name"],
                category=category_enum,
                set=set_enum,
                nerd=nerd,
                cp=row["codepoint"],
                ascii=cpp_char_literal(row["ascii"]),
            )
        )

    content = f"""#include <tuinator/render/line_icon.hpp>

#include <tuinator/render/glyphs.hpp>

#include <array>
#include <cstring>
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

constexpr LineIconDescriptor make(
    LineIcon kind,
    LineIconCategory category,
    LineIconSet set,
    const char* nerd_name,
    char32_t codepoint,
    char ascii) {{
    return LineIconDescriptor{{
        kind,
        category,
        set,
        nerd_name,
        codepoint,
        ascii,
    }};
}}

// data/nerd_line_icons.tsv ({NERD_FONTS_TAG} PLE + Unicode box/block/diff).
constexpr std::array<LineIconDescriptor, {len(rows)}> kDescriptors{{{{
{chr(10).join(make_lines)}
}}}};

static_assert(kDescriptors.size() == {len(rows)}, "descriptor table out of sync with LineIcon enum");

const LineIconDescriptor& descriptor_or_default(LineIcon icon) {{
    const auto index = static_cast<std::size_t>(icon);
    if (index < kDescriptors.size()) {{
        return kDescriptors[index];
    }}
    return kDescriptors[static_cast<std::size_t>(LineIcon::BoxHorizontal)];
}}

GlyphSet resolve_glyph_set(const LineIconDescriptor& descriptor, GlyphSet glyphs) {{
    if (glyphs != GlyphSet::Auto) {{
        return glyphs;
    }}

    if (descriptor.set == LineIconSet::Powerline) {{
        return detect_file_icon_glyph_set();
    }}

    return supports_unicode_text() ? GlyphSet::Unicode : GlyphSet::Ascii;
}}

}} // namespace

const LineIconDescriptor& line_icon_descriptor(LineIcon icon) {{
    return descriptor_or_default(icon);
}}

LineIconCategory line_icon_category(LineIcon icon) {{
    return line_icon_descriptor(icon).category;
}}

LineIconSet line_icon_set(LineIcon icon) {{
    return line_icon_descriptor(icon).set;
}}

const char* line_icon_nerd_name(LineIcon icon) {{
    return line_icon_descriptor(icon).nerd_name;
}}

std::optional<LineIcon> line_icon_from_nerd_name(std::string_view nerd_name) {{
    for (const LineIconDescriptor& descriptor : kDescriptors) {{
        if (nerd_name == descriptor.nerd_name) {{
            return descriptor.kind;
        }}
    }}
    return std::nullopt;
}}

std::vector<LineIcon> line_icons_in_category(LineIconCategory category) {{
    std::vector<LineIcon> icons;
    icons.reserve(kDescriptors.size());
    for (const LineIconDescriptor& descriptor : kDescriptors) {{
        if (descriptor.category == category) {{
            icons.push_back(descriptor.kind);
        }}
    }}
    return icons;
}}

std::string line_icon_glyph(LineIcon icon, GlyphSet glyphs) {{
    const LineIconDescriptor& descriptor = line_icon_descriptor(icon);
    glyphs = resolve_glyph_set(descriptor, glyphs);

    if (glyphs == GlyphSet::Ascii) {{
        return std::string(1, descriptor.ascii_fallback);
    }}

    return utf8_from(descriptor.codepoint);
}}

}} // namespace tuinator
"""
    CPP.write_text(content, encoding="utf-8")


def main() -> int:
    verify = "--verify" in sys.argv
    rows = load_tsv()
    if verify:
        verify_rows(rows)
        print(f"Verified {len(rows)} line icons against nerd-fonts {NERD_FONTS_TAG}")
    write_header(rows)
    write_cpp(rows)
    print(f"Wrote {HEADER.name} and {CPP.name} ({len(rows)} icons)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
