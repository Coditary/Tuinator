#include <tuinator/render/text_edit.hpp>

#include <algorithm>
#include <cctype>

namespace tuinator::text_edit {

bool is_word_char(unsigned char ch) { return std::isalnum(ch) != 0 || ch == '_' || ch == '-'; }

std::size_t previous_word_boundary(std::string_view text, std::size_t pos) {
    if (pos == 0) {
        return 0;
    }

    pos = std::min(pos, text.size());
    while (pos > 0 && !is_word_char(static_cast<unsigned char>(text[pos - 1]))) {
        --pos;
    }
    while (pos > 0 && is_word_char(static_cast<unsigned char>(text[pos - 1]))) {
        --pos;
    }
    return pos;
}

std::size_t next_word_boundary(std::string_view text, std::size_t pos) {
    if (pos >= text.size()) {
        return text.size();
    }

    while (pos < text.size() && !is_word_char(static_cast<unsigned char>(text[pos]))) {
        ++pos;
    }
    while (pos < text.size() && is_word_char(static_cast<unsigned char>(text[pos]))) {
        ++pos;
    }
    return pos;
}

} // namespace tuinator::text_edit
