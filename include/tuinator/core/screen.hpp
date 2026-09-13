#pragma once

#include <tuinator/core/application.hpp>
#include <tuinator/layout/box.hpp>

#include <memory>

namespace tuinator {

/// Full-screen root container. Prefer this over a bare widget as the app root.
class Screen : public VBox {
  public:
    explicit Screen(BoxOptions options = {});

    bool wants_full_screen() const override { return true; }
};

/// Create a full-screen vertical layout root.
inline std::unique_ptr<Screen> make_screen(BoxOptions options = {}) { return std::make_unique<Screen>(options); }

/// Attach a screen root and run the application event loop.
int run_screen(Application& app, std::unique_ptr<Screen> root);

} // namespace tuinator
