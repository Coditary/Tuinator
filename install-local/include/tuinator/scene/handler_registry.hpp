#pragma once

#include <functional>
#include <string>
#include <unordered_map>

namespace tuinator {

class Application;

namespace scene {

/// Maps scene handler names (e.g. onClick: "quit") to C++ callbacks.
class HandlerRegistry {
public:
    using Handler = std::function<void(Application&)>;

    void register_handler(std::string name, Handler handler);
    const Handler* find(const std::string& name) const;
    void invoke(const std::string& name, Application& app) const;

private:
    std::unordered_map<std::string, Handler> handlers_;
};

} // namespace scene
} // namespace tuinator
