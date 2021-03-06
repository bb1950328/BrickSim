#pragma once

#include <optional>
#include <string>
#include <vector>

namespace bricksim::keyboard_shortcut_manager {
    enum class Event {
        ON_PRESS = 1 /*GLFW_PRESS*/,
        ON_REPEAT = 2 /*GLFW_REPEAT*/,
        ON_RELEASE = 0 /*GLFW_RELEASE*/,
    };

    class KeyboardShortcut {
    public:
        int actionId;
        int key;
        uint8_t modifiers;
        Event event;
        std::string getDisplayName() const;
        KeyboardShortcut(int actionId, int key, uint8_t modifiers, Event event);
    };

    void initialize();
    void shortcutPressed(int key, int keyAction, int modifiers, bool isCapturedByGui);
    std::vector<KeyboardShortcut>& getAllShortcuts();
    void replaceAllShortcuts(std::vector<KeyboardShortcut>& newShortcuts);
    const std::string& getShortcutForAction(int actionId);

    void setCatchNextShortcut(bool doCatch);
    std::optional<KeyboardShortcut>& getCaughtShortcut();
    void clearCaughtShortcut();
}
