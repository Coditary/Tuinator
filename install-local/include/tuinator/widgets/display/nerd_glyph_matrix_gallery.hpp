#pragma once

#include <tuinator/render/nerd_glyph_catalog.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/widgets/widget.hpp>

#include <vector>

namespace tuinator {

struct NerdGlyphMatrixGalleryStyle {
    Style background{};
    Style title{};
    Style domain_header{};
    Style category_header{};
    Style glyph{};
};

/// Matrix catalog of all exported Nerd Font glyphs (multiple icons per row).
class NerdGlyphMatrixGallery : public Widget {
public:
    NerdGlyphMatrixGallery(
        std::vector<NerdGlyphDomainSection> sections,
        int columns = 32,
        NerdGlyphMatrixGalleryStyle style = {});

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;

    const std::vector<NerdGlyphDomainSection>& sections() const { return sections_; }
    int columns() const { return columns_; }

private:
    std::vector<NerdGlyphDomainSection> sections_;
    int columns_;
    NerdGlyphMatrixGalleryStyle style_;
};

} // namespace tuinator
