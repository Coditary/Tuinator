#pragma once

#include <tuinator/render/file_icon.hpp>
#include <tuinator/render/line_icon.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/render/ui_icon.hpp>
#include <tuinator/widgets/widget.hpp>

#include <optional>
#include <string>
#include <vector>

namespace tuinator {

enum class StatusSegmentKind {
    Text,
    Pill,
    Box,
    Separator,
};

struct StatusSegment {
    StatusSegmentKind kind = StatusSegmentKind::Text;
    std::optional<FileIcon> icon;
    std::optional<UiIcon> ui_icon;
    std::optional<LineIcon> line_icon;
    std::string text;
    std::optional<Rgb> foreground_rgb;
    std::optional<Rgb> background_rgb;
    bool bold = false;
    bool dim = false;
};

struct StatusLineStyle {
    Style background{};
    std::optional<Rgb> accent_bar_rgb;
};

class StatusLine : public Widget {
  public:
    explicit StatusLine(StatusLineStyle style = {});

    void set_left(std::vector<StatusSegment> segments);
    void set_center(std::vector<StatusSegment> segments);
    void set_right(std::vector<StatusSegment> segments);

    const std::vector<StatusSegment>& left() const { return left_; }
    const std::vector<StatusSegment>& center() const { return center_; }
    const std::vector<StatusSegment>& right() const { return right_; }

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;

  private:
    int segment_width(const StatusSegment& segment) const;
    Style segment_style(const StatusSegment& segment) const;
    void paint_segment(Canvas& canvas, int x, int y, const StatusSegment& segment) const;
    int paint_segments(Canvas& canvas, int x, int y, const std::vector<StatusSegment>& segments) const;

    StatusLineStyle style_;
    std::vector<StatusSegment> left_;
    std::vector<StatusSegment> center_;
    std::vector<StatusSegment> right_;
};

} // namespace tuinator
