#include <tuinator/render/asciidoc.hpp>

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

TUINATOR_TEST(asciidoc_parses_section_headings) {
    const auto doc = tuinator::parse_asciidoc("= Doc Title\n\n== Section\n\n===== Deep\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 3U);
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::Heading);
    TUINATOR_CHECK_EQ(doc.blocks[0].level, 1);
    TUINATOR_CHECK_EQ(flatten(doc.blocks[0]), "Doc Title");
    TUINATOR_CHECK_EQ(doc.blocks[1].level, 2);
    TUINATOR_CHECK_EQ(doc.blocks[2].level, 5);
}

TUINATOR_TEST(asciidoc_heading_requires_space) {
    const auto doc = tuinator::parse_asciidoc("=not a heading\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 1U);
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::Paragraph);
}

TUINATOR_TEST(asciidoc_heading_inline_parsed) {
    const auto doc = tuinator::parse_asciidoc("== *Bold* Title\n");
    const auto* bold = find_span(doc.blocks[0].spans, "Bold");
    TUINATOR_CHECK(bold != nullptr);
    TUINATOR_CHECK(bold->flags.bold);
}

TUINATOR_TEST(asciidoc_source_block_with_language) {
    const auto doc = tuinator::parse_asciidoc("[source,cpp]\n----\nint x = 1;\n----\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 1U);
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::CodeBlock);
    TUINATOR_CHECK_EQ(doc.blocks[0].info, "cpp");
    TUINATOR_CHECK_EQ(doc.blocks[0].code_lines.size(), 1U);
    TUINATOR_CHECK_EQ(doc.blocks[0].code_lines[0], "int x = 1;");
}

TUINATOR_TEST(asciidoc_listing_block) {
    const auto doc = tuinator::parse_asciidoc("----\nplain code\n----\n");
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::CodeBlock);
    TUINATOR_CHECK_EQ(doc.blocks[0].code_lines[0], "plain code");
}

TUINATOR_TEST(asciidoc_unclosed_block_consumes_rest) {
    const auto doc = tuinator::parse_asciidoc("----\nline one\nline two");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 1U);
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::CodeBlock);
    TUINATOR_CHECK_EQ(doc.blocks[0].code_lines.size(), 2U);
}

TUINATOR_TEST(asciidoc_literal_block) {
    const auto doc = tuinator::parse_asciidoc("....\nliteral *not bold*\n....\n");
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::CodeBlock);
    TUINATOR_CHECK_EQ(doc.blocks[0].code_lines[0], "literal *not bold*");
}

TUINATOR_TEST(asciidoc_quote_block) {
    const auto doc = tuinator::parse_asciidoc("____\nquoted _text_\n____\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 1U);
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::Quote);
    const auto* italic = find_span(doc.blocks[0].spans, "text");
    TUINATOR_CHECK(italic != nullptr);
    TUINATOR_CHECK(italic->flags.italic);
}

TUINATOR_TEST(asciidoc_markdown_style_quote_line) {
    const auto doc = tuinator::parse_asciidoc("> quoted words\n");
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::Quote);
    TUINATOR_CHECK_EQ(flatten(doc.blocks[0]), "quoted words");
}

TUINATOR_TEST(asciidoc_unordered_dash_list) {
    const auto doc = tuinator::parse_asciidoc("- alpha\n- beta\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 2U);
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::ListItem);
    TUINATOR_CHECK(!doc.blocks[0].ordered);
    TUINATOR_CHECK_EQ(flatten(doc.blocks[0]), "alpha");
}

TUINATOR_TEST(asciidoc_star_list_nesting) {
    const auto doc = tuinator::parse_asciidoc("* a\n** b\n*** c\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 3U);
    TUINATOR_CHECK_EQ(doc.blocks[0].level, 0);
    TUINATOR_CHECK_EQ(doc.blocks[1].level, 1);
    TUINATOR_CHECK_EQ(doc.blocks[2].level, 2);
}

TUINATOR_TEST(asciidoc_ordered_dot_list) {
    const auto doc = tuinator::parse_asciidoc(". first\n. second\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 2U);
    TUINATOR_CHECK(doc.blocks[0].ordered);
    TUINATOR_CHECK_EQ(doc.blocks[0].number, 1);
    TUINATOR_CHECK_EQ(doc.blocks[1].number, 2);
}

TUINATOR_TEST(asciidoc_ordered_nested_dots) {
    const auto doc = tuinator::parse_asciidoc(". a\n.. b\n");
    TUINATOR_CHECK_EQ(doc.blocks[1].level, 1);
    TUINATOR_CHECK(doc.blocks[1].ordered);
}

TUINATOR_TEST(asciidoc_thematic_break) {
    const auto doc = tuinator::parse_asciidoc("above\n\n'''\n\nbelow\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 3U);
    TUINATOR_CHECK(doc.blocks[1].kind == tuinator::MdBlockKind::ThematicBreak);
}

TUINATOR_TEST(asciidoc_admonition_renders_as_labeled_quote) {
    const auto doc = tuinator::parse_asciidoc("NOTE: remember this\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 1U);
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::Quote);
    const auto* label = find_span(doc.blocks[0].spans, "NOTE:");
    TUINATOR_CHECK(label != nullptr);
    TUINATOR_CHECK(label->flags.bold);
    TUINATOR_CHECK_EQ(flatten(doc.blocks[0]), "NOTE: remember this");
}

TUINATOR_TEST(asciidoc_comment_lines_are_skipped) {
    const auto doc = tuinator::parse_asciidoc("// a comment\ntext\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 1U);
    TUINATOR_CHECK_EQ(flatten(doc.blocks[0]), "text");
}

TUINATOR_TEST(asciidoc_attribute_entries_are_skipped) {
    const auto doc = tuinator::parse_asciidoc(":author: Jane\n:toc:\n\nHello\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 1U);
    TUINATOR_CHECK_EQ(flatten(doc.blocks[0]), "Hello");
}

TUINATOR_TEST(asciidoc_block_title_renders_bold_paragraph) {
    const auto doc = tuinator::parse_asciidoc(".My Title\nSome text\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 2U);
    TUINATOR_CHECK(doc.blocks[0].kind == tuinator::MdBlockKind::Paragraph);
    TUINATOR_CHECK_EQ(doc.blocks[0].spans.size(), 1U);
    TUINATOR_CHECK(doc.blocks[0].spans[0].flags.bold);
    TUINATOR_CHECK_EQ(doc.blocks[0].spans[0].text, "My Title");
}

TUINATOR_TEST(asciidoc_paragraph_joins_lines) {
    const auto doc = tuinator::parse_asciidoc("foo\nbar\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 1U);
    TUINATOR_CHECK_EQ(flatten(doc.blocks[0]), "foo bar");
}

TUINATOR_TEST(asciidoc_blank_line_splits_paragraphs) {
    const auto doc = tuinator::parse_asciidoc("foo\n\nbar\n");
    TUINATOR_CHECK_EQ(doc.blocks.size(), 2U);
}

TUINATOR_TEST(asciidoc_stable_prefix_after_blank_line) {
    const std::string source = "foo\n\nbar";
    const auto doc = tuinator::parse_asciidoc(source);
    TUINATOR_CHECK_EQ(doc.stable_prefix, 5U);
}

TUINATOR_TEST(asciidoc_stable_prefix_after_closed_block) {
    const std::string source = "----\nx\n----\n";
    const auto doc = tuinator::parse_asciidoc(source);
    TUINATOR_CHECK_EQ(doc.stable_prefix, source.size());
}

TUINATOR_TEST(asciidoc_stable_prefix_not_inside_open_block) {
    const std::string source = "intro\n\n----\ncode\n\nmore\n";
    const auto doc = tuinator::parse_asciidoc(source);
    TUINATOR_CHECK_EQ(doc.stable_prefix, 7U);
}

// ---------------------------------------------------------------- inline

TUINATOR_TEST(asciidoc_inline_plain_text) {
    const auto spans = tuinator::parse_asciidoc_inline("hello world");
    TUINATOR_CHECK_EQ(spans.size(), 1U);
    TUINATOR_CHECK(spans[0].flags == tuinator::MdInlineFlags{});
}

TUINATOR_TEST(asciidoc_inline_constrained_bold) {
    const auto spans = tuinator::parse_asciidoc_inline("a *bold* b");
    const auto* span = find_span(spans, "bold");
    TUINATOR_CHECK(span != nullptr);
    TUINATOR_CHECK(span->flags.bold);
}

TUINATOR_TEST(asciidoc_inline_constrained_star_stays_literal_intraword) {
    const auto spans = tuinator::parse_asciidoc_inline("a*b*c");
    TUINATOR_CHECK_EQ(flatten_spans(spans), "a*b*c");
    TUINATOR_CHECK(spans[0].flags == tuinator::MdInlineFlags{});
}

TUINATOR_TEST(asciidoc_inline_unconstrained_bold) {
    const auto spans = tuinator::parse_asciidoc_inline("a**b**c");
    const auto* span = find_span(spans, "b");
    TUINATOR_CHECK(span != nullptr);
    TUINATOR_CHECK(span->flags.bold);
}

TUINATOR_TEST(asciidoc_inline_constrained_italic) {
    const auto spans = tuinator::parse_asciidoc_inline("_it_");
    TUINATOR_CHECK_EQ(spans.size(), 1U);
    TUINATOR_CHECK(spans[0].flags.italic);
}

TUINATOR_TEST(asciidoc_inline_snake_case_stays_literal) {
    const auto spans = tuinator::parse_asciidoc_inline("snake_case_name");
    TUINATOR_CHECK_EQ(spans.size(), 1U);
    TUINATOR_CHECK(!spans[0].flags.italic);
}

TUINATOR_TEST(asciidoc_inline_unconstrained_italic) {
    const auto spans = tuinator::parse_asciidoc_inline("a__b__c");
    const auto* span = find_span(spans, "b");
    TUINATOR_CHECK(span != nullptr);
    TUINATOR_CHECK(span->flags.italic);
}

TUINATOR_TEST(asciidoc_inline_code) {
    const auto spans = tuinator::parse_asciidoc_inline("run `make test` now");
    const auto* span = find_span(spans, "make test");
    TUINATOR_CHECK(span != nullptr);
    TUINATOR_CHECK(span->flags.code);
}

TUINATOR_TEST(asciidoc_inline_unconstrained_code) {
    const auto spans = tuinator::parse_asciidoc_inline("a``b``c");
    const auto* span = find_span(spans, "b");
    TUINATOR_CHECK(span != nullptr);
    TUINATOR_CHECK(span->flags.code);
}

TUINATOR_TEST(asciidoc_inline_unclosed_bold_is_literal) {
    const auto spans = tuinator::parse_asciidoc_inline("*bold");
    TUINATOR_CHECK_EQ(flatten_spans(spans), "*bold");
}

TUINATOR_TEST(asciidoc_inline_highlight) {
    const auto spans = tuinator::parse_asciidoc_inline("this #matters# here");
    const auto* span = find_span(spans, "matters");
    TUINATOR_CHECK(span != nullptr);
    TUINATOR_CHECK(span->flags.code); // highlight renders with the code/mark background
}

TUINATOR_TEST(asciidoc_inline_strikethrough_role) {
    const auto spans = tuinator::parse_asciidoc_inline("[line-through]#gone#");
    const auto* span = find_span(spans, "gone");
    TUINATOR_CHECK(span != nullptr);
    TUINATOR_CHECK(span->flags.strike);
}

TUINATOR_TEST(asciidoc_inline_url_with_text) {
    const auto spans = tuinator::parse_asciidoc_inline("see https://example.com[Docs] now");
    const auto* span = find_span(spans, "Docs");
    TUINATOR_CHECK(span != nullptr);
    TUINATOR_CHECK(span->flags.link);
    TUINATOR_CHECK_EQ(span->link_target, "https://example.com");
}

TUINATOR_TEST(asciidoc_inline_bare_url_autolinks) {
    const auto spans = tuinator::parse_asciidoc_inline("see https://example.com/x now");
    const auto* span = find_span(spans, "https://example.com/x");
    TUINATOR_CHECK(span != nullptr);
    TUINATOR_CHECK(span->flags.link);
}

TUINATOR_TEST(asciidoc_inline_link_macro) {
    const auto spans = tuinator::parse_asciidoc_inline("link:report.pdf[Report]");
    const auto* span = find_span(spans, "Report");
    TUINATOR_CHECK(span != nullptr);
    TUINATOR_CHECK(span->flags.link);
    TUINATOR_CHECK_EQ(span->link_target, "report.pdf");
}

TUINATOR_TEST(asciidoc_inline_image_macro) {
    const auto spans = tuinator::parse_asciidoc_inline("image:logo.png[Logo]");
    TUINATOR_CHECK_EQ(spans.size(), 1U);
    TUINATOR_CHECK(spans[0].flags.image);
    TUINATOR_CHECK_EQ(spans[0].text, "Logo");
    TUINATOR_CHECK_EQ(spans[0].link_target, "logo.png");
}

TUINATOR_TEST(asciidoc_inline_escape) {
    const auto spans = tuinator::parse_asciidoc_inline("\\*not bold\\*");
    TUINATOR_CHECK_EQ(flatten_spans(spans), "*not bold*");
    TUINATOR_CHECK(spans[0].flags == tuinator::MdInlineFlags{});
}

TUINATOR_TEST(asciidoc_inline_bold_inside_italic) {
    const auto spans = tuinator::parse_asciidoc_inline("_a *b* c_");
    const auto* span = find_span(spans, "b");
    TUINATOR_CHECK(span != nullptr);
    TUINATOR_CHECK(span->flags.italic);
    TUINATOR_CHECK(span->flags.bold);
}
