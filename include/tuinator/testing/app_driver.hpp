#pragma once

#include <tuinator/core/application.hpp>
#include <tuinator/core/event.hpp>
#include <tuinator/testing/cell_styles.hpp>
#include <tuinator/testing/terminal_snapshot.hpp>
#include <tuinator/testing/ui_tree.hpp>
#include <tuinator/testing/widget_query.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace tuinator::testing {

/// Headless application harness for integration tests and agent protocols.
class AppDriver {
  public:
    AppDriver(Size terminal_size, std::unique_ptr<Widget> root);

    Application& app() { return app_; }
    const Application& app() const { return app_; }
    MemoryTerminalBackend& backend() { return *memory_; }
    const MemoryTerminalBackend& backend() const { return *memory_; }
    Widget& root() { return *app_.root(); }
    const Widget& root() const { return *app_.root(); }

    void present();
    void step_frame();

    Widget& require(std::string_view selector);
    void focus(std::string_view selector);
    void click(std::string_view selector);
    void activate(std::string_view selector);
    void press_key(Key key, char character = '\0');
    void type_text(std::string_view text);
    void type_into(std::string_view selector, std::string_view text);
    void hover(std::string_view selector);
    void hover_at(Point position);
    void resize_terminal(Size size);
    void wheel_at(Point position, bool down = true);

    RegionStyleAnalysis analyze_selector(std::string_view selector) const;
    std::string selector_styles_json(std::string_view selector) const;

    std::string snapshot_text() const;
    std::string ui_tree_json(UiTreeOptions options = {}) const;
    std::string snapshot_ansi() const;
    bool write_snapshot_png(const std::filesystem::path& path, TerminalPngOptions options = {}) const;

  private:
    MemoryTerminalBackend* memory_ = nullptr;
    Application app_;
};

} // namespace tuinator::testing
