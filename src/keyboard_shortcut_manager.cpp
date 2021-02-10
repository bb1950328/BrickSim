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
        const int ALL_MODIFIER_KEYS[] = {GLFW_KEY_LEFT_SHIFT, GLFW_KEY_RIGHT_SHIFT, GLFW_KEY_LEFT_CONTROL, GLFW_KEY_RIGHT_CONTROL, GLFW_KEY_LEFT_ALT, GLFW_KEY_RIGHT_ALT, GLFW_KEY_LEFT_SUPER, GLFW_KEY_RIGHT_SUPER};

        std::map<int, const char*> MISC_KEY_DISPLAY_NAMES = { // NOLINT(cert-err58-cpp)
                {GLFW_KEY_BACKSPACE, "Backspace"},
                {GLFW_KEY_ENTER, "Enter"},
                {GLFW_KEY_INSERT, "Ins"},
                {GLFW_KEY_DELETE, "Del"},
                {GLFW_KEY_HOME, "Home"},
                {GLFW_KEY_END, "End"},
                {GLFW_KEY_PAGE_UP, "PageUp"},
                {GLFW_KEY_PAGE_DOWN, "PageDown"},
                {GLFW_KEY_PRINT_SCREEN, "PrintScreen"},
                {GLFW_KEY_UP, "ArrowUp"},
                {GLFW_KEY_DOWN, "ArrowDown"},
                {GLFW_KEY_LEFT, "ArrowLeft"},
                {GLFW_KEY_RIGHT, "ArrowRight"},
                {GLFW_KEY_PAUSE, "Pause"},
                {GLFW_KEY_MENU, "Menu"},
                {GLFW_KEY_ESCAPE, "Esc"},
        };

        const std::vector<KeyboardShortcut> DEFAULT_SHORTCUTS = { // NOLINT(cert-err58-cpp)
                {user_actions::COPY.id, GLFW_KEY_C, (uint8_t)GLFW_MOD_CONTROL, (Event)Event::ON_PRESS},
                {user_actions::CUT.id, GLFW_KEY_X, (uint8_t)GLFW_MOD_CONTROL, (Event)Event::ON_PRESS},
                {user_actions::PASTE.id, GLFW_KEY_V, (uint8_t)GLFW_MOD_CONTROL, (Event)Event::ON_PRESS},
                {user_actions::SAVE_FILE.id, GLFW_KEY_S, (uint8_t)GLFW_MOD_CONTROL, (Event)Event::ON_PRESS},
                {user_actions::SAVE_FILE_AS.id, GLFW_KEY_S, (uint8_t)(GLFW_MOD_CONTROL|GLFW_MOD_SHIFT), (Event)Event::ON_PRESS},
                {user_actions::SELECT_ALL.id, GLFW_KEY_A, (uint8_t)GLFW_MOD_CONTROL, (Event)Event::ON_PRESS},
                {user_actions::SELECT_NOTHING.id, GLFW_KEY_A, (uint8_t)(GLFW_MOD_CONTROL|GLFW_MOD_SHIFT), (Event)Event::ON_PRESS},
                {user_actions::UNDO.id, GLFW_KEY_Z, (uint8_t)GLFW_MOD_CONTROL, (Event)Event::ON_PRESS},
                //todo add more
        };

        std::vector<KeyboardShortcut> shortcuts;

        bool shouldCatchNextShortcut;
        std::optional<KeyboardShortcut> caughtShortcut;

        void saveDefaultToDb() {
            for (const auto &shortcut : DEFAULT_SHORTCUTS) {
                db::key_shortcuts::saveShortcut({shortcut.actionId, shortcut.key, shortcut.modifiers, (uint8_t)shortcut.event});
            }
        }
    }

    void initialize() {
        auto dbShortcuts = db::key_shortcuts::loadShortcuts();
        if (dbShortcuts.empty()) {
            saveDefaultToDb();
            shortcuts = DEFAULT_SHORTCUTS;
            spdlog::info("key_shortcuts were empty in DB, load default shortcuts");
        } else {
            for (const auto &record : dbShortcuts) {
                shortcuts.emplace_back(
                        std::get<0>(record),
                        std::get<1>(record),
                        std::get<2>(record),
                        (Event) std::get<3>(record)
                );
            }
        }
    }

    void shortcutPressed(int key, int keyAction, int modifiers, bool isCapturedByGui) {
        for (const auto &modifierKey : ALL_MODIFIER_KEYS) {
            if (modifierKey==key) {
                return;
            }
        }
        modifiers &= ALL_MODIFIERS_MASK;
        auto event = static_cast<Event>(keyAction);
        if (shouldCatchNextShortcut) {
            caughtShortcut = std::make_optional<KeyboardShortcut>(-1, key, modifiers, event);
            spdlog::debug("caught key shortcut {}", caughtShortcut->getDisplayName());
            return;
        }
        if (!isCapturedByGui) {
            for (auto &shortcut : shortcuts) {
                if (shortcut.key == key && shortcut.event == event && (shortcut.modifiers & modifiers) == shortcut.modifiers) {
                    spdlog::debug("event {} matched shortcut, executing action", shortcut.getDisplayName());
                    user_actions::executeAction(shortcut.actionId);
                    return;
                }
            }
            spdlog::debug("event {} did not match any shortcut (key={}, modifiers={:b})", KeyboardShortcut(-1, key, modifiers, event).getDisplayName(), key, modifiers);
        }
    }

    std::vector<KeyboardShortcut> &getAllShortcuts() {
        return shortcuts;
    }

    void replaceAllShortcuts(std::vector<KeyboardShortcut> &newShortcuts) {
        db::key_shortcuts::deleteAll();
        for (const auto &shortcut : newShortcuts) {
            db::key_shortcuts::saveShortcut({shortcut.actionId, shortcut.key, shortcut.modifiers, (uint8_t)shortcut.event});
        }
        shortcuts = newShortcuts;
    }

    void setCatchNextShortcut(bool doCatch) {
        shouldCatchNextShortcut = doCatch;
    }

    std::optional<KeyboardShortcut> &getCaughtShortcut() {
        return caughtShortcut;
    }

    void clearCaughtShortcut() {
        caughtShortcut = {};
    }

    const std::string & getShortcutForAction(int actionId) {
        static std::map<int, std::string> cache;
        auto it = cache.find(actionId);
        if (it == cache.end()) {
            for (auto &shortcut : shortcuts) {
                if (shortcut.actionId==actionId) {
                    return cache[actionId] = shortcut.getDisplayName();
                }
            }
            return cache[actionId] = "";
        }
        return it->second;
    }

    std::string KeyboardShortcut::getDisplayName() {
        std::string displayName;
            for (const auto &mod : ALL_MODIFIERS) {
                if (modifiers & mod) {
                    displayName += MODIFIER_DISPLAY_NAMES[mod];
                    displayName += '+';
                }
            }
            if (GLFW_KEY_KP_0 <= key && key <= GLFW_KEY_KP_EQUAL) {
                displayName += "NUM";
            }

            const auto *keyName = glfwGetKeyName(key, 0);
            std::map<int, const char*>::iterator miscNameIt;
            if (keyName) {
                displayName += keyName;
            } else if (GLFW_KEY_F1 <= key && key <= GLFW_KEY_F25) {
                displayName += 'F';
                displayName += std::to_string(key-GLFW_KEY_F1+1);
            } else if ((miscNameIt=MISC_KEY_DISPLAY_NAMES.find(key))!=MISC_KEY_DISPLAY_NAMES.end()) {
                displayName += miscNameIt->second;
            }
            else {
                displayName += '?';
            }
        util::toUpperInPlace(displayName.data());
        return displayName;
    }

    KeyboardShortcut::KeyboardShortcut(int actionId, int key, uint8_t modifiers, Event event) : actionId(actionId), key(key), modifiers(modifiers), event(event) {}
}