#pragma once

#include <cstddef>
#include <string_view>

namespace tuinator::text_edit {

bool is_word_char(unsigned char ch);

/// Byte index of the word boundary before `pos` (skips whitespace, then word chars).
std::size_t previous_word_boundary(std::string_view text, std::size_t pos);

/// Byte index of the word boundary after `pos`.
std::size_t next_word_boundary(std::string_view text, std::size_t pos);

} // namespace tuinator::text_edit
