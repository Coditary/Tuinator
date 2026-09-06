#pragma once

#include <tuinator/render/nerd_icon_set.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/widgets/widget.hpp>

namespace tuinator {

struct NerdIconCatalogGalleryStyle {
    Style background{};
    Style title{};
    Style domain_header{};
    Style category_header{};
    Style row_text{};
    Style icon{};
};

/// Catalog of all 228 Nerd Fonts weather-set glyphs split into semantic domains.
class NerdIconCatalogGallery : public Widget {
  public:
    explicit NerdIconCatalogGallery(NerdIconCatalogGalleryStyle style = {});

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;

  private:
    NerdIconCatalogGalleryStyle style_;
};

} // namespace tuinator
