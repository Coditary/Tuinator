#include <tuinator/render/nerd_glyph_catalog.hpp>

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace tuinator {

namespace {

std::string utf8_from(char32_t cp) {
    std::string out;
    if (cp <= 0x7F) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    return out;
}

char32_t parse_codepoint(const std::string& hex) {
    return static_cast<char32_t>(std::strtoul(hex.c_str(), nullptr, 16));
}

bool parse_tsv_line(const std::string& line, NerdGlyphEntry& entry) {
    if (line.empty() || line.compare(0, 5, "name\t") == 0) {
        return false;
    }
    std::vector<std::string> parts;
    std::istringstream stream(line);
    std::string cell;
    while (std::getline(stream, cell, '\t')) {
        parts.push_back(cell);
    }
    if (parts.size() < 6) {
        return false;
    }
    entry.domain = parts[1];
    entry.category = parts[2];
    entry.path = parts[4];
    entry.codepoint = parse_codepoint(parts[5]);
    return true;
}

void load_tsv_dir(const std::filesystem::path& dir, std::vector<NerdGlyphEntry>& out) {
    if (!std::filesystem::is_directory(dir)) {
        return;
    }
    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.path().extension() == ".tsv") {
            files.push_back(entry.path());
        }
    }
    std::sort(files.begin(), files.end());
    for (const auto& path : files) {
        std::ifstream file(path);
        std::string line;
        while (std::getline(file, line)) {
            NerdGlyphEntry entry;
            if (parse_tsv_line(line, entry)) {
                out.push_back(std::move(entry));
            }
        }
    }
}

} // namespace

std::vector<NerdGlyphDomainSection> load_nerd_glyph_catalog(const std::string& data_root) {
    std::vector<NerdGlyphEntry> entries;
    load_tsv_dir(std::filesystem::path(data_root) / "nerd_icons", entries);
    load_tsv_dir(std::filesystem::path(data_root) / "nerd_glyphs", entries);

    std::sort(entries.begin(), entries.end(), [](const NerdGlyphEntry& a, const NerdGlyphEntry& b) {
        if (a.domain != b.domain) {
            return a.domain < b.domain;
        }
        if (a.category != b.category) {
            return a.category < b.category;
        }
        return a.path < b.path;
    });

    std::vector<NerdGlyphDomainSection> sections;
    for (const NerdGlyphEntry& entry : entries) {
        if (sections.empty() || sections.back().domain != entry.domain) {
            sections.push_back({entry.domain, {}});
        }
        sections.back().entries.push_back(entry);
    }
    return sections;
}

std::string nerd_glyph_utf8(char32_t codepoint) {
    return utf8_from(codepoint);
}

std::size_t nerd_glyph_catalog_total(const std::vector<NerdGlyphDomainSection>& sections) {
    std::size_t total = 0;
    for (const NerdGlyphDomainSection& section : sections) {
        total += section.entries.size();
    }
    return total;
}

} // namespace tuinator
