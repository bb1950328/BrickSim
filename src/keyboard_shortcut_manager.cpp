#include <map>
#include <GLFW/glfw3.h>
#include "keyboard_shortcut_manager.h"
#include "helpers/platform_detection.h"

namespace keyboard_shortcut_manager {
    namespace {
        std::map<int, const char*> MODIFIER_DISPLAY_NAMES = { // NOLINT(cert-err58-cpp)
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
                {GLFW_MOD_CAPS_LOCK, "CapsLock"},
                {GLFW_MOD_NUM_LOCK, "NumLock"}
        };

    }
}