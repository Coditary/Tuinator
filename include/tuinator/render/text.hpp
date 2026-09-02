#pragma once

#include <string_view>

namespace tuinator {

/// Terminal display width of UTF-8 text (falls back to byte length on failure).
int text_display_width(std::string_view text);

/// Byte length of the longest UTF-8 prefix that fits within `max_columns`.
std::size_t text_byte_length_for_width(std::string_view text, int max_columns);

} // namespace tuinator
