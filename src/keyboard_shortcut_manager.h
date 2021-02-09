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
        std::string& getDisplayName();
        KeyboardShortcut(int actionId, int key, uint8_t modifiers, Event event);
    private:
        std::string displayName;
    };

    void initialize();
    void shortcutPressed(int key, int scancode, int keyAction, int modifiers);
    std::vector<KeyboardShortcut>& getAllShortcuts();
    void catchNextShortcut();
    std::optional<KeyboardShortcut>& getCaughtShortcut();
    void clearCaughtShortcut();
}
#endif //BRICKSIM_KEYBOARD_SHORTCUT_MANAGER_H
