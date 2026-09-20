#include <tuinator/render/markdown.hpp>

#include <algorithm>
#include <cctype>

namespace tuinator {

namespace {

struct SourceLine {
    std::string_view text; ///< Line content without trailing newline / CR.
    std::size_t start = 0; ///< Byte offset of the line start in the source.
    std::size_t end = 0;   ///< Byte offset just past the newline (or source size).
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

bool is_blank(std::string_view line) {
    return std::all_of(line.begin(), line.end(), is_space);
}

std::string_view trim_view(std::string_view text) {
    while (!text.empty() && is_space(text.front())) {
        text.remove_prefix(1);
    }
    while (!text.empty() && is_space(text.back())) {
        text.remove_suffix(1);
    }
    return text;
}

int leading_spaces(std::string_view line) {
    int count = 0;
    while (count < static_cast<int>(line.size()) && line[static_cast<std::size_t>(count)] == ' ') {
        ++count;
    }
    return count;
}

// ---------------------------------------------------------------- block starters

bool fence_start(std::string_view line, char& fence_char, int& fence_length, std::string& info) {
    const int indent = leading_spaces(line);
    if (indent > 3) {
        return false;
    }
    line.remove_prefix(static_cast<std::size_t>(indent));
    if (line.size() < 3 || (line[0] != '`' && line[0] != '~')) {
        return false;
    }
    const char marker = line[0];
    int run = 0;
    while (run < static_cast<int>(line.size()) && line[static_cast<std::size_t>(run)] == marker) {
        ++run;
    }
    if (run < 3) {
        return false;
    }
    const std::string_view rest = trim_view(line.substr(static_cast<std::size_t>(run)));
    if (marker == '`' && rest.find('`') != std::string_view::npos) {
        return false; // info string with backtick is not a fence (CommonMark)
    }
    fence_char = marker;
    fence_length = run;
    info = std::string(rest);
    return true;
}

bool fence_end(std::string_view line, char fence_char, int fence_length) {
    const int indent = leading_spaces(line);
    if (indent > 3) {
        return false;
    }
    line.remove_prefix(static_cast<std::size_t>(indent));
    int run = 0;
    while (run < static_cast<int>(line.size()) && line[static_cast<std::size_t>(run)] == fence_char) {
        ++run;
    }
    if (run < fence_length) {
        return false;
    }
    return is_blank(line.substr(static_cast<std::size_t>(run)));
}

int heading_level(std::string_view line) {
    const int indent = leading_spaces(line);
    if (indent > 3) {
        return 0;
    }
    line.remove_prefix(static_cast<std::size_t>(indent));
    int level = 0;
    while (level < static_cast<int>(line.size()) && line[static_cast<std::size_t>(level)] == '#') {
        ++level;
    }
    if (level < 1 || level > 6) {
        return 0;
    }
    if (level < static_cast<int>(line.size()) && line[static_cast<std::size_t>(level)] != ' ' &&
        line[static_cast<std::size_t>(level)] != '\t') {
        return 0;
    }
    return level;
}

std::string_view heading_content(std::string_view line) {
    line.remove_prefix(static_cast<std::size_t>(leading_spaces(line)));
    int level = 0;
    while (level < static_cast<int>(line.size()) && line[static_cast<std::size_t>(level)] == '#') {
        ++level;
    }
    std::string_view content = trim_view(line.substr(static_cast<std::size_t>(level)));
    // Strip optional closing sequence: " title ###" -> " title"
    if (!content.empty() && content.back() == '#') {
        std::size_t close_start = content.size();
        while (close_start > 0 && content[close_start - 1] == '#') {
            --close_start;
        }
        if (close_start > 0 && content[close_start - 1] == ' ') {
            content = trim_view(content.substr(0, close_start - 1));
        }
    }
    return content;
}

bool is_thematic_break(std::string_view line) {
    const int indent = leading_spaces(line);
    if (indent > 3) {
        return false;
    }
    line.remove_prefix(static_cast<std::size_t>(indent));
    char marker = '\0';
    int count = 0;
    for (const char c : line) {
        if (c == ' ' || c == '\t') {
            continue;
        }
        if (c != '-' && c != '*' && c != '_') {
            return false;
        }
        if (marker == '\0') {
            marker = c;
        } else if (c != marker) {
            return false;
        }
        ++count;
    }
    return count >= 3;
}

int quote_depth(std::string_view line) {
    const int indent = leading_spaces(line);
    if (indent > 3) {
        return 0;
    }
    line.remove_prefix(static_cast<std::size_t>(indent));
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
    line.remove_prefix(static_cast<std::size_t>(leading_spaces(line)));
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
    int number = 0;
    int indent = 0;
    std::size_t content_offset = 0;
};

bool list_marker(std::string_view line, ListMarker& marker) {
    const int indent = leading_spaces(line);
    std::string_view rest = line.substr(static_cast<std::size_t>(indent));
    if (rest.empty()) {
        return false;
    }

    const char first = rest[0];
    if (first == '-' || first == '*' || first == '+') {
        if (rest.size() >= 2 && (rest[1] == ' ' || rest[1] == '\t')) {
            marker.ordered = false;
            marker.indent = indent;
            marker.content_offset = static_cast<std::size_t>(indent) + 2;
            return true;
        }
        return false;
    }

    if (std::isdigit(static_cast<unsigned char>(first)) != 0) {
        std::size_t digits = 0;
        int number = 0;
        while (digits < rest.size() && std::isdigit(static_cast<unsigned char>(rest[digits])) != 0) {
            number = number * 10 + (rest[digits] - '0');
            ++digits;
        }
        if (digits >= 1 && digits <= 9 && digits + 1 < rest.size() &&
            (rest[digits] == '.' || rest[digits] == ')') &&
            (rest[digits + 1] == ' ' || rest[digits + 1] == '\t')) {
            marker.ordered = true;
            marker.number = number;
            marker.indent = indent;
            marker.content_offset = static_cast<std::size_t>(indent) + digits + 2;
            return true;
        }
    }
    return false;
}

bool starts_block(std::string_view line) {
    char fence_char = '\0';
    int fence_length = 0;
    std::string info;
    if (fence_start(line, fence_char, fence_length, info)) {
        return true;
    }
    if (heading_level(line) > 0) {
        return true;
    }
    if (is_thematic_break(line)) {
        return true;
    }
    if (quote_depth(line) > 0) {
        return true;
    }
    ListMarker marker;
    return list_marker(line, marker);
}

// ---------------------------------------------------------------- inline parsing

bool is_word_char(char c) { return std::isalnum(static_cast<unsigned char>(c)) != 0; }

bool is_ascii_punct(char c) { return std::ispunct(static_cast<unsigned char>(c)) != 0; }

std::size_t char_run(std::string_view text, std::size_t pos, char c) {
    std::size_t run = 0;
    while (pos + run < text.size() && text[pos + run] == c) {
        ++run;
    }
    return run;
}

char char_at(std::string_view text, std::size_t pos) {
    return pos < text.size() ? text[pos] : '\0';
}

bool can_open_emphasis(std::string_view text, std::size_t pos, std::size_t run, char delim) {
    const char next = char_at(text, pos + run);
    if (next == '\0' || is_space(next)) {
        return false;
    }
    if (delim == '_' && pos > 0 && is_word_char(text[pos - 1])) {
        return false; // intraword underscore cannot open (snake_case stays literal)
    }
    return true;
}

bool can_close_emphasis(std::string_view text, std::size_t pos, std::size_t run, char delim) {
    const char prev = pos > 0 ? text[pos - 1] : '\0';
    if (prev == '\0' || is_space(prev)) {
        return false;
    }
    if (delim == '_' && is_word_char(char_at(text, pos + run))) {
        return false; // intraword underscore cannot close
    }
    return true;
}

std::vector<MdSpan> parse_inline(std::string_view text, MdInlineFlags active);

void push_span(std::vector<MdSpan>& spans, std::string text, MdInlineFlags flags, std::string target = {}) {
    if (text.empty() && target.empty()) {
        return;
    }
    spans.push_back({std::move(text), flags, std::move(target)});
}

// Find a closing delimiter run of at least `run` length that may close emphasis.
// Runs of the same delimiter that can open emphasis are treated as nested
// openers and skipped with depth tracking, so "*a **b** c*" closes at the
// final star instead of the tail of the inner "**" opener.
std::size_t find_emphasis_closer(std::string_view text, std::size_t from, char delim, std::size_t run) {
    std::size_t pos = from;
    int depth = 0;
    while (pos < text.size()) {
        const char c = text[pos];
        if (c == '\\' && pos + 1 < text.size()) {
            pos += 2;
            continue;
        }
        if (c == delim) {
            const std::size_t found = char_run(text, pos, delim);
            const bool closes = can_close_emphasis(text, pos, found, delim);
            const bool opens = can_open_emphasis(text, pos, found, delim);
            if (closes && found >= run && depth == 0) {
                return pos;
            }
            if (closes && depth > 0) {
                --depth;
            } else if (opens) {
                ++depth;
            }
            pos += found;
            continue;
        }
        ++pos;
    }
    return std::string_view::npos;
}

MdInlineFlags with_flag(MdInlineFlags flags, int use, char delim) {
    (void)delim;
    if (use >= 3) {
        flags.bold = true;
        flags.italic = true;
    } else if (use == 2) {
        flags.bold = true;
    } else {
        flags.italic = true;
    }
    return flags;
}

// Try emphasis at pos (text[pos] is '*' or '_'). Returns consumed length, 0 if not emphasis.
std::size_t try_emphasis(std::string_view text, std::size_t pos, MdInlineFlags active, std::vector<MdSpan>& spans) {
    const char delim = text[pos];
    const std::size_t available = std::min<std::size_t>(char_run(text, pos, delim), 3);
    for (std::size_t use = available; use >= 1; --use) {
        if (!can_open_emphasis(text, pos, use, delim)) {
            continue;
        }
        const std::size_t closer = find_emphasis_closer(text, pos + use, delim, use);
        if (closer == std::string_view::npos || closer == pos + use) {
            continue;
        }
        auto inner = parse_inline(text.substr(pos + use, closer - (pos + use)), with_flag(active, static_cast<int>(use), delim));
        for (auto& span : inner) {
            spans.push_back(std::move(span));
        }
        return closer + use - pos;
    }
    return 0;
}

std::size_t try_code_span(std::string_view text, std::size_t pos, MdInlineFlags active, std::vector<MdSpan>& spans) {
    const std::size_t run = char_run(text, pos, '`');
    std::size_t scan = pos + run;
    while (scan < text.size()) {
        if (text[scan] == '`' && char_run(text, scan, '`') == run) {
            std::string content(text.substr(pos + run, scan - (pos + run)));
            std::replace(content.begin(), content.end(), '\n', ' ');
            const bool all_spaces = std::all_of(content.begin(), content.end(), is_space);
            if (content.size() >= 2 && content.front() == ' ' && content.back() == ' ' && !all_spaces) {
                content = content.substr(1, content.size() - 2);
            }
            MdInlineFlags flags = active;
            flags.code = true;
            push_span(spans, std::move(content), flags);
            return scan + run - pos;
        }
        ++scan;
    }
    return 0;
}

std::size_t try_strike(std::string_view text, std::size_t pos, MdInlineFlags active, std::vector<MdSpan>& spans) {
    if (char_run(text, pos, '~') < 2) {
        return 0;
    }
    const std::size_t closer = text.find("~~", pos + 2);
    if (closer == std::string_view::npos || closer == pos + 2) {
        return 0;
    }
    MdInlineFlags flags = active;
    flags.strike = true;
    auto inner = parse_inline(text.substr(pos + 2, closer - (pos + 2)), flags);
    for (auto& span : inner) {
        spans.push_back(std::move(span));
    }
    return closer + 2 - pos;
}

std::size_t try_link_or_image(std::string_view text, std::size_t pos, MdInlineFlags active,
                              std::vector<MdSpan>& spans) {
    const bool image = text[pos] == '!';
    const std::size_t open = pos + (image ? 2 : 1); // past '['
    if (open > text.size()) {
        return 0;
    }

    std::size_t close_bracket = open;
    while (close_bracket < text.size()) {
        const char c = text[close_bracket];
        if (c == '\\' && close_bracket + 1 < text.size()) {
            close_bracket += 2;
            continue;
        }
        if (c == '[') {
            return 0; // nested brackets unsupported
        }
        if (c == ']') {
            break;
        }
        ++close_bracket;
    }
    if (close_bracket >= text.size() || close_bracket + 1 >= text.size() || text[close_bracket + 1] != '(') {
        return 0;
    }

    std::size_t scan = close_bracket + 2;
    while (scan < text.size() && is_space(text[scan])) {
        ++scan;
    }
    const std::size_t target_start = scan;
    int depth = 0;
    while (scan < text.size()) {
        const char c = text[scan];
        if (c == '\\' && scan + 1 < text.size()) {
            scan += 2;
            continue;
        }
        if (c == '(') {
            ++depth;
        } else if (c == ')') {
            if (depth == 0) {
                break;
            }
            --depth;
        } else if (is_space(c) && depth == 0) {
            break; // optional title follows
        }
        ++scan;
    }
    const std::string_view target = text.substr(target_start, scan - target_start);

    if (scan < text.size() && text[scan] != ')') {
        // Skip optional title ("..." or '...') up to the closing paren.
        while (scan < text.size() && is_space(text[scan])) {
            ++scan;
        }
        if (scan < text.size() && (text[scan] == '"' || text[scan] == '\'')) {
            const char quote = text[scan];
            ++scan;
            while (scan < text.size() && text[scan] != quote) {
                ++scan;
            }
            if (scan < text.size()) {
                ++scan; // past closing quote
            }
            while (scan < text.size() && is_space(text[scan])) {
                ++scan;
            }
        }
    }
    if (scan >= text.size() || text[scan] != ')') {
        return 0;
    }

    MdInlineFlags flags = active;
    flags.link = true;
    flags.image = image;
    auto inner = parse_inline(text.substr(open, close_bracket - open), flags);
    for (auto& span : inner) {
        span.link_target = std::string(target);
        spans.push_back(std::move(span));
    }
    return scan + 1 - pos;
}

std::size_t try_autolink(std::string_view text, std::size_t pos, MdInlineFlags active, std::vector<MdSpan>& spans) {
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
           text[end] != '"' && text[end] != '\'') {
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
    push_span(spans, url, flags, url);
    return url.size();
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
        if (c == '`') {
            consumed = try_code_span(text, pos, active, produced);
        } else if (c == '!' && pos + 1 < text.size() && text[pos + 1] == '[') {
            consumed = try_link_or_image(text, pos, active, produced);
        } else if (c == '[') {
            consumed = try_link_or_image(text, pos, active, produced);
        } else if (c == '*' || c == '_') {
            consumed = try_emphasis(text, pos, active, produced);
        } else if (c == '~') {
            consumed = try_strike(text, pos, active, produced);
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

std::vector<MdSpan> parse_markdown_inline(std::string_view text) { return parse_inline(text, MdInlineFlags{}); }

MarkdownDocument parse_markdown(std::string_view source) {
    MarkdownDocument doc;
    const std::vector<SourceLine> lines = split_source_lines(source);
    std::size_t stable = 0;
    std::size_t index = 0;

    while (index < lines.size()) {
        const SourceLine& line = lines[index];

        if (is_blank(line.text)) {
            stable = line.end;
            ++index;
            continue;
        }

        char fence_char = '\0';
        int fence_length = 0;
        std::string info;
        if (fence_start(line.text, fence_char, fence_length, info)) {
            MdBlock block;
            block.kind = MdBlockKind::CodeBlock;
            block.info = std::move(info);
            ++index;
            while (index < lines.size()) {
                if (fence_end(lines[index].text, fence_char, fence_length)) {
                    stable = lines[index].end;
                    ++index;
                    break;
                }
                block.code_lines.push_back(std::string(lines[index].text));
                ++index;
            }
            doc.blocks.push_back(std::move(block));
            continue;
        }

        if (const int level = heading_level(line.text); level > 0) {
            MdBlock block;
            block.kind = MdBlockKind::Heading;
            block.level = level;
            block.spans = parse_inline(heading_content(line.text), MdInlineFlags{});
            doc.blocks.push_back(std::move(block));
            ++index;
            continue;
        }

        if (is_thematic_break(line.text)) {
            MdBlock block;
            block.kind = MdBlockKind::ThematicBreak;
            doc.blocks.push_back(std::move(block));
            ++index;
            continue;
        }

        if (const int depth = quote_depth(line.text); depth > 0) {
            MdBlock block;
            block.kind = MdBlockKind::Quote;
            block.level = depth;
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
            continue;
        }

        ListMarker marker;
        if (list_marker(line.text, marker)) {
            MdBlock block;
            block.kind = MdBlockKind::ListItem;
            block.ordered = marker.ordered;
            block.number = marker.number;
            block.level = marker.indent / 2;
            block.spans = parse_inline(trim_view(line.text.substr(marker.content_offset)), MdInlineFlags{});
            doc.blocks.push_back(std::move(block));
            ++index;
            continue;
        }

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
