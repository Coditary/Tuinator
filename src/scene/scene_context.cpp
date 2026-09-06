#include <tuinator/core/application.hpp>
#include <tuinator/scene/scene_context.hpp>
#include <tuinator/window/desktop.hpp>
#include <tuinator/window/window_host.hpp>

#include <stdexcept>

namespace tuinator::scene::detail {

SceneContext::SceneContext(Application& app, Theme theme, scene::HandlerRegistry* handlers)
    : app(app), theme(std::move(theme)), handlers(handlers) {}

void SceneContext::register_widget(const std::string& id, const std::string& type, Widget* ptr) {
    if (widgets_.count(id)) {
        throw std::runtime_error("Duplicate node id: " + id);
    }
    widgets_.emplace(id, WidgetEntry{type, ptr});
    if (type == "Desktop") {
        desktop = static_cast<Desktop*>(ptr);
    } else if (type == "WindowHost") {
        window_host = static_cast<WindowHost*>(ptr);
    }
}

WidgetEntry* SceneContext::find_widget(const std::string& id) {
    const auto it = widgets_.find(id);
    return it == widgets_.end() ? nullptr : &it->second;
}

const WidgetEntry* SceneContext::find_widget(const std::string& id) const {
    const auto it = widgets_.find(id);
    return it == widgets_.end() ? nullptr : &it->second;
}

void SceneContext::invoke_handler(const std::string& name) {
    if (handlers == nullptr) {
        throw std::runtime_error("Scene handler not registered: " + name);
    }
    handlers->invoke(name, app);
}

void SceneContext::schedule_post_init(std::function<void()> fn) { post_init_.push_back(std::move(fn)); }

void SceneContext::run_post_init() {
    for (auto& fn : post_init_) {
        fn();
    }
}

} // namespace tuinator::scene::detail
