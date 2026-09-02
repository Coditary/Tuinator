#include <tuinator/widgets/display/nerd_glyph_matrix_gallery.hpp>

#include <tuinator/render/text.hpp>

#include <cstdio>
#include <string>

namespace tuinator {

namespace {

int matrix_rows_for(std::size_t count, int columns) {
    if (count == 0 || columns <= 0) {
        return 0;
    }
    return static_cast<int>((count + static_cast<std::size_t>(columns) - 1) / static_cast<std::size_t>(columns));
}

} // namespace

NerdGlyphMatrixGallery::NerdGlyphMatrixGallery(
    std::vector<NerdGlyphDomainSection> sections,
    int columns,
    NerdGlyphMatrixGalleryStyle style)
    : sections_(std::move(sections))
    , columns_(columns > 0 ? columns : 32)
    , style_(std::move(style)) {}

Size NerdGlyphMatrixGallery::preferred_size() const {
    int rows = 1; // title
    for (const NerdGlyphDomainSection& section : sections_) {
        if (section.entries.empty()) {
            continue;
        }
        rows += 1; // domain header
        std::string last_category;
        std::size_t category_count = 0;
        auto flush_category = [&](std::size_t count) {
            rows += matrix_rows_for(count, columns_);
        };
        for (const NerdGlyphEntry& entry : section.entries) {
            if (entry.category != last_category) {
                if (!last_category.empty()) {
                    flush_category(category_count);
                    category_count = 0;
                }
                rows += 1;
                last_category = entry.category;
            }
            ++category_count;
        }
        flush_category(category_count);
    }
    const int width = columns_ * 2 + 8;
    return {std::max(40, width), std::max(1, rows)};
}

void NerdGlyphMatrixGallery::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    canvas.fill_rect(bounds_, ' ', style_.background);

    int y = bounds_.y;
    const int x = bounds_.x;

    auto draw_line = [&](const std::string& text, const Style& style) {
        if (y >= bounds_.y + bounds_.height) {
            return;
        }
        canvas.draw_text({x, y}, text, style);
        ++y;
    };

    char title[96];
    std::snprintf(
        title,
        sizeof(title),
        "Nerd Glyph Matrix (%zu glyphs, %d cols)",
        nerd_glyph_catalog_total(sections_),
        columns_);
    draw_line(title, style_.title);

    for (const NerdGlyphDomainSection& section : sections_) {
        if (section.entries.empty()) {
            continue;
        }

        char header[96];
        std::snprintf(
            header,
            sizeof(header),
            "[[ %s ]] (%zu)",
            section.domain.c_str(),
            section.entries.size());
        draw_line(header, style_.domain_header);

        std::string last_category;
        int col = 0;
        std::string row;

        auto flush_row = [&]() {
            if (row.empty()) {
                return;
            }
            draw_line(row, style_.glyph);
            row.clear();
            col = 0;
        };

        for (const NerdGlyphEntry& entry : section.entries) {
            if (entry.category != last_category) {
                flush_row();
                char category_line[96];
                std::snprintf(category_line, sizeof(category_line), "-- %s --", entry.category.c_str());
                draw_line(category_line, style_.category_header);
                last_category = entry.category;
            }

            const std::string glyph = nerd_glyph_utf8(entry.codepoint);
            if (col >= columns_) {
                flush_row();
            }
            if (!row.empty()) {
                row.push_back(' ');
            }
            row += glyph;
            ++col;
        }
        flush_row();
    }
}

} // namespace tuinator
