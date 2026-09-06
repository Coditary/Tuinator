#include <tuinator/tuinator.hpp>

#include <memory>
#include <string>
#include <variant>

namespace {

tuinator::GutterCell marked_gutter(const tuinator::GutterLine& line) {
    tuinator::GutterCell cell = tuinator::hybrid_gutter(line);
    char mark = ' ';
    if (line.current) {
        mark = '>';
    } else if (!line.text.empty() && line.text[0] == '#') {
        mark = '#';
    } else if (!line.text.empty() && line.text[0] == '>') {
        mark = '~';
    }
    cell.text = std::string(1, mark) + cell.text;
    return cell;
}

class EditorRoot : public tuinator::VBox {
  public:
    explicit EditorRoot(tuinator::BoxOptions options) : tuinator::VBox(options) {}

    bool wants_full_screen() const override { return true; }

    void set_editor(tuinator::TextArea* editor) { editor_ = editor; }
    void set_hint(tuinator::Label* hint) { hint_ = hint; }

    bool handle_event(const tuinator::Event& event) override {
        if (const auto* key = std::get_if<tuinator::KeyPress>(&event)) {
            const bool tab = key->key == tuinator::Key::Tab || key->character == '\t';
            if (tab && editor_ != nullptr) {
                cycle_gutter();
                return true;
            }
        }
        return tuinator::VBox::handle_event(event);
    }

  private:
    enum class GutterMode {
        Hidden,
        Absolute,
        Relative,
        Hybrid,
    };

    void cycle_gutter() {
        switch (mode_) {
        case GutterMode::Hidden: mode_ = GutterMode::Absolute; break;
        case GutterMode::Absolute: mode_ = GutterMode::Relative; break;
        case GutterMode::Relative: mode_ = GutterMode::Hybrid; break;
        case GutterMode::Hybrid: mode_ = GutterMode::Hidden; break;
        }

        editor_->set_gutter_width(0);
        switch (mode_) {
        case GutterMode::Hidden:
            editor_->set_line_numbers(false);
            set_hint_text("gutter off");
            break;
        case GutterMode::Absolute:
            editor_->set_line_numbers(true);
            editor_->set_gutter_renderer(tuinator::absolute_gutter);
            set_hint_text("absolute line numbers");
            break;
        case GutterMode::Relative:
            editor_->set_line_numbers(true);
            editor_->set_gutter_renderer(tuinator::relative_gutter);
            set_hint_text("relative line numbers");
            break;
        case GutterMode::Hybrid:
            editor_->set_line_numbers(true);
            editor_->set_gutter_renderer(marked_gutter);
            editor_->set_gutter_width(5);
            set_hint_text("hybrid numbers + symbols");
            break;
        }
    }

    void set_hint_text(const std::string& gutter) {
        if (hint_ == nullptr) {
            return;
        }
        hint_->set_text("Esc/Ctrl+Q quit  |  Tab gutter (" + gutter +
                        ")  |  arrows/wheel scroll  |  click to place cursor");
    }

    tuinator::TextArea* editor_ = nullptr;
    tuinator::Label* hint_ = nullptr;
    GutterMode mode_ = GutterMode::Absolute;
};

} // namespace

int main() {
    tuinator::Application app;
    const tuinator::Theme& theme = app.theme();

    auto root = std::make_unique<EditorRoot>(tuinator::BoxOptions{.gap = 1, .padding = 1});
    root->add_child(std::make_unique<tuinator::Label>("Tuinator textarea", theme.heading));

    tuinator::TextAreaOptions options;
    options.min_width = 48;
    options.min_height = 12;
    options.line_numbers = true;
    options.status_bar = true;
    options.title = "README.md";
    options.placeholder = "Start typing...";
    options.gutter = tuinator::absolute_gutter;

    auto editor = std::make_unique<tuinator::TextArea>(options, theme.text_input, theme.text_input_focused);
    editor->set_flex(1);
    editor->set_value("# tuinator-textarea\n"
                      "\n"
                      "A multi-line editor widget with a replaceable gutter.\n"
                      "\n"
                      "> Type to edit. Arrows, Home/End, PgUp/PgDn and mouse wheel scroll.\n"
                      "> Click to move the cursor. Enter inserts a new line.\n"
                      "\n"
                      "Tab cycles the sidebar: absolute, relative, symbols, off.\n"
                      "Esc or Ctrl+Q quits.");

    auto hint = std::make_unique<tuinator::Label>(
        "Esc/Ctrl+Q quit  |  Tab gutter (absolute line numbers)  |  arrows/wheel scroll  |  click to place cursor",
        theme.muted);

    root->set_editor(editor.get());
    root->set_hint(hint.get());
    root->add_child(std::move(editor));
    root->add_child(std::move(hint));

    app.set_root(std::move(root));
    return app.run();
}
