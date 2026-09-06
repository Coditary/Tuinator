#pragma once

#include <tuinator/backend/inline_backend.hpp>
#include <tuinator/core/application.hpp>

#include <memory>

namespace tuinator {

/// Convenience wrapper for inline CLI rendering below the shell prompt.
///
/// Typical usage (ESLint-style live reporter):
/// @code
/// std::cout << "Running checks...\n";
///
/// InlineView view({.height = 12});
/// view.set_root(build_widget_tree());
/// view.present();
///
/// view.app().set_interval(100, [&]() {
///     update_state();
///     view.present();
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

    /// Render a single frame into the inline region.
    void present();

    /// Run the event loop (timers, idle callbacks). Does not capture keyboard input.
    int run();

    /// Tear down the inline region (optionally clears it).
    void finish();

    Size region_size() const { return app_.terminal_size(); }
    int anchor_row() const;

private:
    Application app_;
    bool finished_ = false;
};

} // namespace tuinator
