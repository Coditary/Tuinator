#include <tuinator/render/glyphs.hpp>

#include <clocale>
#include <cstdlib>
#include <cstring>

namespace tuinator {

namespace {

bool locale_supports_utf8() {
    const char* ctype = std::setlocale(LC_CTYPE, nullptr);
    if (ctype == nullptr) {
        return false;
    }

    return std::strstr(ctype, "UTF-8") != nullptr
        || std::strstr(ctype, "utf8") != nullptr
        || std::strstr(ctype, "UTF8") != nullptr;
}

bool terminal_looks_utf8_capable() {
    const char* term = std::getenv("TERM");
    if (term == nullptr || term[0] == '\0') {
        return true;
    }

    return std::strcmp(term, "dumb") != 0;
}

GlyphSet glyph_set_from_env_override() {
    const char* setting = std::getenv("TUINATOR_GLYPHS");
    if (setting == nullptr || setting[0] == '\0') {
        return GlyphSet::Auto;
    }

    if (std::strcmp(setting, "ascii") == 0) {
        return GlyphSet::Ascii;
    }
    if (std::strcmp(setting, "unicode") == 0 || std::strcmp(setting, "box") == 0) {
        return GlyphSet::Unicode;
    }
    if (std::strcmp(setting, "rounded") == 0) {
        return GlyphSet::UnicodeRounded;
    }

    return GlyphSet::Auto;
}

BorderGlyphs make_glyphs(
    const char* tl,
    const char* tr,
    const char* bl,
    const char* br,
    const char* h,
    const char* v,
    const char* resize) {
    return BorderGlyphs{
        tl,
        tr,
        bl,
        br,
        h,
        v,
        resize,
    };
}

} // namespace

GlyphSet detect_glyph_set() {
    static const GlyphSet cached = []() {
        const GlyphSet override = glyph_set_from_env_override();
        if (override != GlyphSet::Auto) {
            return override;
        }

        const char* force_ascii = std::getenv("TUINATOR_ASCII");
        if (force_ascii != nullptr && force_ascii[0] != '\0' && std::strcmp(force_ascii, "0") != 0) {
            return GlyphSet::Ascii;
        }

        if (!locale_supports_utf8() || !terminal_looks_utf8_capable()) {
            return GlyphSet::Ascii;
        }

        return GlyphSet::Unicode;
    }();
    return cached;
}

BorderGlyphs ascii_border_glyphs() {
    return make_glyphs("+", "+", "+", "+", "-", "|", "#");
}

BorderGlyphs unicode_border_glyphs() {
    return make_glyphs(
        "\xe2\x94\x8c", // ┌
        "\xe2\x94\x90", // ┐
        "\xe2\x94\x94", // └
        "\xe2\x94\x98", // ┘
        "\xe2\x94\x80", // ─
        "\xe2\x94\x82", // │
        "\xe2\x97\xa2"  // ◢
    );
}

BorderGlyphs unicode_rounded_border_glyphs() {
    return make_glyphs(
        "\xe2\x95\xad", // ╭
        "\xe2\x95\xae", // ╮
        "\xe2\x95\xb0", // ╰
        "\xe2\x95\xaf", // ╯
        "\xe2\x94\x80", // ─
        "\xe2\x94\x82", // │
        "\xe2\x97\xa2"  // ◢
    );
}

BorderGlyphs border_glyphs_for(GlyphSet set) {
    if (set == GlyphSet::Auto) {
        set = detect_glyph_set();
    }

    switch (set) {
    case GlyphSet::Ascii:
        return ascii_border_glyphs();
    case GlyphSet::UnicodeRounded:
        return unicode_rounded_border_glyphs();
    case GlyphSet::Unicode:
    case GlyphSet::Auto:
        return unicode_border_glyphs();
    }

    return ascii_border_glyphs();
}

} // namespace tuinator
