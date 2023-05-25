#include "keyboard_shortcut_manager.h"
#include "db.h"
#include "helpers/stringutil.h"
#include "helpers/util.h"
#include "user_actions.h"
#include <GLFW/glfw3.h>
#include <map>
#include <numeric>
#include <spdlog/spdlog.h>

namespace bricksim::keyboard_shortcut_manager {

    namespace {
        const uomap_t<modifier_t, const char*> MODIFIER_DISPLAY_NAMES = {
                // NOLINT(cert-err58-cpp)
                {static_cast<modifier_t>(GLFW_MOD_SHIFT), "Shift"},
                {static_cast<modifier_t>(GLFW_MOD_CONTROL), "Ctrl"},
                {static_cast<modifier_t>(GLFW_MOD_ALT), "Alt"},
#ifdef BRICKSIM_PLATFORM_MACOS
                {static_cast<modifier_t>(GLFW_MOD_SUPER), "Cmd"},
#elif defined(BRICKSIM_PLATFORM_WINDOWS)
                {static_cast<modifier_t>(GLFW_MOD_SUPER), "Win"},
#else
                {static_cast<modifier_t>(GLFW_MOD_SUPER), "Super"},
#endif
                //{static_cast<modifier_t>(GLFW_MOD_CAPS_LOCK), "CapsLock"},
                //{static_cast<modifier_t>(GLFW_MOD_NUM_LOCK), "NumLock"}
        };
        constexpr std::array<std::byte, 4> ALL_MODIFIERS = {
                static_cast<modifier_t>(GLFW_MOD_SHIFT),
                static_cast<modifier_t>(GLFW_MOD_CONTROL),
                static_cast<modifier_t>(GLFW_MOD_ALT),
                static_cast<modifier_t>(GLFW_MOD_SUPER),
        };
        constexpr std::byte ALL_MODIFIERS_MASK = std::accumulate(ALL_MODIFIERS.begin(),
                                                                 ALL_MODIFIERS.end(),
                                                                 std::byte(0),
                                                                 [](auto a, auto b) { return a | b; });
        constexpr std::array<key_t, 8> ALL_MODIFIER_KEYS = {
                GLFW_KEY_LEFT_SHIFT,
                GLFW_KEY_RIGHT_SHIFT,
                GLFW_KEY_LEFT_CONTROL,
                GLFW_KEY_RIGHT_CONTROL,
                GLFW_KEY_LEFT_ALT,
                GLFW_KEY_RIGHT_ALT,
                GLFW_KEY_LEFT_SUPER,
                GLFW_KEY_RIGHT_SUPER,
        };

        const uomap_t<key_t, const char*> MISC_KEY_DISPLAY_NAMES = {
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

        const std::array<KeyboardShortcut, 8> DEFAULT_SHORTCUTS = {{
                // NOLINT(cert-err58-cpp)
                {user_actions::COPY, GLFW_KEY_C, static_cast<modifier_t>(GLFW_MOD_CONTROL), Event::ON_PRESS},
                {user_actions::CUT, GLFW_KEY_X, static_cast<modifier_t>(GLFW_MOD_CONTROL), Event::ON_PRESS},
                {user_actions::PASTE, GLFW_KEY_V, static_cast<modifier_t>(GLFW_MOD_CONTROL), Event::ON_PRESS},
                {user_actions::SAVE_FILE, GLFW_KEY_S, static_cast<modifier_t>(GLFW_MOD_CONTROL), Event::ON_PRESS},
                {user_actions::SAVE_FILE_AS, GLFW_KEY_S, static_cast<modifier_t>(GLFW_MOD_CONTROL | GLFW_MOD_SHIFT), Event::ON_PRESS},
                {user_actions::SELECT_ALL, GLFW_KEY_A, static_cast<modifier_t>(GLFW_MOD_CONTROL), Event::ON_PRESS},
                {user_actions::SELECT_NOTHING, GLFW_KEY_A, static_cast<modifier_t>(GLFW_MOD_CONTROL | GLFW_MOD_SHIFT), Event::ON_PRESS},
                {user_actions::UNDO, GLFW_KEY_Z, static_cast<modifier_t>(GLFW_MOD_CONTROL), Event::ON_PRESS},
                //todo add more
        }};

        std::vector<KeyboardShortcut> shortcuts;

        bool shouldCatchNextShortcut;
        std::optional<KeyboardShortcut> caughtShortcut;

        void saveDefaultToDb() {
            for (const auto& shortcut: DEFAULT_SHORTCUTS) {
                db::key_shortcuts::saveShortcut({shortcut.action, shortcut.key, static_cast<uint8_t>(shortcut.modifiers), static_cast<uint8_t>(shortcut.event)});
            }
        }
    }

    void initialize() {
        auto dbShortcuts = db::key_shortcuts::loadShortcuts();
        if (dbShortcuts.empty()) {
            saveDefaultToDb();
            shortcuts.insert(shortcuts.end(), DEFAULT_SHORTCUTS.begin(), DEFAULT_SHORTCUTS.end());
            spdlog::info("key_shortcuts were empty in DB, load default shortcuts");
        } else {
            for (const auto& [action, key, modifier, event]: dbShortcuts) {
                shortcuts.emplace_back(
                        static_cast<user_actions::Action>(action),
                        key,
                        static_cast<modifier_t>(modifier),
                        static_cast<Event>(event));
            }
        }
    }

    void shortcutPressed(key_t key, int keyAction, modifier_t modifiers, bool isCapturedByGui) {
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
            for (const auto& shortcut: shortcuts) {
                if (shortcut.key == key
                    && (shortcut.event == event || shortcut.event == Event::ON_REPEAT && event == Event::ON_PRESS)
                    && (shortcut.modifiers & modifiers) == shortcut.modifiers) {
                    spdlog::debug("event {} {} matched shortcut, executing action", magic_enum::enum_name(event), shortcut.getDisplayName());
                    try {
                        user_actions::execute(shortcut.action);
                    } catch (const std::invalid_argument& ex) {
                        spdlog::warn("could not execute action because {}", ex.what());
                    }
                    return;
                }
            }
            spdlog::debug("event {} {} did not match any shortcut (key={}, modifiers={:b})", magic_enum::enum_name(event), KeyboardShortcut(user_actions::Action::DO_NOTHING, key, modifiers, event).getDisplayName(), key, modifiers);
        }
    }

    std::vector<KeyboardShortcut>& getAllShortcuts() {
        return shortcuts;
    }

    void replaceAllShortcuts(const std::vector<KeyboardShortcut>& newShortcuts) {
        db::key_shortcuts::deleteAll();
        for (const auto& shortcut: newShortcuts) {
            db::key_shortcuts::saveShortcut({shortcut.action, shortcut.key, static_cast<uint8_t>(shortcut.modifiers), static_cast<uint8_t>(shortcut.event)});
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
            for (const auto& shortcut: shortcuts) {
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
            if ((modifiers & mod) != std::byte(0)) {
                displayName += MODIFIER_DISPLAY_NAMES.find(mod)->second;
                displayName += '+';
            }
        }
        if (GLFW_KEY_KP_0 <= key && key <= GLFW_KEY_KP_EQUAL) {
            displayName += "NUM";
        }

        const auto* keyName = glfwGetKeyName(key, 0);
        if (keyName) {
            displayName += keyName;
        } else if (GLFW_KEY_F1 <= key && key <= GLFW_KEY_F25) {
            displayName += 'F';
            displayName += std::to_string(key - GLFW_KEY_F1 + 1);
        } else if (const auto miscNameIt = MISC_KEY_DISPLAY_NAMES.find(key); miscNameIt != MISC_KEY_DISPLAY_NAMES.end()) {
            displayName += miscNameIt->second;
        } else {
            displayName += '?';
        }
        stringutil::toUpperInPlace(displayName.data());
        return displayName;
    }

    KeyboardShortcut::KeyboardShortcut(user_actions::Action action, int key, modifier_t modifiers, Event event) :
        action(action), key(key), modifiers(modifiers), event(event) {}
    KeyboardShortcut::KeyboardShortcut() :
        action(user_actions::Action::DO_NOTHING), key(0), modifiers(modifier_t(0)), event(Event::ON_PRESS) {}
}
