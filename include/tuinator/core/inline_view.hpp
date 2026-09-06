#pragma once

#include <tuinator/backend/inline_backend.hpp>
#include <tuinator/core/application.hpp>

#include <memory>

namespace tuinator {

/// Flush regular CLI output (cout/cerr/stdout/stderr) before starting an inline band.
void flush_cli_output();

/// Convenience wrapper for inline CLI rendering below the shell prompt.
///
/// Typical usage (ESLint-style live reporter):
/// @code
/// std::cout << "Running checks...\n";
///
/// InlineView view({.height = 12});
/// view.set_root(build_widget_tree());
/// view.start();  // flushes CLI output, then draws the first frame
///
/// view.app().set_interval(100, [&]() {
///     update_state();
/// });
/// view.run();
/// view.finish();
/// @endcode
class InlineView {
  public:
    explicit InlineView(InlineBackendOptions options = {});
    ~InlineView();

    InlineView(const InlineView&) = delete;
    InlineView& operator=(const InlineView&) = delete;

    Application& app() { return app_; }
    const Application& app() const { return app_; }

    void set_root(std::unique_ptr<Widget> root);
    void set_theme(Theme theme) { app_.set_theme(std::move(theme)); }

    /// Flush CLI output and render the first inline frame.
    void start();

    /// Render a frame into the inline region (flushes CLI output on the first call).
    void present();

    /// Run the event loop (timers, idle callbacks). Stdin is swallowed by default;
    /// pass {.keyboard_input = true} in InlineBackendOptions to handle key events.
    int run();

    /// Tear down the inline region (optionally clears it).
    void finish();

    Size region_size() const { return app_.terminal_size(); }
    int anchor_row() const;

  private:
    void begin_inline_output();

    Application app_;
    bool finished_ = false;
    bool output_committed_ = false;
};

} // namespace tuinator
