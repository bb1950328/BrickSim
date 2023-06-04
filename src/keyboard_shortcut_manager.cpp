#include "keyboard_shortcut_manager.h"
#include "db.h"
#include "gui/gui.h"
#include "helpers/stringutil.h"
#include "user_actions.h"
#include <GLFW/glfw3.h>
#include <numeric>
#include <spdlog/spdlog.h>

namespace bricksim::keyboard_shortcut_manager {
    namespace modifier {
        constexpr auto NONE = static_cast<modifier_t>(0);
        constexpr auto CTRL = static_cast<modifier_t>(GLFW_MOD_CONTROL);
        constexpr auto SUPER = static_cast<modifier_t>(GLFW_MOD_SUPER);
        constexpr auto ALT = static_cast<modifier_t>(GLFW_MOD_ALT);
        constexpr auto SHIFT = static_cast<modifier_t>(GLFW_MOD_SHIFT);

        constexpr std::array<modifier_t, 4> ALL = {CTRL, SUPER, ALT, SHIFT};
        const uomap_t<modifier_t, const char*> DISPLAY_NAMES = {
                // NOLINT(cert-err58-cpp)
                {SHIFT, "Shift"},
                {CTRL, "Ctrl"},
                {ALT, "Alt"},
#ifdef BRICKSIM_PLATFORM_MACOS
                {SUPER, "Cmd"},
#elif defined(BRICKSIM_PLATFORM_WINDOWS)
                {SUPER, "Win"},
#else
                {SUPER, "Super"},
#endif
                //{static_cast<modifier_t>(GLFW_MOD_CAPS_LOCK), "CapsLock"},
                //{static_cast<modifier_t>(GLFW_MOD_NUM_LOCK), "NumLock"}
        };

        constexpr std::byte ALL_MASK = std::accumulate(ALL.begin(),
                                                       ALL.end(),
                                                       modifier_t(0),
                                                       [](auto a, auto b) { return a | b; });
        constexpr std::array<key_t, 8> ALL_KEYS = {
                GLFW_KEY_LEFT_SHIFT,
                GLFW_KEY_RIGHT_SHIFT,
                GLFW_KEY_LEFT_CONTROL,
                GLFW_KEY_RIGHT_CONTROL,
                GLFW_KEY_LEFT_ALT,
                GLFW_KEY_RIGHT_ALT,
                GLFW_KEY_LEFT_SUPER,
                GLFW_KEY_RIGHT_SUPER,
        };
    }

    namespace {

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

        constexpr std::array<char, 11> SPECIAL_CHARS = {
                '`',
                '-',
                '=',
                '[',
                ']',
                '\\',
                ',',
                ';',
                '\'',
                '.',
                '/',
        };
        constexpr std::array<int, std::size(SPECIAL_CHARS)> SPECIAL_CHAR_KEYS = {
                GLFW_KEY_GRAVE_ACCENT,
                GLFW_KEY_MINUS,
                GLFW_KEY_EQUAL,
                GLFW_KEY_LEFT_BRACKET,
                GLFW_KEY_RIGHT_BRACKET,
                GLFW_KEY_BACKSLASH,
                GLFW_KEY_COMMA,
                GLFW_KEY_SEMICOLON,
                GLFW_KEY_APOSTROPHE,
                GLFW_KEY_PERIOD,
                GLFW_KEY_SLASH,
        };

        const std::array<KeyboardShortcut, 17> DEFAULT_SHORTCUTS = {{
                // NOLINT(cert-err58-cpp)
                {user_actions::COPY, GLFW_KEY_C, modifier::CTRL, Event::ON_PRESS},
                {user_actions::CUT, GLFW_KEY_X, modifier::CTRL, Event::ON_PRESS},
                {user_actions::PASTE, GLFW_KEY_V, modifier::CTRL, Event::ON_PRESS},
                {user_actions::SAVE_FILE, GLFW_KEY_S, modifier::CTRL, Event::ON_PRESS},
                {user_actions::SAVE_FILE_AS, GLFW_KEY_S, modifier::CTRL | modifier::SHIFT, Event::ON_PRESS},
                {user_actions::SELECT_ALL, GLFW_KEY_A, modifier::CTRL, Event::ON_PRESS},
                {user_actions::SELECT_NOTHING, GLFW_KEY_A, modifier::CTRL | modifier::SHIFT, Event::ON_PRESS},
                {user_actions::UNDO, GLFW_KEY_Z, modifier::CTRL, Event::ON_PRESS},
                {user_actions::START_TRANSFORMING_SELECTED_NODES, GLFW_KEY_T, modifier::NONE, Event::ON_PRESS, gui::windows::Id::VIEW_3D},
                {user_actions::TRANSFORMATION_LOCK_XY, GLFW_KEY_Z, modifier::NONE, Event::ON_PRESS, gui::windows::Id::VIEW_3D},
                {user_actions::TRANSFORMATION_LOCK_XZ, GLFW_KEY_Y, modifier::NONE, Event::ON_PRESS, gui::windows::Id::VIEW_3D},
                {user_actions::TRANSFORMATION_LOCK_YZ, GLFW_KEY_X, modifier::NONE, Event::ON_PRESS, gui::windows::Id::VIEW_3D},
                {user_actions::TRANSFORMATION_LOCK_X, GLFW_KEY_X, modifier::SHIFT, Event::ON_PRESS, gui::windows::Id::VIEW_3D},
                {user_actions::TRANSFORMATION_LOCK_Y, GLFW_KEY_Y, modifier::SHIFT, Event::ON_PRESS, gui::windows::Id::VIEW_3D},
                {user_actions::TRANSFORMATION_LOCK_Z, GLFW_KEY_Z, modifier::SHIFT, Event::ON_PRESS, gui::windows::Id::VIEW_3D},
                {user_actions::END_TRANSFORMATION, GLFW_KEY_ENTER, modifier::NONE, Event::ON_PRESS, gui::windows::Id::VIEW_3D},
                {user_actions::CANCEL_TRANSFORMATION, GLFW_KEY_ESCAPE, modifier::NONE, Event::ON_PRESS, gui::windows::Id::VIEW_3D},
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

        /**
         * Translates key from GLFW value back to the physical value
         * @param keyFromGlfw for example `GLFW_KEY_Z`
         * @return for example `GLFW_KEY_Y` on a machine with QWERTZ keyboard
         */
        key_t translateKey(key_t keyFromGlfw) {
            const char* keyName = glfwGetKeyName(keyFromGlfw, 0);
            if (keyName == nullptr) {
                return keyFromGlfw;
            }
            char charOnKeycap = static_cast<char>(std::tolower(static_cast<unsigned char>(keyName[0])));
            if (GLFW_KEY_A <= keyFromGlfw && keyFromGlfw <= GLFW_KEY_Z) {
                return charOnKeycap - 'a' + GLFW_KEY_A;
            }

            const auto it = std::find(SPECIAL_CHARS.begin(), SPECIAL_CHARS.end(), charOnKeycap);
            if (it != SPECIAL_CHARS.end()) {
                return SPECIAL_CHAR_KEYS[std::distance(SPECIAL_CHARS.begin(), it)];
            }
            return keyFromGlfw;
        }

        /**
         * inverse function of translateKey(key_t)
         * @param translatedKey
         * @return
         */
        key_t untranslateKey(key_t translatedKey) {
            static uomap_t<key_t, key_t> keyTranslation;
            if (keyTranslation.empty()) {
                for (int i = GLFW_KEY_SPACE; i < GLFW_KEY_LAST; ++i) {
                    keyTranslation.emplace(translateKey(i), i);
                }
            }
            if (translateKey(translatedKey) == translatedKey) {
                return translatedKey;
            }
            const auto it = keyTranslation.find(translatedKey);
            return it != keyTranslation.end()
                           ? it->second
                           : translatedKey;
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
        for (const auto& modifierKey: modifier::ALL_KEYS) {
            if (modifierKey == key) {
                return;
            }
        }
        key = translateKey(key);
        modifiers &= modifier::ALL_MASK;
        const auto event = static_cast<Event>(keyAction);
        if (shouldCatchNextShortcut) {
            caughtShortcut = std::make_optional<KeyboardShortcut>(user_actions::Action::DO_NOTHING, key, modifiers, event);
            spdlog::debug("caught key shortcut {}", caughtShortcut->getDisplayName());
            return;
        }
        if (!isCapturedByGui) {
            for (const auto& shortcut: shortcuts) {
                if (shortcut.key == key
                    && (shortcut.event == event || shortcut.event == Event::ON_REPEAT && event == Event::ON_PRESS)
                    && (shortcut.modifiers & modifiers) == shortcut.modifiers
                    && (!shortcut.windowScope.has_value() || shortcut.windowScope == gui::getCurrentlyFocusedWindow())
                    && user_actions::isEnabled(shortcut.action)) {
                    spdlog::debug("event {} {} matched shortcut, executing action {}", magic_enum::enum_name(event), shortcut.getDisplayName(), user_actions::getName(shortcut.action));
                    try {
                        user_actions::execute(shortcut.action);
                    } catch (const std::invalid_argument& ex) {
                        spdlog::warn("could not execute action because {}", ex.what());
                    }
                    return;
                }
            }
            spdlog::trace("event {} {} did not match any shortcut (key={}, modifiers={:b})", magic_enum::enum_name(event), KeyboardShortcut(user_actions::Action::DO_NOTHING, key, modifiers, event).getDisplayName(), key, modifiers);
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
    void resetToDefault() {
        db::key_shortcuts::deleteAll();
        saveDefaultToDb();
        shortcuts.clear();
        shortcuts.insert(shortcuts.end(), DEFAULT_SHORTCUTS.begin(), DEFAULT_SHORTCUTS.end());
    }

    std::string KeyboardShortcut::getDisplayName() const {
        std::string displayName;
        for (const auto& mod: modifier::ALL) {
            if ((modifiers & mod) != std::byte(0)) {
                displayName += modifier::DISPLAY_NAMES.find(mod)->second;
                displayName += '+';
            }
        }
        if (GLFW_KEY_KP_0 <= key && key <= GLFW_KEY_KP_EQUAL) {
            displayName += "NUM";
        }

        const auto* keyName = glfwGetKeyName(untranslateKey(key), 0);
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

    KeyboardShortcut::KeyboardShortcut(user_actions::Action action,
                                       int key,
                                       modifier_t modifiers,
                                       Event event,
                                       std::optional<gui::windows::Id> windowScope) :
        action(action),
        key(key),
        modifiers(modifiers),
        event(event),
        windowScope(windowScope) {}

    KeyboardShortcut::KeyboardShortcut() :
        action(user_actions::Action::DO_NOTHING), key(0), modifiers(modifier_t(0)), event(Event::ON_PRESS), windowScope(std::nullopt) {}
}
