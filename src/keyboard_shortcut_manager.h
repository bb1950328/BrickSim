#ifndef BRICKSIM_KEYBOARD_SHORTCUT_MANAGER_H
#define BRICKSIM_KEYBOARD_SHORTCUT_MANAGER_H
#include <vector>
#include <string>
//#include <GLFW/glfw3.h>

namespace keyboard_shortcut_manager {
    enum class Event {
        ON_PRESS = 1/*GLFW_PRESS*/,
        ON_REPEAT = 2/*GLFW_REPEAT*/,
        ON_RELEASE = 0/*GLFW_RELEASE*/,
    };

    class KeyboardShortcut {
    public:
        int actionId;
        int key;
        uint8_t modifiers;
        Event event;
        std::string getDisplayName();
        KeyboardShortcut(int actionId, int key, uint8_t modifiers, Event event);
    };

    void initialize();
    void shortcutPressed(int key, int keyAction, int modifiers, bool isCapturedByGui);
    std::vector<KeyboardShortcut>& getAllShortcuts();
    void replaceAllShortcuts(std::vector<KeyboardShortcut>& newShortcuts);
    const std::string & getShortcutForAction(int actionId);

    void setCatchNextShortcut(bool doCatch);
    std::optional<KeyboardShortcut>& getCaughtShortcut();
    void clearCaughtShortcut();
}
#endif //BRICKSIM_KEYBOARD_SHORTCUT_MANAGER_H
