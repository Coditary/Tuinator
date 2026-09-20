#include <tuinator/render/asciidoc.hpp>

#include <algorithm>
#include <cctype>

namespace tuinator {

namespace {

struct SourceLine {
    std::string_view text;
    std::size_t start = 0;
    std::size_t end = 0;
};

std::vector<SourceLine> split_source_lines(std::string_view source) {
    std::vector<SourceLine> lines;
    std::size_t pos = 0;
    while (pos < source.size()) {
        const std::size_t newline = source.find('\n', pos);
        const std::size_t line_end = newline == std::string_view::npos ? source.size() : newline;
        std::string_view text = source.substr(pos, line_end - pos);
        if (!text.empty() && text.back() == '\r') {
            text.remove_suffix(1);
        }
        lines.push_back({text, pos, newline == std::string_view::npos ? source.size() : newline + 1});
        pos = line_end + 1;
        if (newline == std::string_view::npos) {
            break;
        }
    }
    return lines;
}

bool is_space(char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; }

bool is_blank(std::string_view line) { return std::all_of(line.begin(), line.end(), is_space); }

std::string_view trim_view(std::string_view text) {
    while (!text.empty() && is_space(text.front())) {
        text.remove_prefix(1);
    }
    while (!text.empty() && is_space(text.back())) {
        text.remove_suffix(1);
    }
    return text;
}

// ---------------------------------------------------------------- block starters

int heading_level(std::string_view line) {
    int level = 0;
    while (level < static_cast<int>(line.size()) && line[static_cast<std::size_t>(level)] == '=') {
        ++level;
    }
    if (level < 1 || level > 6) {
        return 0;
    }
    if (level < static_cast<int>(line.size()) && line[static_cast<std::size_t>(level)] != ' ') {
        return 0;
    }
    return level;
}

/// A delimited block boundary: 4+ identical chars and nothing else.
bool delimited_block(std::string_view line, char& marker) {
    if (line.size() < 4) {
        return false;
    }
    const char c = line[0];
    if (c != '-' && c != '.' && c != '_' && c != '*') {
        return false;
    }
    for (const char ch : line) {
        if (ch != c) {
            return false;
        }
    }
    marker = c;
    return true;
}

bool is_thematic_break(std::string_view line) {
    if (line.size() < 3) {
        return false;
    }
    for (const char c : line) {
        if (c != '\'') {
            return false;
        }
    }
    return true;
}

bool is_comment_line(std::string_view line) { return line.size() >= 2 && line[0] == '/' && line[1] == '/'; }

bool is_attribute_entry(std::string_view line) {
    if (line.size() < 3 || line[0] != ':') {
        return false;
    }
    const std::size_t close = line.find(':', 1);
    if (close == std::string_view::npos || close == 1) {
        return false;
    }
    for (std::size_t i = 1; i < close; ++i) {
        if (is_space(line[i])) {
            return false;
        }
    }
    return true;
}

/// Block attribute line like [source,cpp]. Returns true and extracts the
/// source language (empty when not a source attribute).
bool block_attribute(std::string_view line, std::string& language) {
    if (line.size() < 2 || line.front() != '[' || line.back() != ']') {
        return false;
    }
    const std::string_view inner = line.substr(1, line.size() - 2);
    if (inner.rfind("source", 0) == 0) {
        const std::size_t comma = inner.find(',');
        if (comma != std::string_view::npos) {
            language = std::string(trim_view(inner.substr(comma + 1)));
        }
    }
    return true;
}

bool is_block_title(std::string_view line) {
    return line.size() >= 2 && line[0] == '.' && line[1] != ' ' && line[1] != '.';
}

struct Admonition {
    std::string_view label;
    std::string_view rest;
};

bool admonition(std::string_view line, Admonition& out) {
    static constexpr std::string_view kLabels[] = {"NOTE", "TIP", "IMPORTANT", "WARNING", "CAUTION"};
    for (const std::string_view label : kLabels) {
        if (line.size() > label.size() + 1 && line.substr(0, label.size()) == label &&
            line[label.size()] == ':' && line[label.size() + 1] == ' ') {
            out.label = label;
            out.rest = trim_view(line.substr(label.size() + 1));
            return true;
        }
    }
    return false;
}

int quote_depth(std::string_view line) {
    int depth = 0;
    while (!line.empty() && line.front() == '>') {
        ++depth;
        line.remove_prefix(1);
        if (!line.empty() && line.front() == ' ') {
            line.remove_prefix(1);
        }
    }
    return depth;
}

std::string_view quote_content(std::string_view line) {
    while (!line.empty() && line.front() == '>') {
        line.remove_prefix(1);
        if (!line.empty() && line.front() == ' ') {
            line.remove_prefix(1);
        }
    }
    return line;
}

struct ListMarker {
    bool ordered = false;
    int level = 0;
    int arabic_number = 0; ///< > 0 when the item used an explicit "N." marker
    std::size_t content_offset = 0;
};

bool list_marker(std::string_view line, ListMarker& marker) {
    if (line.size() < 2) {
        return false;
    }

    if (line[0] == '-' && line[1] == ' ') {
        marker.content_offset = 2;
        return true;
    }

    if (line[0] == '*') {
        std::size_t run = 0;
        while (run < line.size() && line[run] == '*') {
            ++run;
        }
        if (run < line.size() && line[run] == ' ') {
            marker.level = static_cast<int>(run) - 1;
            marker.content_offset = run + 1;
            return true;
        }
        return false;
    }

    if (line[0] == '.') {
        std::size_t run = 0;
        while (run < line.size() && line[run] == '.') {
            ++run;
        }
        if (run < line.size() && line[run] == ' ') {
            marker.ordered = true;
            marker.level = static_cast<int>(run) - 1;
            marker.content_offset = run + 1;
            return true;
        }
        return false;
    }

    if (std::isdigit(static_cast<unsigned char>(line[0])) != 0) {
        std::size_t digits = 0;
        int number = 0;
        while (digits < line.size() && std::isdigit(static_cast<unsigned char>(line[digits])) != 0) {
            number = number * 10 + (line[digits] - '0');
            ++digits;
        }
        if (digits >= 1 && digits + 1 < line.size() && line[digits] == '.' && line[digits + 1] == ' ') {
            marker.ordered = true;
            marker.arabic_number = number;
            marker.content_offset = digits + 2;
            return true;
        }
    }
    return false;
}

bool starts_block(std::string_view line) {
    if (heading_level(line) > 0 || is_thematic_break(line) || is_comment_line(line) || is_attribute_entry(line) ||
        is_block_title(line)) {
        return true;
    }
    char marker = '\0';
    if (delimited_block(line, marker)) {
        return true;
    }
    if (quote_depth(line) > 0) {
        return true;
    }
    std::string language;
    if (block_attribute(line, language)) {
        return true;
    }
    Admonition adm;
    if (admonition(line, adm)) {
        return true;
    }
    ListMarker list;
    return list_marker(line, list);
}

// ---------------------------------------------------------------- inline parsing

bool is_word_char(char c) { return std::isalnum(static_cast<unsigned char>(c)) != 0; }

bool is_ascii_punct(char c) { return std::ispunct(static_cast<unsigned char>(c)) != 0; }

char char_at(std::string_view text, std::size_t pos) { return pos < text.size() ? text[pos] : '\0'; }

std::size_t char_run(std::string_view text, std::size_t pos, char c) {
    std::size_t run = 0;
    while (pos + run < text.size() && text[pos + run] == c) {
        ++run;
    }
    return run;
}

std::vector<MdSpan> parse_inline(std::string_view text, MdInlineFlags active);

void push_span(std::vector<MdSpan>& spans, std::string text, MdInlineFlags flags, std::string target = {}) {
    if (text.empty() && target.empty()) {
        return;
    }
    spans.push_back({std::move(text), flags, std::move(target)});
}

/// Constrained formatting: opener must not follow a word char and not precede
/// whitespace; closer must not follow whitespace and not precede a word char.
bool can_open_constrained(std::string_view text, std::size_t pos) {
    if (pos > 0 && is_word_char(text[pos - 1])) {
        return false;
    }
    const char next = char_at(text, pos + 1);
    return next != '\0' && !is_space(next);
}

bool can_close_constrained(std::string_view text, std::size_t pos) {
    const char prev = pos > 0 ? text[pos - 1] : '\0';
    if (prev == '\0' || is_space(prev)) {
        return false;
    }
    return !is_word_char(char_at(text, pos + 1));
}

/// Find closer for a constrained single-char delimiter, skipping nested openers.
std::size_t find_constrained_closer(std::string_view text, std::size_t from, char delim) {
    std::size_t pos = from;
    int depth = 0;
    while (pos < text.size()) {
        const char c = text[pos];
        if (c == '\\' && pos + 1 < text.size()) {
            pos += 2;
            continue;
        }
        if (c == delim) {
            const bool closes = can_close_constrained(text, pos);
            const bool opens = can_open_constrained(text, pos);
            if (closes && depth == 0) {
                return pos;
            }
            if (closes) {
                --depth;
            } else if (opens) {
                ++depth;
            }
        }
        ++pos;
    }
    return std::string_view::npos;
}

MdInlineFlags flag_for(char delim, MdInlineFlags base) {
    switch (delim) {
    case '*': base.bold = true; break;
    case '_': base.italic = true; break;
    case '`':
    case '#': base.code = true; break; // #highlight# renders with the code/mark background
    default: break;
    }
    return base;
}

/// Constrained single-char formatting (*bold*, _italic_, `code`, #mark#).
std::size_t try_constrained(std::string_view text, std::size_t pos, MdInlineFlags active,
                            std::vector<MdSpan>& spans) {
    const char delim = text[pos];
    if (!can_open_constrained(text, pos)) {
        return 0;
    }
    const std::size_t closer = find_constrained_closer(text, pos + 1, delim);
    if (closer == std::string_view::npos || closer == pos + 1) {
        return 0;
    }
    const MdInlineFlags flags = flag_for(delim, active);
    if (delim == '`') {
        push_span(spans, std::string(text.substr(pos + 1, closer - pos - 1)), flags);
    } else {
        auto inner = parse_inline(text.substr(pos + 1, closer - pos - 1), flags);
        for (auto& span : inner) {
            spans.push_back(std::move(span));
        }
    }
    return closer + 1 - pos;
}

/// Unconstrained double-char formatting (**b**, __i__, ``c``) — no flanking rules.
std::size_t try_unconstrained(std::string_view text, std::size_t pos, MdInlineFlags active,
                              std::vector<MdSpan>& spans) {
    const char delim = text[pos];
    if (char_at(text, pos + 1) != delim) {
        return 0;
    }
    const char close[3] = {delim, delim, '\0'};
    const std::size_t closer = text.find(close, pos + 2);
    if (closer == std::string_view::npos || closer == pos + 2) {
        return 0;
    }
    const MdInlineFlags flags = flag_for(delim, active);
    if (delim == '`') {
        push_span(spans, std::string(text.substr(pos + 2, closer - pos - 2)), flags);
    } else {
        auto inner = parse_inline(text.substr(pos + 2, closer - pos - 2), flags);
        for (auto& span : inner) {
            spans.push_back(std::move(span));
        }
    }
    return closer + 2 - pos;
}

/// [line-through]#text#
std::size_t try_strike_role(std::string_view text, std::size_t pos, MdInlineFlags active,
                            std::vector<MdSpan>& spans) {
    static constexpr std::string_view kPrefix = "[line-through]#";
    if (text.substr(pos, kPrefix.size()) != kPrefix) {
        return 0;
    }
    const std::size_t closer = text.find('#', pos + kPrefix.size());
    if (closer == std::string_view::npos || closer == pos + kPrefix.size()) {
        return 0;
    }
    MdInlineFlags flags = active;
    flags.strike = true;
    auto inner = parse_inline(text.substr(pos + kPrefix.size(), closer - pos - kPrefix.size()), flags);
    for (auto& span : inner) {
        spans.push_back(std::move(span));
    }
    return closer + 1 - pos;
}

/// name[target] macro body: returns consumed length past `name`, fills target/text.
std::size_t macro_body(std::string_view text, std::size_t target_start, std::string_view& target,
                       std::string_view& label) {
    const std::size_t open = text.find('[', target_start);
    if (open == std::string_view::npos) {
        return 0;
    }
    const std::size_t close = text.find(']', open + 1);
    if (close == std::string_view::npos) {
        return 0;
    }
    target = text.substr(target_start, open - target_start);
    label = text.substr(open + 1, close - open - 1);
    return close + 1 - target_start;
}

/// image:path[alt] or image::path[alt]
std::size_t try_image_macro(std::string_view text, std::size_t pos, MdInlineFlags active,
                            std::vector<MdSpan>& spans) {
    std::size_t target_start = pos + 6; // "image:"
    if (char_at(text, target_start) == ':') {
        ++target_start;
    }
    std::string_view target;
    std::string_view label;
    const std::size_t consumed = macro_body(text, target_start, target, label);
    if (consumed == 0 || target.empty()) {
        return 0;
    }
    MdInlineFlags flags = active;
    flags.link = true;
    flags.image = true;
    push_span(spans, std::string(label), flags, std::string(target));
    return target_start - pos + consumed;
}

/// link:url[text]
std::size_t try_link_macro(std::string_view text, std::size_t pos, MdInlineFlags active,
                           std::vector<MdSpan>& spans) {
    const std::size_t target_start = pos + 5; // "link:"
    std::string_view target;
    std::string_view label;
    const std::size_t consumed = macro_body(text, target_start, target, label);
    if (consumed == 0 || target.empty()) {
        return 0;
    }
    MdInlineFlags flags = active;
    flags.link = true;
    auto inner = parse_inline(label, flags);
    for (auto& span : inner) {
        span.link_target = std::string(target);
        spans.push_back(std::move(span));
    }
    return 5 + consumed;
}

/// Bare https?:// URL, optionally followed by [display text].
std::size_t try_autolink(std::string_view text, std::size_t pos, MdInlineFlags active,
                         std::vector<MdSpan>& spans) {
    std::size_t scheme_length = 0;
    if (text.substr(pos, 8) == "https://") {
        scheme_length = 8;
    } else if (text.substr(pos, 7) == "http://") {
        scheme_length = 7;
    } else {
        return 0;
    }
    if (pos > 0 && !is_space(text[pos - 1]) && text[pos - 1] != '(') {
        return 0;
    }

    std::size_t end = pos + scheme_length;
    while (end < text.size() && !is_space(text[end]) && text[end] != '<' && text[end] != '>' &&
           text[end] != '"' && text[end] != '\'' && text[end] != '[') {
        ++end;
    }
    std::string url(text.substr(pos, end - pos));
    while (!url.empty() && (url.back() == '.' || url.back() == ',' || url.back() == ';' || url.back() == ':' ||
                            url.back() == '!' || url.back() == '?')) {
        url.pop_back();
    }
    while (!url.empty() && url.back() == ')') {
        const std::size_t opens = static_cast<std::size_t>(std::count(url.begin(), url.end(), '('));
        const std::size_t closes = static_cast<std::size_t>(std::count(url.begin(), url.end(), ')'));
        if (closes <= opens) {
            break;
        }
        url.pop_back();
    }
    if (url.size() <= scheme_length) {
        return 0;
    }

    MdInlineFlags flags = active;
    flags.link = true;

    std::size_t consumed = url.size();
    std::string_view label;
    std::string_view target;
    if (char_at(text, pos + consumed) == '[') {
        const std::size_t macro = macro_body(text, pos + consumed, target, label);
        if (macro > 0) {
            consumed += macro;
        }
    }

    if (!label.empty()) {
        auto inner = parse_inline(label, flags);
        for (auto& span : inner) {
            span.link_target = url;
            spans.push_back(std::move(span));
        }
    } else {
        push_span(spans, url, flags, url);
    }
    return consumed;
}

std::vector<MdSpan> parse_inline(std::string_view text, MdInlineFlags active) {
    std::vector<MdSpan> spans;
    std::string literal;

    const auto flush_literal = [&] {
        if (!literal.empty()) {
            push_span(spans, std::move(literal), active);
            literal.clear();
        }
    };

    std::size_t pos = 0;
    while (pos < text.size()) {
        const char c = text[pos];

        if (c == '\\' && pos + 1 < text.size() && is_ascii_punct(text[pos + 1])) {
            literal += text[pos + 1];
            pos += 2;
            continue;
        }

        std::size_t consumed = 0;
        std::vector<MdSpan> produced;
        if (c == '[') {
            consumed = try_strike_role(text, pos, active, produced);
        } else if (c == '*' || c == '_' || c == '`') {
            if (char_at(text, pos + 1) == c) {
                consumed = try_unconstrained(text, pos, active, produced);
            } else {
                consumed = try_constrained(text, pos, active, produced);
            }
        } else if (c == '#') {
            consumed = try_constrained(text, pos, active, produced);
        } else if (c == 'i' && text.substr(pos, 6) == "image:") {
            consumed = try_image_macro(text, pos, active, produced);
        } else if (c == 'l' && text.substr(pos, 5) == "link:") {
            consumed = try_link_macro(text, pos, active, produced);
        } else if (c == 'h') {
            consumed = try_autolink(text, pos, active, produced);
        }

        if (consumed > 0) {
            flush_literal();
            for (auto& span : produced) {
                spans.push_back(std::move(span));
            }
            pos += consumed;
            continue;
        }
        literal += c;
        ++pos;
    }
    flush_literal();
    return spans;
}

} // namespace

std::vector<MdSpan> parse_asciidoc_inline(std::string_view text) { return parse_inline(text, MdInlineFlags{}); }

MarkupDocument parse_asciidoc(std::string_view source) {
    MarkupDocument doc;
    const std::vector<SourceLine> lines = split_source_lines(source);
    std::size_t stable = 0;
    std::size_t index = 0;
    std::vector<int> ordered_counters;
    std::string pending_language;

    const auto reset_numbering = [&] { ordered_counters.clear(); };

    while (index < lines.size()) {
        const SourceLine& line = lines[index];

        if (is_blank(line.text)) {
            stable = line.end;
            reset_numbering();
            ++index;
            continue;
        }

        if (is_comment_line(line.text) || is_attribute_entry(line.text)) {
            reset_numbering();
            ++index;
            continue;
        }

        std::string language;
        if (block_attribute(line.text, language)) {
            pending_language = std::move(language);
            ++index;
            continue;
        }

        char delimiter = '\0';
        if (delimited_block(line.text, delimiter)) {
            MdBlock block;
            const bool is_code = delimiter == '-' || delimiter == '.';
            block.kind = is_code ? MdBlockKind::CodeBlock : MdBlockKind::Quote;
            block.level = 1;
            if (is_code) {
                block.info = std::move(pending_language);
            }
            pending_language.clear();
            reset_numbering();
            ++index;
            std::string joined;
            while (index < lines.size()) {
                char closing = '\0';
                if (delimited_block(lines[index].text, closing) && closing == delimiter) {
                    stable = lines[index].end;
                    ++index;
                    break;
                }
                if (is_code) {
                    block.code_lines.push_back(std::string(lines[index].text));
                } else {
                    const std::string_view content = trim_view(lines[index].text);
                    if (!joined.empty() && !content.empty()) {
                        joined += ' ';
                    }
                    joined += content;
                }
                ++index;
            }
            if (!is_code) {
                block.spans = parse_inline(joined, MdInlineFlags{});
            }
            doc.blocks.push_back(std::move(block));
            continue;
        }
        pending_language.clear();

        if (const int level = heading_level(line.text); level > 0) {
            MdBlock block;
            block.kind = MdBlockKind::Heading;
            block.level = level;
            block.spans = parse_inline(trim_view(line.text.substr(static_cast<std::size_t>(level))), MdInlineFlags{});
            doc.blocks.push_back(std::move(block));
            reset_numbering();
            ++index;
            continue;
        }

        if (is_thematic_break(line.text)) {
            MdBlock block;
            block.kind = MdBlockKind::ThematicBreak;
            doc.blocks.push_back(std::move(block));
            reset_numbering();
            ++index;
            continue;
        }

        Admonition adm;
        if (admonition(line.text, adm)) {
            MdBlock block;
            block.kind = MdBlockKind::Quote;
            block.level = 1;
            MdInlineFlags bold;
            bold.bold = true;
            block.spans.push_back({std::string(adm.label) + ":", bold, {}});
            auto rest = parse_inline(adm.rest, MdInlineFlags{});
            if (!rest.empty()) {
                block.spans.push_back({" ", MdInlineFlags{}, {}});
                for (auto& span : rest) {
                    block.spans.push_back(std::move(span));
                }
            }
            doc.blocks.push_back(std::move(block));
            reset_numbering();
            ++index;
            continue;
        }

        if (quote_depth(line.text) > 0) {
            MdBlock block;
            block.kind = MdBlockKind::Quote;
            block.level = quote_depth(line.text);
            std::string joined;
            while (index < lines.size() && quote_depth(lines[index].text) > 0) {
                const std::string_view content = trim_view(quote_content(lines[index].text));
                if (!joined.empty() && !content.empty()) {
                    joined += ' ';
                }
                joined += content;
                ++index;
            }
            block.spans = parse_inline(joined, MdInlineFlags{});
            doc.blocks.push_back(std::move(block));
            reset_numbering();
            continue;
        }

        ListMarker marker;
        if (list_marker(line.text, marker)) {
            MdBlock block;
            block.kind = MdBlockKind::ListItem;
            block.ordered = marker.ordered;
            block.level = marker.level;
            if (marker.ordered) {
                if (marker.arabic_number > 0) {
                    block.number = marker.arabic_number;
                } else {
                    if (static_cast<int>(ordered_counters.size()) <= marker.level) {
                        ordered_counters.resize(static_cast<std::size_t>(marker.level) + 1, 0);
                    }
                    ordered_counters[static_cast<std::size_t>(marker.level)] += 1;
                    ordered_counters.resize(static_cast<std::size_t>(marker.level) + 1);
                    block.number = ordered_counters[static_cast<std::size_t>(marker.level)];
                }
            }
            block.spans = parse_inline(trim_view(line.text.substr(marker.content_offset)), MdInlineFlags{});
            doc.blocks.push_back(std::move(block));
            ++index;
            continue;
        }

        if (is_block_title(line.text)) {
            MdBlock block;
            block.kind = MdBlockKind::Paragraph;
            MdInlineFlags bold;
            bold.bold = true;
            block.spans.push_back({std::string(trim_view(line.text.substr(1))), bold, {}});
            doc.blocks.push_back(std::move(block));
            reset_numbering();
            ++index;
            continue;
        }

        reset_numbering();
        MdBlock paragraph;
        paragraph.kind = MdBlockKind::Paragraph;
        std::string joined(trim_view(line.text));
        ++index;
        while (index < lines.size() && !is_blank(lines[index].text) && !starts_block(lines[index].text)) {
            const std::string_view content = trim_view(lines[index].text);
            if (!joined.empty() && !content.empty()) {
                joined += ' ';
            }
            joined += content;
            ++index;
        }
        paragraph.spans = parse_inline(joined, MdInlineFlags{});
        doc.blocks.push_back(std::move(paragraph));
    }

    doc.stable_prefix = stable;
    return doc;
}

} // namespace tuinator
