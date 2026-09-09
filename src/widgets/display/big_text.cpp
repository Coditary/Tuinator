#include <tuinator/render/color.hpp>
#include <tuinator/widgets/display/big_text.hpp>

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>

#include "widgets/display/big_text_fonts.inc"

namespace tuinator {

namespace {

#include "widgets/display/big_text_tables.inc"

bool glyph_bit(unsigned char ch, int row, int col) {
    if (ch < 32 || ch > 127 || row < 0 || row > 7 || col < 0 || col > 7) {
        return false;
    }
    return (kFont8x8[ch - 32][row] & (1U << col)) != 0;
}

void append_utf8(std::string& out, char32_t cp) {
    if (cp < 0x80) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

std::string utf8_from(char32_t cp) {
    std::string out;
    append_utf8(out, cp);
    return out;
}

bool is_figlet(BigTextKind kind) {
    switch (kind) {
    case BigTextKind::Slant:
    case BigTextKind::Standard:
    case BigTextKind::Small:
    case BigTextKind::Big:
    case BigTextKind::Doom:
    case BigTextKind::FigletShadow: return true;
    default: return false;
    }
}

const char* figlet_id(BigTextKind kind) {
    switch (kind) {
    case BigTextKind::Slant: return "slant";
    case BigTextKind::Standard: return "standard";
    case BigTextKind::Small: return "small";
    case BigTextKind::Big: return "big";
    case BigTextKind::Doom: return "doom";
    case BigTextKind::FigletShadow: return "shadow";
    default: return nullptr;
    }
}

void pixels_per_cell(BigTextKind kind, int& px, int& py) {
    switch (kind) {
    case BigTextKind::Half:
    case BigTextKind::Shade:
        px = 1;
        py = 2;
        return;
    case BigTextKind::Narrow:
        px = 2;
        py = 1;
        return;
    case BigTextKind::Quadrant:
        px = 2;
        py = 2;
        return;
    case BigTextKind::Third:
        px = 1;
        py = 3;
        return;
    case BigTextKind::Sextant:
        px = 2;
        py = 3;
        return;
    case BigTextKind::Quarter:
        px = 1;
        py = 4;
        return;
    case BigTextKind::Octant:
    case BigTextKind::Braille:
        px = 2;
        py = 4;
        return;
    default:
        px = 1;
        py = 1;
        return;
    }
}

char32_t pack_cell(BigTextKind kind, const std::uint8_t sample[8], int count) {
    auto on = [&](int i) -> int { return (i < count && sample[i] != 0) ? 1 : 0; };

    switch (kind) {
    case BigTextKind::Half:
        if (!on(0) && !on(1)) {
            return U' ';
        }
        if (on(0) && !on(1)) {
            return U'▀';
        }
        if (!on(0) && on(1)) {
            return U'▄';
        }
        return U'█';
    case BigTextKind::Narrow:
        if (!on(0) && !on(1)) {
            return U' ';
        }
        if (on(0) && !on(1)) {
            return U'▌';
        }
        if (!on(0) && on(1)) {
            return U'▐';
        }
        return U'█';
    case BigTextKind::Quadrant: {
        const int idx = on(0) | (on(1) << 1) | (on(2) << 2) | (on(3) << 3);
        return kQuadrant[idx];
    }
    case BigTextKind::Third:
        return kSextant[on(0) | (on(0) << 1) | (on(1) << 2) | (on(1) << 3) | (on(2) << 4) | (on(2) << 5)];
    case BigTextKind::Sextant:
        return kSextant[on(0) | (on(1) << 1) | (on(2) << 2) | (on(3) << 3) | (on(4) << 4) | (on(5) << 5)];
    case BigTextKind::Quarter:
        return kOctant[on(0) | (on(0) << 1) | (on(1) << 2) | (on(1) << 3) | (on(2) << 4) | (on(2) << 5) | (on(3) << 6) |
                       (on(3) << 7)];
    case BigTextKind::Octant:
        return kOctant[on(0) | (on(1) << 1) | (on(2) << 2) | (on(3) << 3) | (on(4) << 4) | (on(5) << 5) | (on(6) << 6) |
                       (on(7) << 7)];
    case BigTextKind::Braille: {
        static const int bits[8] = {0, 3, 1, 4, 2, 5, 6, 7};
        int mask = 0;
        for (int i = 0; i < 8; ++i) {
            if (on(i)) {
                mask |= 1 << bits[i];
            }
        }
        return static_cast<char32_t>(0x2800 + mask);
    }
    case BigTextKind::Shade: {
        const int n = on(0) + on(1);
        if (n == 0) {
            return U' ';
        }
        if (n == 1) {
            return on(0) ? U'▀' : U'▄';
        }
        return U'█';
    }
    default: return on(0) ? U'█' : U' ';
    }
}

struct Bitmap {
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> px;
    std::vector<int> letter;
    std::vector<char> letter_ch;

    std::uint8_t at(int x, int y) const {
        if (x < 0 || y < 0 || x >= width || y >= height) {
            return 0;
        }
        return px[static_cast<std::size_t>(y * width + x)];
    }

    int letter_at(int x, int y) const {
        if (x < 0 || y < 0 || x >= width || y >= height) {
            return 0;
        }
        return letter[static_cast<std::size_t>(y * width + x)];
    }

    void set(int x, int y, std::uint8_t value, int lit) {
        if (x < 0 || y < 0 || x >= width || y >= height) {
            return;
        }
        const std::size_t i = static_cast<std::size_t>(y * width + x);
        if (value >= px[i]) {
            px[i] = value;
            letter[i] = lit;
        }
    }
};

void stamp_ascii(Bitmap& bmp, int origin_x, unsigned char ch, int letter, int scale, int* next_x) {
    int max_x = origin_x;
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (!glyph_bit(ch, row, col)) {
                continue;
            }
            for (int sy = 0; sy < scale; ++sy) {
                for (int sx = 0; sx < scale; ++sx) {
                    const int x = origin_x + col * scale + sx;
                    const int y = row * scale + sy;
                    bmp.set(x, y, 2, letter);
                    max_x = std::max(max_x, x + 1);
                }
            }
        }
    }
    if (ch == ' ') {
        max_x = origin_x + 4 * scale;
    }
    *next_x = max_x;
}

Bitmap make_bitmap(std::string_view line, const BigTextOptions& options) {
    const int scale = std::max(1, options.scale);
    const int spacing = std::max(0, options.letter_spacing) * scale;
    int width = 0;
    for (unsigned char ch : line) {
        if (ch == '\n') {
            break;
        }
        width += 8 * scale + spacing;
    }
    if (width > 0) {
        width -= spacing;
    }

    Bitmap bmp;
    bmp.width = std::max(1, width);
    bmp.height = 8 * scale;
    bmp.px.assign(static_cast<std::size_t>(bmp.width * bmp.height), 0);
    bmp.letter.assign(bmp.px.size(), 0);

    int x = 0;
    int letter = 0;
    for (unsigned char ch : line) {
        if (ch == '\n') {
            break;
        }
        if (ch < 32 || ch > 126) {
            ch = '?';
        }
        bmp.letter_ch.push_back(static_cast<char>(ch));
        int next = x;
        stamp_ascii(bmp, x, ch, letter, scale, &next);
        x = next + spacing;
        ++letter;
    }
    return bmp;
}

void apply_outline(Bitmap& bmp) {
    Bitmap copy = bmp;
    std::fill(bmp.px.begin(), bmp.px.end(), 0);
    for (int y = 0; y < copy.height; ++y) {
        for (int x = 0; x < copy.width; ++x) {
            if (copy.at(x, y) == 0) {
                continue;
            }
            const bool edge =
                copy.at(x - 1, y) == 0 || copy.at(x + 1, y) == 0 || copy.at(x, y - 1) == 0 || copy.at(x, y + 1) == 0;
            if (edge) {
                bmp.set(x, y, 2, copy.letter_at(x, y));
            }
        }
    }
}

int clamp_int(int value, int lo, int hi) { return std::max(lo, std::min(hi, value)); }

int effective_shadow_layers(BigTextKind kind, const BigTextOptions& options) {
    if (options.shadow_layers > 0) {
        return clamp_int(options.shadow_layers, 1, 8);
    }
    if (kind == BigTextKind::Stacked) {
        return 2;
    }
    if (kind == BigTextKind::Isometric) {
        return 1;
    }
    if (!options.shadow_style.has_value()) {
        return 0;
    }
    switch (kind) {
    case BigTextKind::Block:
    case BigTextKind::Outline:
    case BigTextKind::Letter:
    case BigTextKind::Banner: return 1;
    default: return 0;
    }
}

void apply_drop_shadow(Bitmap& src, int dx, int dy, int layers, bool edges_only) {
    dx = clamp_int(dx, -8, 8);
    dy = clamp_int(dy, -8, 8);
    layers = clamp_int(layers, 0, 8);
    if (layers <= 0 || (dx == 0 && dy == 0)) {
        return;
    }

    int min_x = 0;
    int min_y = 0;
    int max_x = src.width;
    int max_y = src.height;
    for (int i = 1; i <= layers; ++i) {
        min_x = std::min(min_x, i * dx);
        min_y = std::min(min_y, i * dy);
        max_x = std::max(max_x, src.width + i * dx);
        max_y = std::max(max_y, src.height + i * dy);
    }
    const int origin_x = -min_x;
    const int origin_y = -min_y;

    Bitmap out;
    out.width = max_x - min_x;
    out.height = max_y - min_y;
    out.px.assign(static_cast<std::size_t>(out.width * out.height), 0);
    out.letter.assign(out.px.size(), 0);
    out.letter_ch = src.letter_ch;

    auto stamp = [&](int shift_x, int shift_y, std::uint8_t value) {
        for (int y = 0; y < src.height; ++y) {
            for (int x = 0; x < src.width; ++x) {
                if (src.at(x, y) == 0) {
                    continue;
                }
                if (edges_only) {
                    const bool edge = src.at(x - 1, y) == 0 || src.at(x + 1, y) == 0 || src.at(x, y - 1) == 0 ||
                                      src.at(x, y + 1) == 0;
                    if (!edge) {
                        continue;
                    }
                }
                out.set(x + origin_x + shift_x, y + origin_y + shift_y, value, src.letter_at(x, y));
            }
        }
    };

    for (int i = layers; i >= 1; --i) {
        stamp(i * dx, i * dy, 1);
    }
    for (int y = 0; y < src.height; ++y) {
        for (int x = 0; x < src.width; ++x) {
            if (src.at(x, y) != 0) {
                out.set(x + origin_x, y + origin_y, 2, src.letter_at(x, y));
            }
        }
    }
    src = std::move(out);
}

struct RasterCell {
    std::string glyph;
    int letter = 0;
    bool shadow = false;
};

void pack_bitmap(const Bitmap& bmp, BigTextKind kind, std::vector<RasterCell>& cells, int& width, int& height) {
    int px = 1;
    int py = 1;
    pixels_per_cell(kind, px, py);
    width = (bmp.width + px - 1) / px;
    height = (bmp.height + py - 1) / py;
    cells.assign(static_cast<std::size_t>(width * height), RasterCell{});

    auto sample = [&](int x, int y) -> std::uint8_t { return bmp.at(x, y); };

    for (int cy = 0; cy < height; ++cy) {
        for (int cx = 0; cx < width; ++cx) {
            std::uint8_t bits[8]{};
            int n = 0;
            int letter = 0;
            bool shadow = false;
            for (int iy = 0; iy < py; ++iy) {
                for (int ix = 0; ix < px; ++ix) {
                    const int x = cx * px + ix;
                    const int y = cy * py + iy;
                    bits[n] = sample(x, y);
                    if (bits[n] != 0) {
                        letter = bmp.letter_at(x, y);
                        shadow = bits[n] == 1;
                    }
                    ++n;
                }
            }

            RasterCell cell;
            cell.letter = letter;
            cell.shadow = shadow;
            if (kind == BigTextKind::Letter && !bmp.letter_ch.empty()) {
                const bool any = std::any_of(bits, bits + n, [](std::uint8_t v) { return v != 0; });
                const char ink = bmp.letter_ch[static_cast<std::size_t>(
                    std::clamp(letter, 0, static_cast<int>(bmp.letter_ch.size()) - 1))];
                cell.glyph = any ? std::string(1, ink == ' ' ? ' ' : ink) : " ";
            } else if (kind == BigTextKind::Banner) {
                const bool any = std::any_of(bits, bits + n, [](std::uint8_t v) { return v != 0; });
                cell.glyph = any ? "#" : " ";
            } else if (kind == BigTextKind::Block || kind == BigTextKind::Outline || kind == BigTextKind::Stacked ||
                       kind == BigTextKind::Isometric) {
                const bool fill = std::any_of(bits, bits + n, [](std::uint8_t v) { return v == 2; });
                const bool dim = std::any_of(bits, bits + n, [](std::uint8_t v) { return v == 1; });
                if (fill) {
                    cell.glyph = utf8_from(U'█');
                } else if (dim) {
                    cell.glyph = utf8_from(U'░');
                    cell.shadow = true;
                } else {
                    cell.glyph = " ";
                }
            } else {
                cell.glyph = utf8_from(pack_cell(kind, bits, n));
            }
            cells[static_cast<std::size_t>(cy * width + cx)] = std::move(cell);
        }
    }
}

std::vector<std::string_view> split_rows(std::string_view glyph) {
    std::vector<std::string_view> rows;
    std::size_t start = 0;
    while (start <= glyph.size()) {
        const std::size_t end = glyph.find('\n', start);
        if (end == std::string_view::npos) {
            rows.emplace_back(glyph.substr(start));
            break;
        }
        rows.emplace_back(glyph.substr(start, end - start));
        start = end + 1;
    }
    return rows;
}

void raster_figlet(std::string_view line, BigTextKind kind, std::vector<RasterCell>& cells, int& width, int& height) {
    const auto* font = big_text_fonts::figlet_font(figlet_id(kind));
    if (font == nullptr) {
        width = 0;
        height = 0;
        cells.clear();
        return;
    }
    height = font->height;
    std::vector<std::string> rows(static_cast<std::size_t>(height));
    std::vector<std::vector<int>> owners(static_cast<std::size_t>(height));

    int letter = 0;
    for (unsigned char ch : line) {
        if (ch == '\n') {
            break;
        }
        if (ch < 32 || ch > 126) {
            ch = '?';
        }
        const auto glyph_rows = split_rows(font->glyphs[ch - 32]);
        int gw = 0;
        for (std::string_view row : glyph_rows) {
            gw = std::max(gw, static_cast<int>(row.size()));
        }
        for (int r = 0; r < height; ++r) {
            std::string_view row =
                r < static_cast<int>(glyph_rows.size()) ? glyph_rows[static_cast<std::size_t>(r)] : std::string_view{};
            rows[static_cast<std::size_t>(r)].append(row);
            const int pad = gw - static_cast<int>(row.size());
            if (pad > 0) {
                rows[static_cast<std::size_t>(r)].append(static_cast<std::size_t>(pad), ' ');
            }
            for (int c = 0; c < gw; ++c) {
                owners[static_cast<std::size_t>(r)].push_back(letter);
            }
        }
        ++letter;
    }

    width = rows.empty() ? 0 : static_cast<int>(rows[0].size());
    cells.assign(static_cast<std::size_t>(std::max(0, width * height)), RasterCell{});
    for (int y = 0; y < height; ++y) {
        const std::string& row = rows[static_cast<std::size_t>(y)];
        for (int x = 0; x < width && x < static_cast<int>(row.size()); ++x) {
            RasterCell cell;
            cell.glyph = std::string(1, row[static_cast<std::size_t>(x)]);
            if (x < static_cast<int>(owners[static_cast<std::size_t>(y)].size())) {
                cell.letter = owners[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
            }
            cells[static_cast<std::size_t>(y * width + x)] = std::move(cell);
        }
    }
}

void append_underline(std::vector<RasterCell>& cells, int& width, int& height, std::string_view pattern) {
    if (pattern.empty() || width <= 0) {
        return;
    }
    std::vector<std::string> glyphs;
    // Treat each byte as a cell for ASCII patterns; UTF-8 patterns walk codepoints loosely.
    for (char ch : pattern) {
        if (ch != '\n') {
            glyphs.emplace_back(1, ch);
        }
    }
    if (glyphs.empty()) {
        return;
    }
    const int y = height;
    ++height;
    cells.resize(static_cast<std::size_t>(width * height));
    for (int x = 0; x < width; ++x) {
        RasterCell cell;
        cell.glyph = glyphs[static_cast<std::size_t>(x) % glyphs.size()];
        cell.letter = 0;
        cells[static_cast<std::size_t>(y * width + x)] = std::move(cell);
    }
}

std::vector<std::string_view> split_lines(const std::string& text) {
    std::vector<std::string_view> lines;
    std::size_t start = 0;
    while (start <= text.size()) {
        const std::size_t end = text.find('\n', start);
        if (end == std::string::npos) {
            lines.emplace_back(text.data() + start, text.size() - start);
            break;
        }
        lines.emplace_back(text.data() + start, end - start);
        start = end + 1;
    }
    if (lines.empty()) {
        lines.emplace_back("");
    }
    return lines;
}

Rgb lerp_rgb(Rgb from, Rgb to, float amount) {
    const auto mix = [](int a, int b, float t) {
        return static_cast<std::uint8_t>(static_cast<float>(a) + (static_cast<float>(b - a) * t));
    };

    return {mix(from.r, to.r, amount), mix(from.g, to.g, amount), mix(from.b, to.b, amount)};
}

Rgb sample_big_text_gradient(const std::vector<BigTextGradientStop>& stops, float t) {
    if (stops.empty()) {
        return {};
    }
    if (stops.size() == 1) {
        return stops.front().color;
    }

    const float clamped = std::clamp(t, 0.0f, 1.0f);
    if (clamped <= stops.front().position) {
        return stops.front().color;
    }
    if (clamped >= stops.back().position) {
        return stops.back().color;
    }

    for (std::size_t index = 1; index < stops.size(); ++index) {
        const BigTextGradientStop& right = stops[index];
        const BigTextGradientStop& left = stops[index - 1];
        if (clamped > right.position) {
            continue;
        }

        const float span = right.position - left.position;
        const float local = span > 0.0f ? (clamped - left.position) / span : 0.0f;
        return lerp_rgb(left.color, right.color, local);
    }

    return stops.back().color;
}

Style style_from_gradient(const Style& base, Rgb rgb) { return ColorValue::from_rgb(rgb).foreground_style(base); }

} // namespace

const std::vector<BigTextPreset>& all_big_text_styles() {
    static const std::vector<BigTextPreset> styles = {
        {"block", "Full size", BigTextKind::Block, {"full", "pixel"}},
        {"half", "1/2 high", BigTextKind::Half, {"half-height"}},
        {"narrow", "1/2 wide", BigTextKind::Narrow, {"half-width"}},
        {"quadrant", "Quadrant", BigTextKind::Quadrant, {"quad"}},
        {"third", "1/3 high", BigTextKind::Third, {}},
        {"sextant", "Sextant", BigTextKind::Sextant, {}},
        {"quarter", "1/4 high", BigTextKind::Quarter, {}},
        {"octant", "Octant", BigTextKind::Octant, {}},
        {"braille", "Braille", BigTextKind::Braille, {"dots"}},
        {"shade", "Shade", BigTextKind::Shade, {"blocks"}},
        {"letter", "Letter fill", BigTextKind::Letter, {"self"}},
        {"banner", "Banner #", BigTextKind::Banner, {"hash"}},
        {"outline", "Outline", BigTextKind::Outline, {"wire"}},
        {"stacked", "Stacked shadow", BigTextKind::Stacked, {"nexis", "double-shadow"}},
        {"isometric", "Isometric", BigTextKind::Isometric, {"3d"}},
        {"slant", "Slant", BigTextKind::Slant, {"figlet"}},
        {"standard", "Standard", BigTextKind::Standard, {}},
        {"small", "Small", BigTextKind::Small, {}},
        {"big", "Big", BigTextKind::Big, {}},
        {"doom", "Doom", BigTextKind::Doom, {}},
        {"figlet-shadow", "Shadow", BigTextKind::FigletShadow, {"shadow"}},
    };
    return styles;
}

const BigTextPreset* big_text_style_named(std::string_view name) {
    for (const BigTextPreset& style : all_big_text_styles()) {
        if (name == style.id || name == style.title) {
            return &style;
        }
        for (const std::string& alias : style.aliases) {
            if (name == alias) {
                return &style;
            }
        }
    }
    return nullptr;
}

BigTextKind big_text_kind_named(std::string_view name, BigTextKind fallback) {
    if (const BigTextPreset* style = big_text_style_named(name)) {
        return style->kind;
    }
    return fallback;
}

std::vector<BigTextGradientStop> big_text_gradient(std::initializer_list<std::pair<float, std::uint32_t>> hex_stops) {
    std::vector<BigTextGradientStop> stops;
    stops.reserve(hex_stops.size());
    for (const auto& stop : hex_stops) {
        stops.push_back({stop.first, Rgb::hex(stop.second)});
    }
    return stops;
}

void apply_big_text_look(BigTextOptions& options, const BigTextLook& look) {
    options.kind = look.kind;
    options.scale = look.scale;
    options.gradient_stops = look.gradient_stops;
    options.gradient_axis = look.gradient_axis;
    options.shadow_style = look.shadow_style;
    options.shadow_dx = look.shadow_dx;
    options.shadow_dy = look.shadow_dy;
    options.shadow_layers = look.shadow_layers;
    options.rainbow = look.rainbow;
}

const std::vector<BigTextLook>& all_big_text_looks() {
    static const std::vector<BigTextLook> looks = {
        {
            "omarchy",
            "Omarchy / synthwave",
            BigTextKind::Stacked,
            1,
            big_text_gradient({
                {0.0f, 0xFFB347},
                {0.45f, 0xFF2A5C},
                {1.0f, 0xC020A0},
            }),
            BigTextGradientAxis::Vertical,
            style_fg(Rgb::hex(0x5A1050)),
            1,
            2,
            2,
        },
        {
            "latchdark",
            "Latchdark mono",
            BigTextKind::Block,
            1,
            big_text_gradient({
                {0.0f, 0x101010},
                {0.35f, 0x505050},
                {0.7f, 0xB0B0B0},
                {1.0f, 0xF5F5F5},
            }),
            BigTextGradientAxis::Vertical,
            style_fg(Rgb::hex(0x202020)),
            1,
            1,
            1,
        },
        {
            "sunset",
            "Sunset sweep",
            BigTextKind::Block,
            1,
            big_text_gradient({
                {0.0f, 0xFF8C00},
                {0.5f, 0xFF3D7A},
                {1.0f, 0x7B2D8E},
            }),
            BigTextGradientAxis::Horizontal,
            style_fg(Rgb::hex(0x3A1848)),
            2,
            1,
            1,
        },
        {
            "ocean",
            "Deep ocean",
            BigTextKind::Quadrant,
            1,
            big_text_gradient({
                {0.0f, 0x7EE8FA},
                {0.5f, 0x1B6CA8},
                {1.0f, 0x0A1F44},
            }),
            BigTextGradientAxis::Vertical,
            style_fg(Rgb::hex(0x061428)),
            1,
            2,
            1,
        },
        {
            "neon",
            "Neon sweep",
            BigTextKind::Block,
            1,
            big_text_gradient({
                {0.0f, 0xFF00FF},
                {0.33f, 0x00FFFF},
                {0.66f, 0x39FF14},
                {1.0f, 0xFFFF00},
            }),
            BigTextGradientAxis::Horizontal,
            style_fg(Rgb::hex(0x1A0030)),
            2,
            1,
            1,
        },
        {
            "ice",
            "Ice outline",
            BigTextKind::Outline,
            1,
            big_text_gradient({
                {0.0f, 0xE8F8FF},
                {0.5f, 0x7AD7F0},
                {1.0f, 0x2E86AB},
            }),
            BigTextGradientAxis::Vertical,
            style_fg(Rgb::hex(0x123A52)),
            1,
            1,
            1,
        },
        {
            "doom-fire",
            "Doom fire",
            BigTextKind::Doom,
            1,
            big_text_gradient({
                {0.0f, 0xFFE066},
                {0.4f, 0xFF5500},
                {1.0f, 0x8B0000},
            }),
            BigTextGradientAxis::Vertical,
            style_fg(Rgb::hex(0x3A0800)),
            1,
            1,
            1,
        },
    };
    return looks;
}

const BigTextLook* big_text_look_named(std::string_view name) {
    for (const BigTextLook& look : all_big_text_looks()) {
        if (name == look.id || name == look.title) {
            return &look;
        }
    }
    return nullptr;
}

BigText::BigText(std::string text, Style style) : BigText(std::move(text), BigTextOptions{}, style) {}

BigText::BigText(std::string text, BigTextKind kind, Style style) : text_(std::move(text)), style_(style) {
    options_.kind = kind;
}

BigText::BigText(std::string text, const BigTextOptions& options, Style style)
    : text_(std::move(text)), options_(options), style_(style) {
    options_.scale = std::max(1, options_.scale);
}

BigText::BigText(std::string text, std::string_view style_name, Style style)
    : BigText(std::move(text), big_text_kind_named(style_name), style) {}

void BigText::set_text(std::string text) {
    text_ = std::move(text);
    dirty_cache_ = true;
    mark_dirty();
}

void BigText::set_kind(BigTextKind kind) {
    options_.kind = kind;
    dirty_cache_ = true;
    mark_dirty();
}

void BigText::set_options(BigTextOptions options) {
    options.scale = std::max(1, options.scale);
    options_ = std::move(options);
    dirty_cache_ = true;
    mark_dirty();
}

void BigText::set_style(Style style) {
    style_ = style;
    mark_dirty();
}

void BigText::set_shadow_style(Style style) {
    const bool needs_rebuild = options_.kind != BigTextKind::Stacked && options_.kind != BigTextKind::Isometric &&
                               !options_.shadow_style.has_value();
    options_.shadow_style = style;
    if (needs_rebuild) {
        dirty_cache_ = true;
    }
    mark_dirty();
}

void BigText::rebuild() const {
    if (!dirty_cache_) {
        return;
    }
    dirty_cache_ = false;
    cache_cells_.clear();
    cache_width_ = 0;
    cache_height_ = 0;

    int y = 0;
    for (std::string_view line : split_lines(text_)) {
        std::vector<RasterCell> cells;
        int width = 0;
        int height = 0;
        if (is_figlet(options_.kind)) {
            raster_figlet(line, options_.kind, cells, width, height);
        } else {
            Bitmap bmp = make_bitmap(line, options_);
            if (options_.kind == BigTextKind::Outline) {
                apply_outline(bmp);
            }
            const int layers = effective_shadow_layers(options_.kind, options_);
            if (layers > 0) {
                apply_drop_shadow(bmp, options_.shadow_dx, options_.shadow_dy, layers,
                                  options_.kind == BigTextKind::Stacked);
            }
            pack_bitmap(bmp, options_.kind, cells, width, height);
        }
        if (!options_.underline.empty()) {
            append_underline(cells, width, height, options_.underline);
        }

        auto to_cache = [](const RasterCell& cell) {
            Cell out;
            out.glyph = cell.glyph;
            out.letter = cell.letter;
            out.shadow = cell.shadow;
            return out;
        };

        if (cache_width_ == 0) {
            cache_width_ = width;
            cache_height_ = height;
            cache_cells_.clear();
            cache_cells_.reserve(cells.size());
            for (const RasterCell& cell : cells) {
                cache_cells_.push_back(to_cache(cell));
            }
            // y tracks the vertical cursor for stacked lines after the first raster.
            // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
            y = height;
            continue;
        }

        const int gap = 1;
        const int new_width = std::max(cache_width_, width);
        const int new_height = cache_height_ + gap + height;
        std::vector<Cell> merged(static_cast<std::size_t>(new_width * new_height));
        auto put = [&](int px, int py, Cell cell) {
            if (px >= 0 && py >= 0 && px < new_width && py < new_height) {
                merged[static_cast<std::size_t>(py * new_width + px)] = std::move(cell);
            }
        };
        for (int row = 0; row < cache_height_; ++row) {
            for (int col = 0; col < cache_width_; ++col) {
                put(col, row, cache_cells_[static_cast<std::size_t>(row * cache_width_ + col)]);
            }
        }
        for (int row = 0; row < height; ++row) {
            for (int col = 0; col < width; ++col) {
                put(col, cache_height_ + gap + row, to_cache(cells[static_cast<std::size_t>(row * width + col)]));
            }
        }
        cache_cells_ = std::move(merged);
        cache_width_ = new_width;
        cache_height_ = new_height;
        y = new_height;
        (void)y;
    }
}

Size BigText::preferred_size() const {
    rebuild();
    return {std::max(1, cache_width_), std::max(1, cache_height_)};
}

void BigText::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }

    paint_bounds_background(ctx, style_);
    rebuild();
    if (cache_width_ <= 0 || cache_height_ <= 0) {
        return;
    }

    static const Color kRainbow[] = {
        Color::Red, Color::Yellow, Color::Green, Color::Cyan, Color::Blue, Color::Magenta,
    };

    const int max_x = std::min(cache_width_, bounds_.width);
    const int max_y = std::min(cache_height_, bounds_.height);
    for (int y = 0; y < max_y; ++y) {
        for (int x = 0; x < max_x; ++x) {
            const Cell& cell = cache_cells_[static_cast<std::size_t>(y * cache_width_ + x)];
            if (cell.glyph.empty() || cell.glyph == " ") {
                continue;
            }
            Style style = style_;
            if (!cell.shadow && !options_.gradient_stops.empty()) {
                float t = 0.0f;
                if (options_.gradient_axis == BigTextGradientAxis::Horizontal) {
                    t = cache_width_ > 1 ? static_cast<float>(x) / static_cast<float>(cache_width_ - 1) : 0.0f;
                } else {
                    t = cache_height_ > 1 ? static_cast<float>(y) / static_cast<float>(cache_height_ - 1) : 0.0f;
                }
                style = style_from_gradient(style, sample_big_text_gradient(options_.gradient_stops, t));
            } else if (options_.rainbow) {
                if (!options_.palette.empty()) {
                    style = options_.palette[static_cast<std::size_t>(cell.letter) % options_.palette.size()];
                } else {
                    style.foreground = kRainbow[static_cast<std::size_t>(cell.letter) % 6];
                    style.bold = true;
                }
            }
            if (cell.shadow) {
                if (options_.shadow_style) {
                    style = *options_.shadow_style;
                } else {
                    style.dim = true;
                }
            }
            canvas.draw_text({x, y}, cell.glyph, style);
        }
    }
}

} // namespace tuinator
