#pragma once

#include "gui/windows/windows.h"
#include "magic_enum.hpp"
#include "config/data.h"

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace bricksim::user_actions {
    enum Action : uint32_t;
}

namespace bricksim::keyboard_shortcut_manager {
    using modifier_t = decltype(config::KeyboardShortcut::modifiers);
    void initialize();
    void shortcutPressed(key_t key, uint64_t keyAction, modifier_t modifiers, bool isCapturedByGui);
    //std::vector<config::KeyboardShortcut>& getAllShortcuts();
    //void replaceAllShortcuts(const std::vector<config::KeyboardShortcut>& newShortcuts);
    void resetToDefault();
    const std::string& getShortcutForAction(user_actions::Action action);
    std::string getDisplayName(const config::KeyboardShortcut& shortcut);

    void setCatchNextShortcut(bool doCatch);
    std::optional<config::KeyboardShortcut>& getCaughtShortcut();
    void clearCaughtShortcut();
}
