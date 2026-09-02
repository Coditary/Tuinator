#!/usr/bin/env python3
"""Regenerate block_element table from data/block_elements.tsv (Unicode block elements)."""

from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TSV = ROOT / "data" / "block_elements.tsv"
HEADER = ROOT / "include" / "tuinator" / "render" / "block_element.hpp"
CPP = ROOT / "src" / "render" / "block_element.cpp"


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


def write_header(rows: list[dict[str, str]]) -> None:
    enum_lines = "\n".join(f"    {row['name']}," for row in rows)
    content = f"""#pragma once

#include <tuinator/render/glyphs.hpp>

#include <string>
#include <vector>

namespace tuinator {{

/// Unicode block elements: full/half blocks, shades, and partial fills.
enum class BlockElement {{
{enum_lines}
}};

struct BlockElementDescriptor {{
    BlockElement kind = BlockElement::{rows[0]['name']};
    char32_t codepoint = 0;
    char ascii_fallback = '?';
}};

const BlockElementDescriptor& block_element_descriptor(BlockElement element);

std::string block_element_glyph(BlockElement element, GlyphSet glyphs = GlyphSet::Auto);

/// All block elements in enum order.
std::vector<BlockElement> all_block_elements();

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
        make_lines.append(
            "    make(BlockElement::{name}, 0x{cp}, {ascii}),".format(
                name=row["name"],
                cp=row["codepoint"],
                ascii=cpp_char_literal(row["ascii"]),
            )
        )

    content = f"""#include <tuinator/render/block_element.hpp>

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

constexpr BlockElementDescriptor make(
    BlockElement kind,
    char32_t codepoint,
    char ascii) {{
    return BlockElementDescriptor{{
        kind,
        codepoint,
        ascii,
    }};
}}

// data/block_elements.tsv (Unicode block elements).
constexpr std::array<BlockElementDescriptor, {len(rows)}> kDescriptors{{{{
{chr(10).join(make_lines)}
}}}};

static_assert(kDescriptors.size() == {len(rows)}, "descriptor table out of sync with BlockElement enum");

const BlockElementDescriptor& descriptor_or_default(BlockElement element) {{
    const auto index = static_cast<std::size_t>(element);
    if (index < kDescriptors.size()) {{
        return kDescriptors[index];
    }}
    return kDescriptors[static_cast<std::size_t>(BlockElement::Full)];
}}

GlyphSet resolve_glyph_set(GlyphSet glyphs) {{
    if (glyphs != GlyphSet::Auto) {{
        return glyphs;
    }}
    return supports_unicode_text() ? GlyphSet::Unicode : GlyphSet::Ascii;
}}

}} // namespace

const BlockElementDescriptor& block_element_descriptor(BlockElement element) {{
    return descriptor_or_default(element);
}}

std::vector<BlockElement> all_block_elements() {{
    std::vector<BlockElement> elements;
    elements.reserve(kDescriptors.size());
    for (const BlockElementDescriptor& descriptor : kDescriptors) {{
        elements.push_back(descriptor.kind);
    }}
    return elements;
}}

std::string block_element_glyph(BlockElement element, GlyphSet glyphs) {{
    const BlockElementDescriptor& descriptor = block_element_descriptor(element);
    glyphs = resolve_glyph_set(glyphs);

    if (glyphs == GlyphSet::Ascii) {{
        return std::string(1, descriptor.ascii_fallback);
    }}

    return utf8_from(descriptor.codepoint);
}}

}} // namespace tuinator
"""
    CPP.write_text(content, encoding="utf-8")


def main() -> int:
    rows = load_tsv()
    write_header(rows)
    write_cpp(rows)
    print(f"Wrote {HEADER.name} and {CPP.name} ({len(rows)} block elements)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
