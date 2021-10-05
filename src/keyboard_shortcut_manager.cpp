#include "keyboard_shortcut_manager.h"
#include "db.h"
#include "helpers/stringutil.h"
#include "helpers/util.h"
#include "user_actions.h"
#include <GLFW/glfw3.h>
#include <map>
#include <spdlog/spdlog.h>

namespace bricksim::keyboard_shortcut_manager {
    namespace {
        uomap_t<int, const char*> MODIFIER_DISPLAY_NAMES = {
                // NOLINT(cert-err58-cpp)
                {GLFW_MOD_SHIFT, "Shift"},
                {GLFW_MOD_CONTROL, "Ctrl"},
                {GLFW_MOD_ALT, "Alt"},
#ifdef BRICKSIM_PLATFORM_MACOS
                {GLFW_MOD_SUPER, "Cmd"},
#elif defined(BRICKSIM_PLATFORM_WINDOWS)
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

        uomap_t<int, const char*> MISC_KEY_DISPLAY_NAMES = {
                // NOLINT(cert-err58-cpp)
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

        const std::vector<KeyboardShortcut> DEFAULT_SHORTCUTS = {
                // NOLINT(cert-err58-cpp)
                {user_actions::COPY, GLFW_KEY_C, (uint8_t)GLFW_MOD_CONTROL, (Event)Event::ON_PRESS},
                {user_actions::CUT, GLFW_KEY_X, (uint8_t)GLFW_MOD_CONTROL, (Event)Event::ON_PRESS},
                {user_actions::PASTE, GLFW_KEY_V, (uint8_t)GLFW_MOD_CONTROL, (Event)Event::ON_PRESS},
                {user_actions::SAVE_FILE, GLFW_KEY_S, (uint8_t)GLFW_MOD_CONTROL, (Event)Event::ON_PRESS},
                {user_actions::SAVE_FILE_AS, GLFW_KEY_S, (uint8_t)(GLFW_MOD_CONTROL | GLFW_MOD_SHIFT), (Event)Event::ON_PRESS},
                {user_actions::SELECT_ALL, GLFW_KEY_A, (uint8_t)GLFW_MOD_CONTROL, (Event)Event::ON_PRESS},
                {user_actions::SELECT_NOTHING, GLFW_KEY_A, (uint8_t)(GLFW_MOD_CONTROL | GLFW_MOD_SHIFT), (Event)Event::ON_PRESS},
                {user_actions::UNDO, GLFW_KEY_Z, (uint8_t)GLFW_MOD_CONTROL, (Event)Event::ON_PRESS},
                //todo add more
        };

        std::vector<KeyboardShortcut> shortcuts;

        bool shouldCatchNextShortcut;
        std::optional<KeyboardShortcut> caughtShortcut;

        void saveDefaultToDb() {
            for (const auto& shortcut: DEFAULT_SHORTCUTS) {
                db::key_shortcuts::saveShortcut({shortcut.action, shortcut.key, shortcut.modifiers, (uint8_t)shortcut.event});
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
            for (const auto& record: dbShortcuts) {
                shortcuts.emplace_back(
                        static_cast<user_actions::Action>(std::get<0>(record)),
                        std::get<1>(record),
                        std::get<2>(record),
                        (Event)std::get<3>(record));
            }
        }
    }

    void shortcutPressed(int key, int keyAction, int modifiers, bool isCapturedByGui) {
        for (const auto& modifierKey: ALL_MODIFIER_KEYS) {
            if (modifierKey == key) {
                return;
            }
        }
        modifiers &= ALL_MODIFIERS_MASK;
        auto event = static_cast<Event>(keyAction);
        if (shouldCatchNextShortcut) {
            caughtShortcut = std::make_optional<KeyboardShortcut>(user_actions::Action::DO_NOTHING, key, modifiers, event);
            spdlog::debug("caught key shortcut {}", caughtShortcut->getDisplayName());
            return;
        }
        if (!isCapturedByGui) {
            for (auto& shortcut: shortcuts) {
                if (shortcut.key == key && shortcut.event == event && (shortcut.modifiers & modifiers) == shortcut.modifiers) {
                    spdlog::debug("event {} matched shortcut, executing action", shortcut.getDisplayName());
                    user_actions::execute(shortcut.action);
                    return;
                }
            }
            spdlog::debug("event {} did not match any shortcut (key={}, modifiers={:b})", KeyboardShortcut(user_actions::Action::DO_NOTHING, key, modifiers, event).getDisplayName(), key, modifiers);
        }
    }

    std::vector<KeyboardShortcut>& getAllShortcuts() {
        return shortcuts;
    }

    void replaceAllShortcuts(std::vector<KeyboardShortcut>& newShortcuts) {
        db::key_shortcuts::deleteAll();
        for (const auto& shortcut: newShortcuts) {
            db::key_shortcuts::saveShortcut({shortcut.action, shortcut.key, shortcut.modifiers, (uint8_t)shortcut.event});
        }
        shortcuts = newShortcuts;
    }

    void setCatchNextShortcut(bool doCatch) {
        shouldCatchNextShortcut = doCatch;
    }

    std::optional<KeyboardShortcut>& getCaughtShortcut() {
        return caughtShortcut;
    }

    void clearCaughtShortcut() {
        caughtShortcut = {};
    }

    const std::string& getShortcutForAction(user_actions::Action action) {
        static uomap_t<int, std::string> cache;
        auto it = cache.find(action);
        if (it == cache.end()) {
            for (auto& shortcut: shortcuts) {
                if (shortcut.action == action) {
                    return cache[action] = shortcut.getDisplayName();
                }
            }
            return cache[action] = "";
        }
        return it->second;
    }

    std::string KeyboardShortcut::getDisplayName() const {
        std::string displayName;
        for (const auto& mod: ALL_MODIFIERS) {
            if (modifiers & mod) {
                displayName += MODIFIER_DISPLAY_NAMES[mod];
                displayName += '+';
            }
        }
        if (GLFW_KEY_KP_0 <= key && key <= GLFW_KEY_KP_EQUAL) {
            displayName += "NUM";
        }

        const auto* keyName = glfwGetKeyName(key, 0);
        uomap_t<int, const char*>::iterator miscNameIt;
        if (keyName) {
            displayName += keyName;
        } else if (GLFW_KEY_F1 <= key && key <= GLFW_KEY_F25) {
            displayName += 'F';
            displayName += std::to_string(key - GLFW_KEY_F1 + 1);
        } else if ((miscNameIt = MISC_KEY_DISPLAY_NAMES.find(key)) != MISC_KEY_DISPLAY_NAMES.end()) {
            displayName += miscNameIt->second;
        } else {
            displayName += '?';
        }
        stringutil::toUpperInPlace(displayName.data());
        return displayName;
    }

    KeyboardShortcut::KeyboardShortcut(user_actions::Action action, int key, uint8_t modifiers, Event event) :
        action(action), key(key), modifiers(modifiers), event(event) {}
    KeyboardShortcut::KeyboardShortcut() :
        action(user_actions::Action::DO_NOTHING), key(0), modifiers(0), event(Event::ON_PRESS) {}
}