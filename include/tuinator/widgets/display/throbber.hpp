#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/widgets/widget.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

struct ThrobberSet {
    const char* id = "";
    const char* title = "";
    int interval_ms = 80;
    std::vector<std::string> frames;
    std::vector<std::string> aliases;
};

const std::vector<ThrobberSet>& all_throbber_sets();
const ThrobberSet* throbber_set_named(std::string_view name);

class Throbber : public Widget {
public:
    explicit Throbber(const ThrobberSet& set, Style style = {});
    explicit Throbber(std::string_view name, Style style = {});

    const char* id() const { return set_.id; }
    const char* title() const { return set_.title; }
    const char* name() const { return set_.id; }
    int interval_ms() const { return set_.interval_ms; }
    const std::string& frame() const;
    int frame_index() const { return frame_; }

    void set_frame(int index);
    void tick(int dt_ms = 0);

    Size preferred_size() const override;
    void paint(Canvas& canvas) const override;

private:
    void advance();

    ThrobberSet set_;
    Style style_;
    int frame_ = 0;
    int elapsed_ms_ = 0;
    int width_ = 1;
};

} // namespace tuinator
