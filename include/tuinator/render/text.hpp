#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

/// Encode a Unicode scalar as UTF-8.
std::string utf8_from_codepoint(char32_t codepoint);

/// Byte length of the UTF-8 scalar starting at `index` (returns 1 on invalid lead byte).
std::size_t utf8_char_length(std::string_view text, std::size_t index);

/// Terminal display width of UTF-8 text (falls back to byte length on failure).
int text_display_width(std::string_view text);

/// Byte length of the longest UTF-8 prefix that fits within `max_columns`.
std::size_t text_byte_length_for_width(std::string_view text, int max_columns);

enum class CaretAffinity {
    /// Caret before the glyph occupying `column` (used when starting a selection).
    Before,
    /// Caret after the glyph occupying `column` (used when extending a selection).
    After,
};

/// Map a terminal column to a UTF-8 byte caret index.
std::size_t text_caret_index_at_column(std::string_view text, int column, CaretAffinity affinity);

/// One glyph cluster of a UTF-8 string as it lands on the terminal cell grid.
struct TextGlyph {
    std::size_t offset = 0; ///< Byte offset of the cluster in the source text.
    std::size_t length = 0; ///< Byte length of the cluster.
    int width = 1;          ///< Occupied terminal cells (1 or 2).
};

/// Split UTF-8 text into glyph clusters with their terminal cell widths.
/// Zero-width scalars (combining marks, ZWJ, variation selectors) merge into
/// the preceding cluster.
std::vector<TextGlyph> text_glyph_breaks(std::string_view text);

} // namespace tuinator
