#include <tuinator/render/text.hpp>

#include <algorithm>
#include <clocale>
#include <cstdint>
#include <cwchar>
#include <vector>

namespace tuinator {

namespace {

bool locale_ready() {
    static const bool ready = [] {
        return std::setlocale(LC_CTYPE, "") != nullptr;
    }();
    return ready;
}

std::vector<wchar_t> utf8_to_wide(std::string_view text) {
    if (!locale_ready() || text.empty()) {
        return {};
    }

    std::mbstate_t state{};
    const char* cursor = text.data();
    const char* const end = text.data() + text.size();
    std::vector<wchar_t> wide;

    while (cursor < end) {
        wchar_t ch = L'\0';
        const std::size_t consumed =
            std::mbrtowc(&ch, cursor, static_cast<std::size_t>(end - cursor), &state);
        if (consumed == 0) {
            break;
        }
        if (consumed == static_cast<std::size_t>(-1) || consumed == static_cast<std::size_t>(-2)) {
            return {};
        }

        wide.push_back(ch);
        cursor += static_cast<std::ptrdiff_t>(consumed);
    }

    return wide;
}

bool is_emoji_plane(char32_t ch) {
    return ch >= 0x1F000 && ch <= 0x1FAFF;
}

int base_cell_width(char32_t ch) {
    if (ch == 0x200D || ch == 0xFE0E || ch == 0xFE0F) {
        return 0;
    }

    if (is_emoji_plane(ch)) {
        return 2;
    }

    const int cell = wcwidth(static_cast<wchar_t>(ch));
    if (cell < 0) {
        return 1;
    }
    return cell;
}

template <typename Fn>
void for_each_utf8_scalar(std::string_view text, Fn&& fn) {
    std::size_t index = 0;
    while (index < text.size()) {
        const unsigned char lead = static_cast<unsigned char>(text[index]);
        if (lead == '\n') {
            break;
        }

        std::size_t seq = 1;
        char32_t ch = lead;
        if (lead < 0x80) {
            seq = 1;
        } else if ((lead & 0xE0) == 0xC0 && index + 1 < text.size()) {
            seq = 2;
            ch = static_cast<char32_t>(lead & 0x1F) << 6;
            ch |= static_cast<char32_t>(static_cast<unsigned char>(text[index + 1]) & 0x3F);
        } else if ((lead & 0xF0) == 0xE0 && index + 2 < text.size()) {
            seq = 3;
            ch = static_cast<char32_t>(lead & 0x0F) << 12;
            ch |= static_cast<char32_t>(static_cast<unsigned char>(text[index + 1]) & 0x3F) << 6;
            ch |= static_cast<char32_t>(static_cast<unsigned char>(text[index + 2]) & 0x3F);
        } else if ((lead & 0xF8) == 0xF0 && index + 3 < text.size()) {
            seq = 4;
            ch = static_cast<char32_t>(lead & 0x07) << 18;
            ch |= static_cast<char32_t>(static_cast<unsigned char>(text[index + 1]) & 0x3F) << 12;
            ch |= static_cast<char32_t>(static_cast<unsigned char>(text[index + 2]) & 0x3F) << 6;
            ch |= static_cast<char32_t>(static_cast<unsigned char>(text[index + 3]) & 0x3F);
        } else {
            seq = std::min<std::size_t>(1, text.size() - index);
            ch = lead;
        }

        seq = std::min(seq, text.size() - index);
        fn(ch, seq);
        index += seq;
    }
}

int width_of_scalars(const std::vector<char32_t>& chars) {
    int width = 0;
    int last = 0;
    for (char32_t ch : chars) {
        if (ch == 0xFE0F) {
            if (last == 1) {
                width += 1;
                last = 2;
            }
            continue;
        }

        const int cell = base_cell_width(ch);
        if (cell <= 0) {
            continue;
        }

        width += cell;
        last = cell;
    }
    return width;
}

} // namespace

int text_display_width(std::string_view text) {
    if (text.empty()) {
        return 0;
    }

    const std::vector<wchar_t> wide = utf8_to_wide(text);
    if (!wide.empty()) {
        std::vector<char32_t> scalars(wide.begin(), wide.end());
        return width_of_scalars(scalars);
    }

    int width = 0;
    int last = 0;
    for_each_utf8_scalar(text, [&](char32_t ch, std::size_t) {
        if (ch == 0xFE0F) {
            if (last == 1) {
                width += 1;
                last = 2;
            }
            return;
        }

        int cell = base_cell_width(ch);
        if (cell <= 0 && ch >= 0x80) {
            cell = (ch >= 0x10000) ? 2 : 1;
        }
        if (cell <= 0) {
            return;
        }

        width += cell;
        last = cell;
    });
    return width;
}

std::size_t text_byte_length_for_width(std::string_view text, int max_columns) {
    if (max_columns <= 0 || text.empty()) {
        return 0;
    }

    int columns = 0;
    int last = 0;
    std::size_t accepted = 0;
    bool ok = true;

    for_each_utf8_scalar(text, [&](char32_t ch, std::size_t bytes) {
        if (!ok || ch == '\n') {
            ok = false;
            return;
        }

        if (ch == 0xFE0F) {
            const int extra = (last == 1) ? 1 : 0;
            if (columns + extra > max_columns) {
                ok = false;
                return;
            }
            columns += extra;
            if (extra > 0) {
                last = 2;
            }
            accepted += bytes;
            return;
        }

        const int cell = base_cell_width(ch);
        if (cell <= 0) {
            accepted += bytes;
            return;
        }
        if (columns + cell > max_columns) {
            ok = false;
            return;
        }

        columns += cell;
        last = cell;
        accepted += bytes;
    });
    return accepted;
}

} // namespace tuinator
