#pragma once

#include <tuinator/render/asciidoc.hpp>
#include <tuinator/widgets/display/markdown_view.hpp>

#include <string>
#include <string_view>

namespace tuinator {

/// MarkdownView preconfigured with the AsciiDoc parser. Same streaming API
/// (append/set_markdown/clear), same styles, same scroll behavior.
class AsciiDocView : public MarkdownView {
  public:
    explicit AsciiDocView(MarkdownViewOptions options = {}) : MarkdownView(with_asciidoc(std::move(options))) {}
    AsciiDocView(std::string asciidoc, MarkdownViewOptions options = {})
        : MarkdownView(std::move(asciidoc), with_asciidoc(std::move(options))) {}

    std::string_view widget_type_name() const override { return "AsciiDocView"; }

  private:
    static MarkdownViewOptions with_asciidoc(MarkdownViewOptions options) {
        options.parser = parse_asciidoc;
        return options;
    }
};

} // namespace tuinator
