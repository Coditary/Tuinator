#include <tuinator/render/markdown.hpp>

#include <string>
#include <vector>

#include "test_harness.hpp"

namespace {

std::string flatten(const tuinator::MdBlock& block) {
    std::string out;
    for (const auto& span : block.spans) {
        out += span.text;
    }
    return out;
}

std::string flatten_spans(const std::vector<tuinator::MdSpan>& spans) {
    std::string out;
    for (const auto& span : spans) {
        out += span.text;
    }
    return out;
}

const tuinator::MdSpan* find_span(const std::vector<tuinator::MdSpan>& spans, std::string_view text) {
    for (const auto& span : spans) {
        if (span.text == text) {
            return &span;
        }
    }
    return nullptr;
}

} // namespace

// ---------------------------------------------------------------- blocks

TUINATOR_TEST(markdown_parses_atx_headings) {
    const auto doc = tuinator::parse_markdown("# H1\n## H2\n###### H6\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 3U);
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::Heading);
    TUINATOR_CHECK_EQ(doc.blocks[0].level, 1);
    TUINATOR_CHECK_EQ(flatten(doc.blocks[0]), "H1");
    TUINATOR_CHECK_EQ(doc.blocks[1].level, 2);
    TUINATOR_CHECK_EQ(doc.blocks[2].level, 6);
}

TUINATOR_TEST(markdown_heading_requires_space) {
    const auto doc = tuinator::parse_markdown("#foo\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 1U);
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::Paragraph);
    TUINATOR_CHECK_EQ(flatten(doc.blocks[0]), "#foo");
}

TUINATOR_TEST(markdown_heading_strips_closing_hashes) {
    const auto doc = tuinator::parse_markdown("## Title ##\n");
    TUINATOR_CHECK_EQ(flatten(doc.blocks[0]), "Title");
}

TUINATOR_TEST(markdown_heading_inline_parsed) {
    const auto doc = tuinator::parse_markdown("# **Bold** Title\n");
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::Heading);
    const auto* bold = find_span(doc.blocks[0].spans, "Bold");
    TUINATOR_CHECK(bold != nullptr);
    TUINATOR_CHECK(bold->flags.bold);
}

TUINATOR_TEST(markdown_parses_fenced_code_block) {
    const auto doc = tuinator::parse_markdown("```cpp\nint x = 1;\nreturn x;\n```\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 1U);
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::CodeBlock);
    TUINATOR_CHECK_EQ(doc.blocks[0].info, "cpp");
    TUINATOR_CHECK_EQ(doc.blocks[0].code_lines.size(), 2U);
    TUINATOR_CHECK_EQ(doc.blocks[0].code_lines[0], "int x = 1;");
}

TUINATOR_TEST(markdown_unclosed_fence_consumes_rest) {
    const auto doc = tuinator::parse_markdown("```\nline one\nline two");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 1U);
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::CodeBlock);
    TUINATOR_CHECK_EQ(doc.blocks[0].code_lines.size(), 2U);
    TUINATOR_CHECK_EQ(doc.blocks[0].code_lines[1], "line two");
}

TUINATOR_TEST(markdown_tilde_fence) {
    const auto doc = tuinator::parse_markdown("~~~sh\necho hi\n~~~\n");
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::CodeBlock);
    TUINATOR_CHECK_EQ(doc.blocks[0].info, "sh");
    TUINATOR_CHECK_EQ(doc.blocks[0].code_lines[0], "echo hi");
}

TUINATOR_TEST(markdown_fence_content_not_inline_parsed) {
    const auto doc = tuinator::parse_markdown("```\n**not bold**\n```\n");
    TUINATOR_CHECK_EQ(doc.blocks[0].code_lines[0], "**not bold**");
}

TUINATOR_TEST(markdown_thematic_breaks) {
    const auto doc = tuinator::parse_markdown("---\n***\n___\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 3U);
    for (const auto& block : doc.blocks) {
        TUINATOR_CHECK(block.kind == tuinator::MdBlockKind::ThematicBreak);
    }
}

TUINATOR_TEST(markdown_unordered_list_items) {
    const auto doc = tuinator::parse_markdown("- alpha\n- beta\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 2U);
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::ListItem);
    TUINATOR_CHECK(!doc.blocks[0].ordered);
    TUINATOR_CHECK_EQ(flatten(doc.blocks[0]), "alpha");
    TUINATOR_CHECK_EQ(flatten(doc.blocks[1]), "beta");
}

TUINATOR_TEST(markdown_ordered_list_items) {
    const auto doc = tuinator::parse_markdown("1. first\n2. second\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 2U);
    TUINATOR_CHECK(doc.blocks[0].ordered);
    TUINATOR_CHECK_EQ(doc.blocks[0].number, 1);
    TUINATOR_CHECK_EQ(doc.blocks[1].number, 2);
}

TUINATOR_TEST(markdown_nested_list_depth) {
    const auto doc = tuinator::parse_markdown("- a\n  - b\n    - c\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 3U);
    TUINATOR_CHECK_EQ(doc.blocks[0].level, 0);
    TUINATOR_CHECK_EQ(doc.blocks[1].level, 1);
    TUINATOR_CHECK_EQ(doc.blocks[2].level, 2);
}

TUINATOR_TEST(markdown_list_item_inline_parsed) {
    const auto doc = tuinator::parse_markdown("- has `code` inside\n");
    const auto* code = find_span(doc.blocks[0].spans, "code");
    TUINATOR_CHECK(code != nullptr);
    TUINATOR_CHECK(code->flags.code);
}

TUINATOR_TEST(markdown_block_quote) {
    const auto doc = tuinator::parse_markdown("> hello\n> world\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 1U);
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::Quote);
    TUINATOR_CHECK_EQ(doc.blocks[0].level, 1);
    TUINATOR_CHECK_EQ(flatten(doc.blocks[0]), "hello world");
}

TUINATOR_TEST(markdown_nested_quote_level) {
    const auto doc = tuinator::parse_markdown(">> deep\n");
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::Quote);
    TUINATOR_CHECK_EQ(doc.blocks[0].level, 2);
    TUINATOR_CHECK_EQ(flatten(doc.blocks[0]), "deep");
}

TUINATOR_TEST(markdown_paragraph_joins_lines) {
    const auto doc = tuinator::parse_markdown("foo\nbar\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 1U);
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::Paragraph);
    TUINATOR_CHECK_EQ(flatten(doc.blocks[0]), "foo bar");
}

TUINATOR_TEST(markdown_blank_line_splits_paragraphs) {
    const auto doc = tuinator::parse_markdown("foo\n\nbar\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 2U);
    TUINATOR_CHECK_EQ(flatten(doc.blocks[0]), "foo");
    TUINATOR_CHECK_EQ(flatten(doc.blocks[1]), "bar");
}

TUINATOR_TEST(markdown_heading_interrupts_paragraph) {
    const auto doc = tuinator::parse_markdown("text\n# title\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 2U);
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::Paragraph);
    TUINATOR_CHECK(doc.blocks[1].kind == tuinator::MdBlockKind::Heading);
}

TUINATOR_TEST(markdown_handles_crlf) {
    const auto doc = tuinator::parse_markdown("foo\r\n\r\nbar\r\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 2U);
    TUINATOR_CHECK_EQ(flatten(doc.blocks[0]), "foo");
    TUINATOR_CHECK_EQ(flatten(doc.blocks[1]), "bar");
}

TUINATOR_TEST(markdown_empty_source) {
    const auto doc = tuinator::parse_markdown("");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 0U);
    TUINATOR_CHECK_EQ(doc.stable_prefix, 0U);
}

// ---------------------------------------------------------------- stable prefix (streaming)

TUINATOR_TEST(markdown_stable_prefix_after_blank_line) {
    const std::string source = "foo\n\nbar";
    const auto doc = tuinator::parse_markdown(source);
    TUINATOR_CHECK_EQ(doc.stable_prefix, 5U);
}

TUINATOR_TEST(markdown_stable_prefix_zero_for_unfinished_paragraph) {
    const auto doc = tuinator::parse_markdown("foo");
    TUINATOR_CHECK_EQ(doc.stable_prefix, 0U);
}

TUINATOR_TEST(markdown_stable_prefix_after_closed_fence) {
    const std::string source = "```\nx\n```\n";
    const auto doc = tuinator::parse_markdown(source);
    TUINATOR_CHECK_EQ(doc.stable_prefix, source.size());
}

TUINATOR_TEST(markdown_stable_prefix_not_inside_open_fence) {
    const std::string source = "intro\n\n```\ncode\n\nmore code\n";
    const auto doc = tuinator::parse_markdown(source);
    TUINATOR_CHECK_EQ(doc.stable_prefix, 7U);
}

// ---------------------------------------------------------------- inline

TUINATOR_TEST(inline_plain_text) {
    const auto spans = tuinator::parse_markdown_inline("hello world");
    TUINATOR_CHECK_EQ(spans.size(), 1U);
    TUINATOR_CHECK_EQ(spans[0].text, "hello world");
    TUINATOR_CHECK(spans[0].flags == tuinator::MdInlineFlags{});
}

TUINATOR_TEST(inline_bold) {
    const auto spans = tuinator::parse_markdown_inline("a **bold** b");
    const auto* span = find_span(spans, "bold");
    TUINATOR_CHECK(span != nullptr);
    TUINATOR_CHECK(span->flags.bold);
    TUINATOR_CHECK_EQ(flatten_spans(spans), "a bold b");
}

TUINATOR_TEST(inline_bold_underscore) {
    const auto spans = tuinator::parse_markdown_inline("__bold__");
    TUINATOR_CHECK_EQ(spans.size(), 1U);
    TUINATOR_CHECK(spans[0].flags.bold);
    TUINATOR_CHECK_EQ(spans[0].text, "bold");
}

TUINATOR_TEST(inline_italic) {
    const auto spans = tuinator::parse_markdown_inline("*it*");
    TUINATOR_CHECK_EQ(spans.size(), 1U);
    TUINATOR_CHECK(spans[0].flags.italic);
}

TUINATOR_TEST(inline_italic_underscore) {
    const auto spans = tuinator::parse_markdown_inline("_it_");
    TUINATOR_CHECK_EQ(spans.size(), 1U);
    TUINATOR_CHECK(spans[0].flags.italic);
}

TUINATOR_TEST(inline_snake_case_stays_literal) {
    const auto spans = tuinator::parse_markdown_inline("snake_case_name");
    TUINATOR_CHECK_EQ(spans.size(), 1U);
    TUINATOR_CHECK_EQ(spans[0].text, "snake_case_name");
    TUINATOR_CHECK(!spans[0].flags.italic);
}

TUINATOR_TEST(inline_intraword_star_italic) {
    const auto spans = tuinator::parse_markdown_inline("a*b*c");
    const auto* span = find_span(spans, "b");
    TUINATOR_CHECK(span != nullptr);
    TUINATOR_CHECK(span->flags.italic);
}

TUINATOR_TEST(inline_unclosed_bold_is_literal) {
    const auto spans = tuinator::parse_markdown_inline("**bold");
    TUINATOR_CHECK_EQ(flatten_spans(spans), "**bold");
    TUINATOR_CHECK(spans[0].flags == tuinator::MdInlineFlags{});
}

TUINATOR_TEST(inline_unclosed_italic_is_literal) {
    const auto spans = tuinator::parse_markdown_inline("word *it");
    TUINATOR_CHECK_EQ(flatten_spans(spans), "word *it");
}

TUINATOR_TEST(inline_code_span) {
    const auto spans = tuinator::parse_markdown_inline("run `make test` now");
    const auto* span = find_span(spans, "make test");
    TUINATOR_CHECK(span != nullptr);
    TUINATOR_CHECK(span->flags.code);
}

TUINATOR_TEST(inline_code_span_double_backtick) {
    const auto spans = tuinator::parse_markdown_inline("``a `b` c``");
    TUINATOR_CHECK_EQ(spans.size(), 1U);
    TUINATOR_CHECK(spans[0].flags.code);
    TUINATOR_CHECK_EQ(spans[0].text, "a `b` c");
}

TUINATOR_TEST(inline_code_trims_one_space) {
    const auto spans = tuinator::parse_markdown_inline("` x `");
    TUINATOR_CHECK_EQ(spans.size(), 1U);
    TUINATOR_CHECK_EQ(spans[0].text, "x");
}

TUINATOR_TEST(inline_unclosed_code_is_literal) {
    const auto spans = tuinator::parse_markdown_inline("`code");
    TUINATOR_CHECK_EQ(flatten_spans(spans), "`code");
}

TUINATOR_TEST(inline_markers_inside_code_stay_literal) {
    const auto spans = tuinator::parse_markdown_inline("`**not bold**`");
    TUINATOR_CHECK_EQ(spans.size(), 1U);
    TUINATOR_CHECK(spans[0].flags.code);
    TUINATOR_CHECK(!spans[0].flags.bold);
    TUINATOR_CHECK_EQ(spans[0].text, "**not bold**");
}

TUINATOR_TEST(inline_code_inside_bold) {
    const auto spans = tuinator::parse_markdown_inline("**`x`**");
    TUINATOR_CHECK_EQ(spans.size(), 1U);
    TUINATOR_CHECK(spans[0].flags.bold);
    TUINATOR_CHECK(spans[0].flags.code);
    TUINATOR_CHECK_EQ(spans[0].text, "x");
}

TUINATOR_TEST(inline_bold_inside_italic) {
    const auto spans = tuinator::parse_markdown_inline("*a **b** c*");
    const auto* span = find_span(spans, "b");
    TUINATOR_CHECK(span != nullptr);
    TUINATOR_CHECK(span->flags.italic);
    TUINATOR_CHECK(span->flags.bold);
}

TUINATOR_TEST(inline_strikethrough) {
    const auto spans = tuinator::parse_markdown_inline("~~gone~~");
    TUINATOR_CHECK_EQ(spans.size(), 1U);
    TUINATOR_CHECK(spans[0].flags.strike);
    TUINATOR_CHECK_EQ(spans[0].text, "gone");
}

TUINATOR_TEST(inline_unclosed_strike_is_literal) {
    const auto spans = tuinator::parse_markdown_inline("~~gone");
    TUINATOR_CHECK_EQ(flatten_spans(spans), "~~gone");
}

TUINATOR_TEST(inline_link) {
    const auto spans = tuinator::parse_markdown_inline("see [docs](https://example.com) now");
    const auto* span = find_span(spans, "docs");
    TUINATOR_CHECK(span != nullptr);
    TUINATOR_CHECK(span->flags.link);
    TUINATOR_CHECK_EQ(span->link_target, "https://example.com");
}

TUINATOR_TEST(inline_unclosed_link_is_literal) {
    const auto spans = tuinator::parse_markdown_inline("[docs](https://example.com");
    TUINATOR_CHECK_EQ(flatten_spans(spans), "[docs](https://example.com");
}

TUINATOR_TEST(inline_image) {
    const auto spans = tuinator::parse_markdown_inline("![logo](img.png)");
    TUINATOR_CHECK_EQ(spans.size(), 1U);
    TUINATOR_CHECK(spans[0].flags.image);
    TUINATOR_CHECK_EQ(spans[0].text, "logo");
    TUINATOR_CHECK_EQ(spans[0].link_target, "img.png");
}

TUINATOR_TEST(inline_bare_url_autolink) {
    const auto spans = tuinator::parse_markdown_inline("see https://example.com/x now");
    const auto* span = find_span(spans, "https://example.com/x");
    TUINATOR_CHECK(span != nullptr);
    TUINATOR_CHECK(span->flags.link);
}

TUINATOR_TEST(inline_bare_url_strips_trailing_punctuation) {
    const auto spans = tuinator::parse_markdown_inline("visit https://a.de/x.");
    const auto* span = find_span(spans, "https://a.de/x");
    TUINATOR_CHECK(span != nullptr);
    TUINATOR_CHECK(span->flags.link);
}

TUINATOR_TEST(inline_backslash_escape) {
    const auto spans = tuinator::parse_markdown_inline("\\*not italic\\*");
    TUINATOR_CHECK_EQ(flatten_spans(spans), "*not italic*");
    TUINATOR_CHECK(spans[0].flags == tuinator::MdInlineFlags{});
}

TUINATOR_TEST(inline_empty_input) {
    const auto spans = tuinator::parse_markdown_inline("");
    TUINATOR_CHECK_EQ(spans.size(), 0U);
}
