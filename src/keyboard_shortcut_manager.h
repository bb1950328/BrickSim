#pragma once

#include "magic_enum.hpp"
#include <array>
#include <optional>
#include <string>
#include <vector>

namespace bricksim::user_actions {
    enum Action : uint32_t;
}

namespace bricksim::keyboard_shortcut_manager {
    enum class Event : uint8_t {
        ON_RELEASE = 0 /*GLFW_RELEASE*/,
        ON_PRESS = 1 /*GLFW_PRESS*/,
        ON_REPEAT = 2 /*GLFW_REPEAT*/,
    };
    constexpr std::array<const char* const, magic_enum::enum_count<Event>()> EVENT_DISPLAY_NAMES = {"On Release", "On Press", "On Repeat"};

    using modifier_t = std::byte;
    using key_t = int;

    class KeyboardShortcut {
    public:
        user_actions::Action action;
        key_t key;
        modifier_t modifiers;
        Event event;
        [[nodiscard]] std::string getDisplayName() const;
        KeyboardShortcut();
        KeyboardShortcut(user_actions::Action action, int key, modifier_t modifiers, Event event);
        KeyboardShortcut(const KeyboardShortcut& other) = default;
        KeyboardShortcut(KeyboardShortcut&& other) = default;
        KeyboardShortcut& operator=(const KeyboardShortcut& other) = default;
        KeyboardShortcut& operator=(KeyboardShortcut&& other) = default;
    };

    void initialize();
    void shortcutPressed(key_t key, int keyAction, modifier_t modifiers, bool isCapturedByGui);
    std::vector<KeyboardShortcut>& getAllShortcuts();
    void replaceAllShortcuts(const std::vector<KeyboardShortcut>& newShortcuts);
    const std::string& getShortcutForAction(user_actions::Action action);

    void setCatchNextShortcut(bool doCatch);
    std::optional<KeyboardShortcut>& getCaughtShortcut();
    void clearCaughtShortcut();
}
