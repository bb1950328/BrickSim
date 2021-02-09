#include <map>
#include "keyboard_shortcut_manager.h"
#include "helpers/platform_detection.h"
#include "user_actions.h"
#include "latest_log_messages_tank.h"
#include "db.h"

namespace keyboard_shortcut_manager {
    namespace {
        std::map<int, const char *> MODIFIER_DISPLAY_NAMES = { // NOLINT(cert-err58-cpp)
                {GLFW_MOD_SHIFT, "Shift"},
                {GLFW_MOD_CONTROL, "Ctrl"},
                {GLFW_MOD_ALT, "Alt"},
#ifdef BRICKSIM_PLATFORM_SOME_APPLE
                {GLFW_MOD_SUPER, "Cmd"},
#elif defined(BRICKSIM_PLATFORM_WIN32_OR_64)
                {GLFW_MOD_SUPER, "Win"},
#else
                {GLFW_MOD_SUPER, "Super"},
#endif
                //{GLFW_MOD_CAPS_LOCK, "CapsLock"},
                //{GLFW_MOD_NUM_LOCK, "NumLock"}
        };
        uint8_t ALL_MODIFIERS[] = {GLFW_MOD_SHIFT, GLFW_MOD_CONTROL, GLFW_MOD_ALT, GLFW_MOD_SUPER};
        uint8_t ALL_MODIFIERS_MASK = GLFW_MOD_SHIFT | GLFW_MOD_CONTROL | GLFW_MOD_ALT | GLFW_MOD_SUPER;

        std::vector<KeyboardShortcut> shortcuts;

        bool shouldCatchNextShortcut;
        std::optional<KeyboardShortcut> caughtShortcut;
    }

    void initialize() {
        for (const auto &record : db::key_shortcuts::loadShortcuts()) {
            shortcuts.emplace_back(
                    std::get<0>(record),
                    std::get<1>(record),
                    std::get<2>(record),
                    (Event) std::get<3>(record)
            );
        }
    }

    void shortcutPressed(int key, int scancode, int keyAction, int modifiers) {
        auto event = static_cast<Event>(keyAction);
        if (shouldCatchNextShortcut) {
            shouldCatchNextShortcut = false;
            caughtShortcut = std::make_optional<KeyboardShortcut>(-1, key, modifiers, event);
            return;
        }
        for (auto &shortcut : shortcuts) {
            if (shortcut.key == key && shortcut.event == event && (shortcut.modifiers & modifiers) == shortcut.modifiers) {
                spdlog::debug("event {} matched shortcut, executing action", shortcut.getDisplayName());
                user_actions::executeAction(shortcut.actionId);
                return;
            }
        }
        spdlog::debug("event {} did not match any shortcut", KeyboardShortcut(-1, key, modifiers, event).getDisplayName());
    }

    std::vector<KeyboardShortcut> &getAllShortcuts() {
        return shortcuts;
    }

    std::string &KeyboardShortcut::getDisplayName() {
        if (displayName.empty()) {
            for (const auto &mod : ALL_MODIFIERS) {
                if (modifiers & mod) {
                    displayName += MODIFIER_DISPLAY_NAMES[mod];
                    displayName += '+';
                }
            }
            displayName += glfwGetKeyName(key, 0);
        }
        return displayName;
    }

    KeyboardShortcut::KeyboardShortcut(int actionId, int key, uint8_t modifiers, Event event) : actionId(actionId), key(key), modifiers(modifiers), event(event) {}
}