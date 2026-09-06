#include <tuinator/render/text.hpp>
#include <tuinator/widgets/display/throbber.hpp>

#include <algorithm>
#include <string>

namespace tuinator {

namespace {

ThrobberSet make_set(const char* id, const char* title, int interval_ms, std::initializer_list<const char*> frames,
                     std::initializer_list<const char*> aliases = {}) {
    ThrobberSet set;
    set.id = id;
    set.title = title;
    set.interval_ms = interval_ms;
    set.frames.reserve(frames.size());
    for (const char* frame : frames) {
        set.frames.emplace_back(frame);
    }
    set.aliases.reserve(aliases.size());
    for (const char* alias : aliases) {
        set.aliases.emplace_back(alias);
    }
    return set;
}

const ThrobberSet kFallback = make_set("line", "Line", 80, {"|", "/", "-", "\\"}, {"ascii"});

bool set_matches(const ThrobberSet& set, std::string_view name) {
    if (name == set.id || name == set.title) {
        return true;
    }
    for (const std::string& alias : set.aliases) {
        if (name == alias) {
            return true;
        }
    }
    return false;
}

} // namespace

const std::vector<ThrobberSet>& all_throbber_sets() {
    static const std::vector<ThrobberSet> sets = {
        make_set("line", "Line", 80, {"|", "/", "-", "\\"}, {"ascii"}),
        make_set("braille", "Braille", 80, {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"},
                 {"dots", "box_drawing", "braille_six"}),
        make_set("braille-heavy", "Braille heavy", 80, {"⣾", "⣽", "⣻", "⢿", "⡿", "⣟", "⣯", "⣷"},
                 {"dots2", "braille_six_double"}),
        make_set("braille-twist", "Braille twist", 80, {"⠋", "⠙", "⠚", "⠞", "⠖", "⠦", "⠴", "⠲", "⠳", "⠓"}, {"dots3"}),
        make_set("braille-wave", "Braille wave", 80,
                 {"⠄", "⠆", "⠇", "⠋", "⠙", "⠸", "⠰", "⠠", "⠰", "⠸", "⠙", "⠋", "⠇", "⠆"}, {"dots4"}),
        make_set("braille-orbit", "Braille orbit", 80, {"⡀", "⠄", "⠂", "⠁", "⠈", "⠐", "⠠", "⢀"},
                 {"dots8", "braille_double", "dots11", "dots12"}),
        make_set("braille-bounce", "Braille bounce", 80, {"⢹", "⢺", "⢼", "⣸", "⣇", "⡧", "⡗", "⡏"}, {"dots9"}),
        make_set("braille-rise", "Braille rise", 80, {"⢄", "⢂", "⢁", "⡁", "⡈", "⡐", "⡠"}, {"dots10"}),
        make_set("braille-fill", "Braille fill", 80, {"⡀", "⡁", "⡂", "⡃", "⡄", "⡅", "⡆", "⡇"}, {"braille_one"}),
        make_set("braille-stack", "Braille stack", 80, {"⢀", "⢠", "⢰", "⢸", "⢹", "⢻", "⢿", "⣿"}, {"braille_eight"}),
        make_set("braille-build", "Braille build", 80, {"⠁", "⠉", "⠋", "⠛", "⠟", "⠿", "⡿", "⣿"},
                 {"braille_eight_double"}),
        make_set("braille-binary", "Braille binary", 80,
                 {"⠀", "⠁", "⠂", "⠃", "⠄", "⠅", "⠆", "⠇", "⡀", "⡁", "⡂", "⡃", "⡄", "⡅", "⡆", "⡇"}, {"dots8Bit"}),
        make_set("arrows", "Arrows", 100, {"←", "↖", "↑", "↗", "→", "↘", "↓", "↙"}, {"arrow"}),
        make_set("arrows-double", "Double arrows", 120, {"⇐", "⇖", "⇑", "⇗", "⇒", "⇘", "⇓", "⇙"}, {"double_arrow"}),
        make_set("chevrons", "Chevrons", 80, {"▹▹▹▹▹", "▸▹▹▹▹", "▹▸▹▹▹", "▹▹▸▹▹", "▹▹▹▸▹", "▹▹▹▹▸"}, {"arrow2"}),
        make_set("chevrons-pulse", "Chevrons pulse", 80,
                 {"▹▹▹▹▹", "▸▹▹▹▹", "▹▸▹▹▹", "▹▹▸▹▹", "▹▹▹▸▹", "▹▹▹▹▸", "▪▪▪▪▪"}, {"arrow3"}),
        make_set("grow-vertical", "Grow vertical", 80, {"▁", "▃", "▄", "▅", "▆", "▇", "█", "▇", "▆", "▅", "▄", "▃"},
                 {"vertical_block", "growVertical"}),
        make_set("grow-horizontal", "Grow horizontal", 80,
                 {"▏", "▎", "▍", "▌", "▋", "▊", "▉", "█", "▉", "▊", "▋", "▌", "▍", "▎"},
                 {"horizontal_block", "growHorizontal"}),
        make_set("quadrants", "Quadrants", 80, {"▖", "▘", "▝", "▗"}, {"quadrant_block", "boxBounce"}),
        make_set("box-bounce", "Box bounce", 80, {"▌", "▀", "▐", "▄"}, {"quadrant_block_crack"}),
        make_set("circle-halves", "Circle halves", 80, {"◐", "◓", "◑", "◒"}, {"white_circle", "circleHalves"}),
        make_set("arc", "Arc", 80, {"◜", "◠", "◝", "◞", "◡", "◟"}, {"black_circle"}),
        make_set("circle", "Circle", 120, {"◡", "⊙", "◠"}),
        make_set("square-corners", "Square corners", 80, {"◰", "◳", "◲", "◱"}, {"white_square", "squareCorners"}),
        make_set("triangle", "Triangle", 50, {"◢", "◣", "◤", "◥"}),
        make_set("clock", "Clock", 100, {"🕛", "🕐", "🕑", "🕒", "🕓", "🕔", "🕕", "🕖", "🕗", "🕘", "🕙", "🕚"}),
        make_set("moon", "Moon", 80, {"🌑", "🌒", "🌓", "🌔", "🌕", "🌖", "🌗", "🌘"}),
        make_set("earth", "Earth", 180, {"🌍", "🌎", "🌏"}),
        make_set("weather", "Weather", 100, {"☀️", "☀️", "☀️", "🌤", "⛅️", "🌥", "☁️", "🌧"}),
        make_set("pipe", "Pipe", 80, {"┤", "┘", "┴", "└", "├", "┌", "┬", "┐"}),
        make_set("ellipsis", "Ellipsis", 400, {".  ", ".. ", "...", "   "}, {"simpleDots"}),
        make_set("star", "Star", 80, {"✶", "✸", "✹", "✺", "✹", "✷"}),
        make_set("star-ascii", "Star ASCII", 80, {"+", "x", "*"}, {"star2"}),
        make_set("flip", "Flip", 70, {"_", "_", "_", "-", "`", "`", "'", "´", "-", "_", "_", "_"}),
        make_set("blocks", "Blocks", 80,
                 {"▰▱▱▱▱▱▱", "▰▰▱▱▱▱▱", "▰▰▰▱▱▱▱", "▰▰▰▰▱▱▱", "▰▰▰▰▰▱▱", "▰▰▰▰▰▰▱", "▰▰▰▰▰▰▰", "▱▰▰▰▰▰▰", "▱▱▰▰▰▰▰",
                  "▱▱▱▰▰▰▰", "▱▱▱▱▰▰▰", "▱▱▱▱▱▰▰", "▱▱▱▱▱▱▰", "▱▱▱▱▱▱▱"},
                 {"aesthetic"}),
        make_set("bouncing-bar", "Bouncing bar", 80,
                 {"[    ]", "[=   ]", "[==  ]", "[=== ]", "[ ===]", "[  ==]", "[   =]", "[    ]", "[   =]", "[  ==]",
                  "[ ===]", "[=== ]", "[==  ]", "[=   ]"},
                 {"bouncingBar"}),
        make_set("bouncing-ball", "Bouncing ball", 80,
                 {"( ●    )", "(  ●   )", "(   ●  )", "(    ● )", "(     ●)", "(    ● )", "(   ●  )", "(  ●   )",
                  "( ●    )", "(●     )"},
                 {"bouncingBall"}),
        make_set("sliding-bar", "Sliding bar", 80,
                 {"█▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁", "██▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁", "███▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁", "████▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁",
                  "█████▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁", "▁█████▁▁▁▁▁▁▁▁▁▁▁▁▁▁", "▁▁█████▁▁▁▁▁▁▁▁▁▁▁▁▁", "▁▁▁█████▁▁▁▁▁▁▁▁▁▁▁▁",
                  "▁▁▁▁█████▁▁▁▁▁▁▁▁▁▁▁", "▁▁▁▁▁█████▁▁▁▁▁▁▁▁▁▁", "▁▁▁▁▁▁█████▁▁▁▁▁▁▁▁▁", "▁▁▁▁▁▁▁█████▁▁▁▁▁▁▁▁",
                  "▁▁▁▁▁▁▁▁█████▁▁▁▁▁▁▁", "▁▁▁▁▁▁▁▁▁█████▁▁▁▁▁▁", "▁▁▁▁▁▁▁▁▁▁█████▁▁▁▁▁", "▁▁▁▁▁▁▁▁▁▁▁█████▁▁▁▁",
                  "▁▁▁▁▁▁▁▁▁▁▁▁█████▁▁▁", "▁▁▁▁▁▁▁▁▁▁▁▁▁█████▁▁", "▁▁▁▁▁▁▁▁▁▁▁▁▁▁█████▁", "▁▁▁▁▁▁▁▁▁▁▁▁▁▁▁█████"},
                 {"material"}),
        make_set("toggle", "Toggle", 250, {"⊶", "⊷"}),
        make_set("toggle-square", "Toggle square", 80, {"▫", "▪"}, {"toggle2"}),
        make_set("toggle-box", "Toggle box", 120, {"□", "■"}, {"toggle3"}),
        make_set("toggle-squares", "Toggle squares", 100, {"■", "□", "▪", "▫"}, {"toggle4"}),
        make_set("toggle-bar", "Toggle bar", 100, {"▮", "▯"}, {"toggle5"}),
        make_set("toggle-circle-mm", "Toggle circles", 300, {"ဝ", "၀"}, {"toggle6"}),
        make_set("toggle-dot", "Toggle dot", 80, {"⦾", "⦿"}, {"toggle7"}),
        make_set("toggle-ring", "Toggle ring", 100, {"◍", "◌"}, {"toggle8"}),
        make_set("toggle-target", "Toggle target", 100, {"◉", "◎"}, {"toggle9", "toggle10"}),
        make_set("toggle-pulse", "Toggle pulse", 50, {"◌", "○", "◎", "●", "◎", "○"}, {"toggle11"}),
        make_set("toggle-frame", "Toggle frame", 120, {"▢", "▣"}, {"toggle12"}),
        make_set("toggle-bold", "Toggle bold", 80, {"○", "⦿"}, {"toggle13"}),
        make_set("christmas", "Christmas", 400, {"🌲", "🎄"}),
        make_set("hearts", "Hearts", 100, {"💛", "💙", "💜", "💚", "❤️"}),
        make_set("smiley", "Smiley", 200, {"😄", "😝"}),
        make_set("monkey", "Monkey", 300, {"🙈", "🙈", "🙉", "🙊"}),
        make_set("trigrams", "Trigrams", 100, {"☱", "☲", "☴"}, {"hamburger"}),
        make_set("runner", "Runner", 140, {"🚶", "🏃"}),
        make_set("letters", "Letters", 100, {"d", "q", "p", "b"}, {"dqpb"}),
        make_set("points", "Points", 125, {"∙∙∙", "●∙∙", "∙●∙", "∙∙●", "∙∙∙"}, {"point"}),
        make_set("layers", "Layers", 150, {"-", "=", "≡"}, {"layer"}),
        make_set("balloon", "Balloon", 140, {".", "o", "O", "@", "*", " "}),
        make_set("noise", "Noise", 100, {"▓", "▒", "░"}),
        make_set("ogham", "Ogham", 80, {" ", "ᚁ", "ᚂ", "ᚃ", "ᚄ", "ᚅ"}, {"ogham_a"}),
        make_set("ogham-fork", "Ogham fork", 120, {"ᚆ", "ᚇ", "ᚈ", "ᚉ", "ᚊ"}, {"ogham_b"}),
        make_set("ogham-mesh", "Ogham mesh", 120, {"ᚋ", "ᚌ", "ᚍ", "ᚎ", "ᚏ"}, {"ogham_c"}),
        make_set("pong", "Pong", 80,
                 {"▐⠂       ▌", "▐⠈       ▌", "▐ ⠂      ▌", "▐ ⠠      ▌", "▐  ⡀     ▌", "▐  ⠠     ▌",
                  "▐   ⠂    ▌", "▐   ⠈    ▌", "▐    ⠂   ▌", "▐    ⠠   ▌", "▐     ⡀  ▌", "▐     ⠠  ▌",
                  "▐      ⠂ ▌", "▐      ⠈ ▌", "▐       ⠂▌", "▐       ⠠▌", "▐       ⡀▌", "▐      ⠠ ▌",
                  "▐      ⠂ ▌", "▐     ⠈  ▌", "▐     ⠂  ▌", "▐    ⠠   ▌", "▐    ⡀   ▌", "▐   ⠠    ▌",
                  "▐   ⠂    ▌", "▐  ⠈     ▌", "▐  ⠂     ▌", "▐ ⠠      ▌", "▐ ⡀      ▌", "▐⠠       ▌"}),
        make_set("shark", "Shark", 120,
                 {"▐|\\____________▌", "▐_|\\___________▌", "▐__|\\__________▌", "▐___|\\_________▌",
                  "▐____|\\________▌", "▐_____|\\_______▌", "▐______|\\______▌", "▐_______|\\_____▌",
                  "▐________|\\____▌", "▐_________|\\___▌", "▐__________|\\__▌", "▐___________|\\_▌",
                  "▐____________|\\▌", "▐____________/|▌",  "▐___________/|_▌",  "▐__________/|__▌",
                  "▐_________/|___▌",  "▐________/|____▌",  "▐_______/|_____▌",  "▐______/|______▌",
                  "▐_____/|_______▌",  "▐____/|________▌",  "▐___/|_________▌",  "▐__/|__________▌",
                  "▐_/|___________▌",  "▐/|____________▌"}),
    };
    return sets;
}

const ThrobberSet* throbber_set_named(std::string_view name) {
    for (const ThrobberSet& set : all_throbber_sets()) {
        if (set_matches(set, name)) {
            return &set;
        }
    }
    return nullptr;
}

Throbber::Throbber(const ThrobberSet& set, Style style) : set_(set), style_(style) {
    if (set_.frames.empty()) {
        set_ = kFallback;
    }
    for (const std::string& frame : set_.frames) {
        width_ = std::max(width_, text_display_width(frame));
    }
}

Throbber::Throbber(std::string_view name, Style style)
    : Throbber(
          [&]() -> const ThrobberSet& {
              const ThrobberSet* named = throbber_set_named(name);
              return named != nullptr ? *named : kFallback;
          }(),
          style) {}

const std::string& Throbber::frame() const { return set_.frames[static_cast<std::size_t>(frame_)]; }

void Throbber::set_frame(int index) {
    if (set_.frames.empty()) {
        return;
    }
    const int next = ((index % static_cast<int>(set_.frames.size())) + static_cast<int>(set_.frames.size())) %
                     static_cast<int>(set_.frames.size());
    if (next == frame_) {
        return;
    }
    frame_ = next;
    mark_dirty();
}

void Throbber::advance() {
    if (set_.frames.size() < 2) {
        return;
    }
    frame_ = (frame_ + 1) % static_cast<int>(set_.frames.size());
    mark_dirty();
}

void Throbber::tick(int dt_ms) {
    if (set_.frames.size() < 2) {
        return;
    }
    if (dt_ms <= 0) {
        advance();
        return;
    }

    elapsed_ms_ += dt_ms;
    const int interval = std::max(16, set_.interval_ms);
    bool moved = false;
    while (elapsed_ms_ >= interval) {
        elapsed_ms_ -= interval;
        frame_ = (frame_ + 1) % static_cast<int>(set_.frames.size());
        moved = true;
    }
    if (moved) {
        mark_dirty();
    }
}

Size Throbber::preferred_size() const { return {std::max(1, width_), 1}; }

void Throbber::paint(PaintContext& ctx) const {
    Canvas& canvas = ctx.canvas;
    if (bounds_.width <= 0 || bounds_.height <= 0 || set_.frames.empty()) {
        return;
    }

    const int slot = std::clamp(std::min(bounds_.width, std::max(1, width_)), 1, 64);
    canvas.fill_rect({0, 0, bounds_.width, 1}, ' ', style_);

    const std::string& glyph = frame();
    const std::size_t bytes = text_byte_length_for_width(glyph, slot);
    std::string padded(glyph.substr(0, bytes));
    int used = text_display_width(padded);
    for (int guard = 0; used < slot && guard < slot + 8; ++guard) {
        padded.push_back(' ');
        ++used;
    }
    canvas.draw_text({0, 0}, padded, style_);
}

} // namespace tuinator
