#pragma once

#include <tuinator/core/event.hpp>
#include <tuinator/widgets/menu/menu_bar.hpp>

#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace tuinator {

struct RegisteredAction {
    std::string id;
    std::string label;
    std::string category;
    std::string shortcut;
    std::string hint;
    std::string icon;
    std::function<void()> callback;
    bool enabled = true;
};

class ActionRegistry {
  public:
    void register_action(RegisteredAction action);
    void clear();

    const RegisteredAction* find(std::string_view id) const;
    std::vector<const RegisteredAction*> all() const;
    std::vector<const RegisteredAction*> in_category(std::string_view category) const;
    std::vector<std::string> categories() const;

    std::vector<MenuItem> menu_for_category(std::string_view category) const;
    bool handle_key(const KeyPress& key) const;

  private:
    bool match_shortcut(const std::string& shortcut, const KeyPress& key) const;

    std::vector<RegisteredAction> actions_;
};

} // namespace tuinator
