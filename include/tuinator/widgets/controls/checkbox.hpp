#pragma once

#include <tuinator/render/style.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/capabilities/widget_roles.hpp>
#include <tuinator/widgets/widget.hpp>

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

struct CheckboxGlyphs {
    std::string unchecked = "[ ]";
    std::string checked = "[X]";
    std::string gap = " ";
};

struct CheckboxStyle {
    const char* id = "";
    const char* title = "";
    CheckboxGlyphs glyphs;
    std::optional<Style> marker_style;
    std::optional<Style> marker_checked_style;
    std::optional<Style> label_style;
    std::vector<std::string> aliases;
};

struct CheckboxOptions {
    CheckboxGlyphs glyphs;
    Style marker_style{};
    Style marker_checked_style{};
    Style label_style{};
    Style label_checked_style{};
    Style focused_style{};
    bool highlight_row_on_focus = true;
};

const std::vector<CheckboxStyle>& all_checkbox_styles();
const CheckboxStyle* checkbox_style_named(std::string_view name);
CheckboxOptions checkbox_options_default(const Theme& theme);
void apply_checkbox_style(CheckboxOptions& options, const CheckboxStyle& style, const Theme& theme);

class Checkbox : public Widget, public ToggleControl {
  public:
    Checkbox(std::string label, bool checked = false, CheckboxOptions options = {},
             std::function<void(bool)> on_change = {});

    Checkbox(std::string label, bool checked, std::string_view style_name, std::function<void(bool)> on_change = {},
             const Theme& theme = dark_theme());

    const std::string& label() const { return label_; }
    bool checked() const { return checked_; }
    bool is_checked() const override { return checked_; }
    const CheckboxOptions& options() const { return options_; }

    void set_label(std::string label);
    void set_checked(bool checked) override;
    void set_options(CheckboxOptions options);
    void set_glyphs(CheckboxGlyphs glyphs);

    std::string_view widget_type_name() const override { return "Checkbox"; }
    bool wants_hover() const override { return true; }

    Size preferred_size() const override;
    void paint(PaintContext& ctx) const override;
    bool handle_event(const Event& event) override;
    bool is_focusable() const override { return true; }

  private:
    void toggle();

    std::string label_;
    bool checked_;
    CheckboxOptions options_;
    std::function<void(bool)> on_change_;
};

} // namespace tuinator
