#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace tuinator {

struct NerdGlyphEntry {
    std::string domain;
    std::string category;
    std::string path;
    char32_t codepoint = 0;
};

struct NerdGlyphDomainSection {
    std::string domain;
    std::vector<NerdGlyphEntry> entries;
};

/// Load all glyph TSVs from data/nerd_icons/ and data/nerd_glyphs/.
std::vector<NerdGlyphDomainSection> load_nerd_glyph_catalog(const std::string& data_root);

std::string nerd_glyph_utf8(char32_t codepoint);

std::size_t nerd_glyph_catalog_total(const std::vector<NerdGlyphDomainSection>& sections);

} // namespace tuinator
