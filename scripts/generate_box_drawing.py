#!/usr/bin/env python3
"""Regenerate box_drawing table from data/box_drawing.tsv (Unicode box drawing block)."""

from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TSV = ROOT / "data" / "box_drawing.tsv"
HEADER = ROOT / "include" / "tuinator" / "render" / "box_drawing.hpp"
CPP = ROOT / "src" / "render" / "box_drawing.cpp"


def load_tsv() -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    for line in TSV.read_text(encoding="utf-8").splitlines():
        if not line or line.startswith("name\t"):
            continue
        name, codepoint, ascii_ch = line.split("\t")
        rows.append(
            {
                "name": name,
                "codepoint": codepoint,
                "ascii": ascii_ch,
            }
        )
    return rows


def cpp_char_literal(ch: str) -> str:
    if ch == "\\":
        return "'\\\\'"
    if ch == "'":
        return "'\\''"
    return f"'{ch}'"


def write_header(rows: list[dict[str, str]]) -> None:
    enum_lines = "\n".join(f"    {row['name']}," for row in rows)
    content = f"""#pragma once

#include <tuinator/render/glyphs.hpp>

#include <string>
#include <vector>

namespace tuinator {{

/// Unicode box-drawing characters (U+2500 block).
enum class BoxDrawing {{
{enum_lines}
}};

struct BoxDrawingDescriptor {{
    BoxDrawing kind = BoxDrawing::{rows[0]['name']};
    char32_t codepoint = 0;
    char ascii_fallback = '?';
}};

const BoxDrawingDescriptor& box_drawing_descriptor(BoxDrawing kind);

std::string box_drawing_glyph(BoxDrawing kind, GlyphSet glyphs = GlyphSet::Auto);

std::vector<BoxDrawing> all_box_drawings();

/// Build border glyphs from individual box-drawing parts.
BorderGlyphs border_glyphs_from_box_drawing(
    BoxDrawing corner_tl,
    BoxDrawing corner_tr,
    BoxDrawing corner_bl,
    BoxDrawing corner_br,
    BoxDrawing horizontal,
    BoxDrawing vertical);

}} // namespace tuinator
"""
    HEADER.write_text(content, encoding="utf-8")


def write_cpp(rows: list[dict[str, str]]) -> None:
    make_lines = []
    for row in rows:
        make_lines.append(
            "    make(BoxDrawing::{name}, 0x{cp}, {ascii}),".format(
                name=row["name"],
                cp=row["codepoint"],
                ascii=cpp_char_literal(row["ascii"]),
            )
        )

    content = f"""#include <tuinator/render/box_drawing.hpp>

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

constexpr BoxDrawingDescriptor make(BoxDrawing kind, char32_t codepoint, char ascii) {{
    return BoxDrawingDescriptor{{
        kind,
        codepoint,
        ascii,
    }};
}}

// data/box_drawing.tsv
constexpr std::array<BoxDrawingDescriptor, {len(rows)}> kDescriptors{{{{
{chr(10).join(make_lines)}
}}}};

static_assert(kDescriptors.size() == {len(rows)}, "descriptor table out of sync with BoxDrawing enum");

const BoxDrawingDescriptor& descriptor_or_default(BoxDrawing kind) {{
    const auto index = static_cast<std::size_t>(kind);
    if (index < kDescriptors.size()) {{
        return kDescriptors[index];
    }}
    return kDescriptors[static_cast<std::size_t>(BoxDrawing::Horizontal)];
}}

GlyphSet resolve_glyph_set(GlyphSet glyphs) {{
    if (glyphs != GlyphSet::Auto) {{
        return glyphs;
    }}
    return supports_unicode_text() ? GlyphSet::Unicode : GlyphSet::Ascii;
}}

std::string resize_handle_glyph(GlyphSet glyphs) {{
    glyphs = resolve_glyph_set(glyphs);
    if (glyphs == GlyphSet::Ascii) {{
        return "#";
    }}
    return "\\xe2\\x97\\xa2"; // ◢
}}

}} // namespace

const BoxDrawingDescriptor& box_drawing_descriptor(BoxDrawing kind) {{
    return descriptor_or_default(kind);
}}

std::string box_drawing_glyph(BoxDrawing kind, GlyphSet glyphs) {{
    const BoxDrawingDescriptor& descriptor = box_drawing_descriptor(kind);
    glyphs = resolve_glyph_set(glyphs);

    if (glyphs == GlyphSet::Ascii) {{
        return std::string(1, descriptor.ascii_fallback);
    }}

    return utf8_from(descriptor.codepoint);
}}

std::vector<BoxDrawing> all_box_drawings() {{
    std::vector<BoxDrawing> kinds;
    kinds.reserve(kDescriptors.size());
    for (const BoxDrawingDescriptor& descriptor : kDescriptors) {{
        kinds.push_back(descriptor.kind);
    }}
    return kinds;
}}

BorderGlyphs border_glyphs_from_box_drawing(
    BoxDrawing corner_tl,
    BoxDrawing corner_tr,
    BoxDrawing corner_bl,
    BoxDrawing corner_br,
    BoxDrawing horizontal,
    BoxDrawing vertical) {{
    const GlyphSet glyphs = GlyphSet::Auto;
    return BorderGlyphs{{
        box_drawing_glyph(corner_tl, glyphs),
        box_drawing_glyph(corner_tr, glyphs),
        box_drawing_glyph(corner_bl, glyphs),
        box_drawing_glyph(corner_br, glyphs),
        box_drawing_glyph(horizontal, glyphs),
        box_drawing_glyph(vertical, glyphs),
        resize_handle_glyph(glyphs),
    }};
}}

}} // namespace tuinator
"""
    CPP.write_text(content, encoding="utf-8")


def main() -> int:
    rows = load_tsv()
    write_header(rows)
    write_cpp(rows)
    print(f"Wrote {HEADER.name} and {CPP.name} ({len(rows)} box drawings)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
