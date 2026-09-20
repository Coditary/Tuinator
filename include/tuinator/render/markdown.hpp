#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

/// Composable inline attributes for a text run inside a markdown block.
struct MdInlineFlags {
    bool bold = false;
    bool italic = false;
    bool code = false;
    bool strike = false;
    bool link = false;
    bool image = false;

    bool operator==(const MdInlineFlags& other) const {
        return bold == other.bold && italic == other.italic && code == other.code && strike == other.strike &&
               link == other.link && image == other.image;
    }
    bool operator!=(const MdInlineFlags& other) const { return !(*this == other); }
};

/// One styled text run. `link_target` is set when `flags.link` (or image source).
struct MdSpan {
    std::string text;
    MdInlineFlags flags{};
    std::string link_target;

    bool operator==(const MdSpan& other) const {
        return text == other.text && flags == other.flags && link_target == other.link_target;
    }
    bool operator!=(const MdSpan& other) const { return !(*this == other); }
};

enum class MdBlockKind {
    Paragraph,
    Heading,
    CodeBlock,
    Quote,
    ListItem,
    ThematicBreak,
};

struct MdBlock {
    MdBlockKind kind = MdBlockKind::Paragraph;
    int level = 0;       ///< Heading level (1-6), quote/list nesting depth.
    bool ordered = false; ///< ListItem: ordered (1.) vs bullet (-).
    int number = 0;      ///< ListItem: number for ordered lists.
    std::string info;    ///< CodeBlock: fence language tag.
    std::vector<MdSpan> spans;          ///< Inline content (Paragraph/Heading/Quote/ListItem).
    std::vector<std::string> code_lines; ///< CodeBlock: raw lines, no inline parsing.
};

struct MarkdownDocument {
    std::vector<MdBlock> blocks;
    /// Byte offset into the source up to which blocks are final: appending more
    /// text cannot change blocks ending before this point. Lets streaming
    /// consumers reparse only the unstable tail.
    std::size_t stable_prefix = 0;
};

/// Tolerant markdown parser for terminal rendering.
///
/// Supported subset (CommonMark-flavored, streaming-safe):
/// - ATX headings (`#` .. `######`), optional closing `#`s
/// - Fenced code blocks (``` / ~~~) with info string; an unclosed fence
///   consumes the rest of the input (streaming code)
/// - Unordered (`-` `*` `+`) and ordered (`1.` / `1)`) list items, nested by indent
/// - Block quotes (`>`, nestable via `>>`)
/// - Thematic breaks (`---` / `***` / `___`)
/// - Inline: `**bold**`/`__bold__`, `*italic*`/`_italic_` (with intraword rules,
///   so snake_case stays literal), `` `code` ``, `~~strike~~`, `[links](url)`,
///   `![images](url)`, bare https?:// autolinks, backslash escapes
///
/// Deliberately unsupported (rendered as plain text): setext headings, indented
/// code blocks, reference links, HTML blocks, tables. Unclosed inline markers
/// render literally — partial input never throws and never swallows text.
MarkdownDocument parse_markdown(std::string_view source);

/// Parse inline markdown spans of a single logical line (no block handling).
std::vector<MdSpan> parse_markdown_inline(std::string_view text);

/// Neutral alias: the document model is shared by all markup parsers
/// (markdown, asciidoc, ...).
using MarkupDocument = MarkdownDocument;

/// Parser function signature used by MarkdownView/AsciiDocView.
using MarkupParser = MarkdownDocument (*)(std::string_view);

} // namespace tuinator
