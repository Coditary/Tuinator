#include <tuinator/scene/handler_registry.hpp>

#include <tuinator/core/application.hpp>

#include <stdexcept>

namespace tuinator::scene {

void HandlerRegistry::register_handler(std::string name, Handler handler) {
    handlers_[std::move(name)] = std::move(handler);
}

const HandlerRegistry::Handler* HandlerRegistry::find(const std::string& name) const {
    const auto it = handlers_.find(name);
    return it == handlers_.end() ? nullptr : &it->second;
}

void HandlerRegistry::invoke(const std::string& name, Application& app) const {
    const Handler* handler = find(name);
    if (handler == nullptr) {
        throw std::runtime_error("Scene handler not found: " + name);
    }
    (*handler)(app);
}

} // namespace tuinator::scene
