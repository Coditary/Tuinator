#pragma once

#include <tuinator/render/markdown.hpp>

#include <string_view>
#include <vector>

namespace tuinator {

/// Tolerant AsciiDoc parser for terminal rendering, producing the shared
/// markup document model (same block/span types as parse_markdown).
///
/// Supported subset (streaming-safe):
/// - Document/section headings (`= ` .. `====== `)
/// - Delimited blocks: `----` listing/source (with optional `[source,lang]`
///   attribute line), `....` literal, `____` quote; an unclosed block consumes
///   the rest of the input (streaming-friendly)
/// - Markdown-style quote lines (`> `)
/// - Unordered lists (`-` or `*`, nested via `**`/`***`), ordered lists
///   (`. `, nested via `..`, or `1.`)
/// - Thematic break (`'''`)
/// - Admonitions (`NOTE:` `TIP:` `IMPORTANT:` `WARNING:` `CAUTION:`) rendered
///   as quote blocks with a bold label
/// - Block titles (`.Title`) rendered as a bold paragraph
/// - Inline: constrained `*bold*`/`_italic_`/`` `code` `` (intraword-safe),
///   unconstrained `**b**`/`__i__`/``` ``c`` ```, `#highlight#`,
///   `[line-through]#strike#`, bare URL autolinks, `url[text]` and
///   `link:url[text]`, `image:path[alt]`, backslash escapes
/// - Comment lines (`//`) and attribute entries (`:name: value`) are skipped
///
/// Deliberately unsupported (rendered as plain text): tables, includes,
/// conditionals, attribute substitution, passthrough blocks, superscript /
/// subscript, footnotes. Unclosed inline markers render literally.
MarkupDocument parse_asciidoc(std::string_view source);

/// Parse inline AsciiDoc spans of a single logical line (no block handling).
std::vector<MdSpan> parse_asciidoc_inline(std::string_view text);

} // namespace tuinator
