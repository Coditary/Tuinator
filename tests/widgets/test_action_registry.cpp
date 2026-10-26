#include <tuinator/core/action_registry.hpp>
#include <tuinator/core/event.hpp>

#include "test_harness.hpp"

TUINATOR_TEST(action_registry_matches_ctrl_shortcut) {
    tuinator::ActionRegistry registry;
    bool fired = false;
    tuinator::RegisteredAction action;
    action.id = "save";
    action.shortcut = "Ctrl+S";
    action.callback = [&fired]() { fired = true; };
    registry.register_action(std::move(action));

    tuinator::KeyPress key{};
    key.ctrl = true;
    key.character = 's';
    TUINATOR_CHECK(registry.handle_key(key));
    TUINATOR_CHECK(fired);
}

TUINATOR_TEST(action_registry_ignores_unregistered_key) {
    tuinator::ActionRegistry registry;

    tuinator::KeyPress key{};
    key.ctrl = true;
    key.character = 'z';
    TUINATOR_CHECK(!registry.handle_key(key));
}
