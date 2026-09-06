#pragma once

#include <tuinator/core/action_registry.hpp>
#include <tuinator/render/theme.hpp>
#include <tuinator/scene/handler_registry.hpp>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace tuinator {

class Application;
class Desktop;
class Widget;
class WindowHost;

namespace scene {

class HandlerRegistry;

namespace detail {

struct WidgetEntry {
    std::string type;
    Widget* ptr = nullptr;
};

class SceneContext {
  public:
    SceneContext(Application& app, Theme theme, scene::HandlerRegistry* handlers);

    Application& app;
    Theme theme;
    scene::HandlerRegistry* handlers;
    std::shared_ptr<ActionRegistry> actions;
    Desktop* desktop = nullptr;
    WindowHost* window_host = nullptr;

    void register_widget(const std::string& id, const std::string& type, Widget* ptr);
    WidgetEntry* find_widget(const std::string& id);
    const WidgetEntry* find_widget(const std::string& id) const;

    void invoke_handler(const std::string& name);
    void schedule_post_init(std::function<void()> fn);
    void run_post_init();

    const std::unordered_map<std::string, WidgetEntry>& widgets() const { return widgets_; }

  private:
    std::unordered_map<std::string, WidgetEntry> widgets_;
    std::vector<std::function<void()>> post_init_;
};

} // namespace detail
} // namespace scene
} // namespace tuinator
