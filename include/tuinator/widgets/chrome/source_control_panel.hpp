#pragma once

#include <tuinator/render/git_change_status.hpp>
#include <tuinator/render/style.hpp>
#include <tuinator/widgets/widget.hpp>

#include <functional>
#include <string>
#include <vector>

namespace tuinator {

struct SourceControlEntry {
    std::string path;
    GitChangeStatus status = GitChangeStatus::Modified;
    std::string additions;
    std::string deletions;
    std::string directory_tag;
    bool selected = false;
};

struct SourceControlSection {
    std::string title;
    std::vector<SourceControlEntry> entries;
    bool expanded = true;
};

struct SourceControlPanelStyle {
    Style background{};
    Style header{};
    Style section_title{};
    Style entry_text{};
    Style muted{};
    Style footer{};
    Rgb panel_bg{0x16, 0x16, 0x1e};
    Rgb text_fg{0xc0, 0xca, 0xf5};
    Rgb muted_fg{0x56, 0x5f, 0x89};
    Rgb selection_bg{0x3b, 0x42, 0x61};
    Rgb status_modified{0xe0, 0xaf, 0x68};
    Rgb status_added{0x9e, 0xce, 0x6a};
    Rgb status_deleted{0xf7, 0x76, 0x8e};
    int preferred_width = 48;
    std::string footer_text = "DIFFVIEWFILES";
    bool show_footer = true;
    int name_max_columns = 0; // 0 = use all available space
    int tag_max_columns = 0;  // 0 = hide directory tags
};

class SourceControlPanel : public Widget {
  public:
    explicit SourceControlPanel(SourceControlPanelStyle style = {});

    void set_header(std::string title, std::string subtitle);
    void set_sections(std::vector<SourceControlSection> sections);
    const std::vector<SourceControlSection>& sections() const { return sections_; }

    void set_selected(int section_index, int entry_index);
    std::pair<int, int> selected() const { return {selected_section_, selected_entry_}; }

    void set_on_select(std::function<void(int section_index, int entry_index, const SourceControlEntry&)> callback);

    Size preferred_size() const override;
    void layout(Rect bounds) override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return true; }

  private:
    struct HitTarget {
        int section = -1;
        int entry = -1;
        bool is_section_header = false;
    };

    int content_height() const;
    void clamp_scroll();
    void ensure_selection_visible();
    HitTarget hit_target_at(Point local) const;
    void select_entry(int section_index, int entry_index);

    SourceControlPanelStyle style_;
    std::string header_title_ = "Source Control";
    std::string header_subtitle_;
    std::vector<SourceControlSection> sections_;
    int selected_section_ = 0;
    int selected_entry_ = 0;
    int scroll_y_ = 0;
    std::function<void(int, int, const SourceControlEntry&)> on_select_;
};

} // namespace tuinator
