#pragma once

#include "config/data.h"

#include <optional>
#include <string>

namespace bricksim::user_actions {
    enum Action : uint32_t;
}

namespace bricksim::keyboard_shortcut_manager {
    using modifier_t = decltype(config::KeyboardShortcut::modifiers);
    void initialize();
    void shortcutPressed(int key, int keyAction, modifier_t modifiers, bool isCapturedByGui);
    void resetToDefault(config::KeyboardShortcuts& cfg);
    const std::string& getShortcutForAction(user_actions::Action action);
    std::string getDisplayName(const config::KeyboardShortcut& shortcut);

    void setCatchNextShortcut(bool doCatch);
    std::optional<config::KeyboardShortcut>& getCaughtShortcut();
    void clearCaughtShortcut();
}
