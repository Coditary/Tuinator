#include <tuinator/core/action_registry.hpp>

#include <tuinator/widgets/menu/menu_bar.hpp>

#include <algorithm>
#include <cctype>

namespace tuinator {

namespace {

char normalize_key(char ch) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
}

} // namespace

void ActionRegistry::register_action(RegisteredAction action) {
    actions_.push_back(std::move(action));
}

void ActionRegistry::clear() {
    actions_.clear();
}

const RegisteredAction* ActionRegistry::find(std::string_view id) const {
    for (const RegisteredAction& action : actions_) {
        if (action.id == id) {
            return &action;
        }
    }
    return nullptr;
}

std::vector<const RegisteredAction*> ActionRegistry::all() const {
    std::vector<const RegisteredAction*> result;
    result.reserve(actions_.size());
    for (const RegisteredAction& action : actions_) {
        result.push_back(&action);
    }
    return result;
}

std::vector<const RegisteredAction*> ActionRegistry::in_category(std::string_view category) const {
    std::vector<const RegisteredAction*> result;
    for (const RegisteredAction& action : actions_) {
        if (action.category == category) {
            result.push_back(&action);
        }
    }
    return result;
}

std::vector<std::string> ActionRegistry::categories() const {
    std::vector<std::string> result;
    for (const RegisteredAction& action : actions_) {
        if (std::find(result.begin(), result.end(), action.category) == result.end()) {
            result.push_back(action.category);
        }
    }
    return result;
}

std::vector<MenuItem> ActionRegistry::menu_for_category(std::string_view category) const {
    std::vector<MenuItem> items;
    for (const RegisteredAction& action : actions_) {
        if (action.category != category) {
            continue;
        }

        MenuItem item;
        item.label = action.label;
        item.shortcut = action.shortcut;
        item.hint = action.hint;
        item.icon = action.icon;
        item.enabled = action.enabled;
        item.action = action.callback;
        items.push_back(std::move(item));
    }
    return items;
}

bool ActionRegistry::match_shortcut(const std::string& shortcut, const KeyPress& key) const {
    if (shortcut.empty()) {
        return false;
    }

    bool wants_ctrl = false;
    bool wants_alt = false;
    char target = '\0';

    std::string token;
    for (std::size_t i = 0; i <= shortcut.size(); ++i) {
        if (i == shortcut.size() || shortcut[i] == '+') {
            if (token.empty()) {
                continue;
            }

            if (token == "Ctrl" || token == "Control") {
                wants_ctrl = true;
            } else if (token == "Alt") {
                wants_alt = true;
            } else if (token.size() == 1) {
                target = normalize_key(token[0]);
            } else if (token == "Enter") {
                return key.key == Key::Enter && !wants_ctrl && !wants_alt;
            } else if (token == "Esc" || token == "Escape") {
                return key.key == Key::Escape && !wants_ctrl && !wants_alt;
            }
            token.clear();
            continue;
        }
        token.push_back(shortcut[i]);
    }

    if (target == '\0') {
        return false;
    }

    if (wants_ctrl != key.ctrl || wants_alt != key.alt) {
        return false;
    }

    if (key.character != '\0') {
        return normalize_key(key.character) == target;
    }

    return false;
}

bool ActionRegistry::handle_key(const KeyPress& key) const {
    for (const RegisteredAction& action : actions_) {
        if (!action.enabled || !action.callback) {
            continue;
        }
        if (match_shortcut(action.shortcut, key)) {
            action.callback();
            return true;
        }
    }
    return false;
}

} // namespace tuinator
