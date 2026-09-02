#!/usr/bin/env python3
"""Regenerate ui_icon table from data/nerd_ui_icons.tsv (Nerd Fonts v3.4.0 codepoints)."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TSV = ROOT / "data" / "nerd_ui_icons.tsv"
HEADER = ROOT / "include" / "tuinator" / "render" / "ui_icon.hpp"
CPP = ROOT / "src" / "render" / "ui_icon.cpp"

NERD_FONTS_TAG = "v3.4.0"


def load_tsv() -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    for line in TSV.read_text(encoding="utf-8").splitlines():
        if not line or line.startswith("name\t"):
            continue
        name, set_name, icon, codepoint, ascii_ch, r, g, b = line.split("\t")
        rows.append(
            {
                "name": name,
                "set": set_name,
                "icon": icon,
                "codepoint": codepoint,
                "ascii": ascii_ch,
                "r": r,
                "g": g,
                "b": b,
            }
        )
    return rows


def fetch_codepoint(set_name: str, icon: str) -> int:
    import urllib.request

    text = urllib.request.urlopen(
        f"https://raw.githubusercontent.com/ryanoasis/nerd-fonts/{NERD_FONTS_TAG}/bin/scripts/lib/i_{set_name}.sh"
    ).read().decode("utf-8")
    var = f"i_{set_name}_{icon}"
    pattern = re.compile(rf"i='(.)' {re.escape(var)}=\$i")
    for line in text.splitlines():
        match = pattern.search(line)
        if match:
            return ord(match.group(1))
    raise KeyError(f"{set_name}-{icon} ({var})")


def verify_rows(rows: list[dict[str, str]]) -> None:
    for row in rows:
        expected = int(row["codepoint"], 16)
        actual = fetch_codepoint(row["set"], row["icon"])
        if actual != expected:
            raise SystemExit(
                f"{row['name']}: TSV has U+{expected:04X}, nerd-fonts has U+{actual:04X}"
            )


def write_header(rows: list[dict[str, str]]) -> None:
    enum_lines = "\n".join(f"    {row['name']}," for row in rows)
    content = f"""#pragma once

#include <tuinator/render/color.hpp>
#include <tuinator/render/glyphs.hpp>
#include <tuinator/render/style.hpp>

#include <optional>
#include <string>
#include <string_view>

namespace tuinator {{

/// Nerd Fonts glyph set (see nerd-fonts bin/scripts/lib/i_*.sh).
enum class UiIconSet {{
    Codicon,
    Devicon,
}};

/// UI / tool / OS symbols from Nerd Fonts ({NERD_FONTS_TAG}).
enum class UiIcon {{
{enum_lines}
}};

struct UiIconDescriptor {{
    UiIcon kind = UiIcon::{rows[0]['name']};
    UiIconSet set = UiIconSet::Codicon;
    const char* nerd_name = "";
    char32_t codepoint = 0;
    char ascii_fallback = '?';
    Rgb color{{0xC0, 0xCA, 0xF5}};
}};

const UiIconDescriptor& ui_icon_descriptor(UiIcon icon);

/// Cheat-sheet name, e.g. `cod-diff_added` or `dev-linux`.
const char* ui_icon_nerd_name(UiIcon icon);

UiIconSet ui_icon_set(UiIcon icon);

std::optional<UiIcon> ui_icon_from_nerd_name(std::string_view nerd_name);

std::string ui_icon_glyph(UiIcon icon, GlyphSet glyphs = GlyphSet::Auto);

Rgb ui_icon_color(UiIcon icon);

Style ui_icon_style(UiIcon icon, GlyphSet glyphs = GlyphSet::Auto);

}} // namespace tuinator
"""
    HEADER.write_text(content, encoding="utf-8")


def write_cpp(rows: list[dict[str, str]]) -> None:
    set_map = {"cod": "UiIconSet::Codicon", "dev": "UiIconSet::Devicon"}
    make_lines = []
    for row in rows:
        set_enum = set_map[row["set"]]
        nerd = f"{row['set']}-{row['icon']}"
        make_lines.append(
            "    make(UiIcon::{name}, {set}, \"{nerd}\", 0x{cp}, '{ascii}', 0x{r}, 0x{g}, 0x{b}),".format(
                name=row["name"],
                set=set_enum,
                nerd=nerd,
                cp=row["codepoint"],
                ascii=row["ascii"],
                r=row["r"],
                g=row["g"],
                b=row["b"],
            )
        )

    content = f"""#include <tuinator/render/ui_icon.hpp>

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

constexpr UiIconDescriptor make(
    UiIcon kind,
    UiIconSet set,
    const char* nerd_name,
    char32_t codepoint,
    char ascii,
    std::uint8_t r,
    std::uint8_t g,
    std::uint8_t b) {{
    return UiIconDescriptor{{
        kind,
        set,
        nerd_name,
        codepoint,
        ascii,
        Rgb{{r, g, b}},
    }};
}}

// Codepoints from Nerd Fonts {NERD_FONTS_TAG} (data/nerd_ui_icons.tsv).
constexpr std::array<UiIconDescriptor, {len(rows)}> kDescriptors{{{{
{chr(10).join(make_lines)}
}}}};

static_assert(kDescriptors.size() == {len(rows)}, "descriptor table out of sync with UiIcon enum");

const UiIconDescriptor& descriptor_or_default(UiIcon icon) {{
    const auto index = static_cast<std::size_t>(icon);
    if (index < kDescriptors.size()) {{
        return kDescriptors[index];
    }}
    return kDescriptors[static_cast<std::size_t>(UiIcon::Info)];
}}

}} // namespace

const UiIconDescriptor& ui_icon_descriptor(UiIcon icon) {{
    return descriptor_or_default(icon);
}}

const char* ui_icon_nerd_name(UiIcon icon) {{
    return ui_icon_descriptor(icon).nerd_name;
}}

UiIconSet ui_icon_set(UiIcon icon) {{
    return ui_icon_descriptor(icon).set;
}}

std::optional<UiIcon> ui_icon_from_nerd_name(std::string_view nerd_name) {{
    for (const UiIconDescriptor& descriptor : kDescriptors) {{
        if (nerd_name == descriptor.nerd_name) {{
            return descriptor.kind;
        }}
    }}
    return std::nullopt;
}}

std::string ui_icon_glyph(UiIcon icon, GlyphSet glyphs) {{
    const UiIconDescriptor& descriptor = ui_icon_descriptor(icon);

    if (glyphs == GlyphSet::Auto) {{
        glyphs = detect_file_icon_glyph_set();
    }}

    if (glyphs == GlyphSet::Ascii) {{
        return std::string(1, descriptor.ascii_fallback);
    }}

    return utf8_from(descriptor.codepoint);
}}

Rgb ui_icon_color(UiIcon icon) {{
    return ui_icon_descriptor(icon).color;
}}

Style ui_icon_style(UiIcon icon, GlyphSet glyphs) {{
    (void)glyphs;
    return style_fg(ui_icon_color(icon));
}}

}} // namespace tuinator
"""
    CPP.write_text(content, encoding="utf-8")


def main() -> int:
    verify = "--verify" in sys.argv
    rows = load_tsv()
    if verify:
        verify_rows(rows)
        print(f"Verified {len(rows)} icons against nerd-fonts {NERD_FONTS_TAG}")
    write_header(rows)
    write_cpp(rows)
    print(f"Wrote {HEADER.name} and {CPP.name} ({len(rows)} icons)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
