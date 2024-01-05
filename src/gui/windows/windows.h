#pragma once

#include "magic_enum.hpp"

namespace bricksim::gui::windows {
    enum class Id {
        VIEW_3D,
        ELEMENT_TREE,
        ELEMENT_PROPERTIES,
        PART_PALETTE,
        SETTINGS,
        ABOUT,
        SYSTEM_INFO,
        DEBUG,
        MESH_INSPECTOR,
        IMGUI_DEMO,
        ORIENTATION_CUBE,
        LOG,
        GEAR_RATIO_CALCULATOR,
        MODEL_INFO,
        EDITOR_META_INFO,
        LDRAW_FILE_INSPECTOR,
        TOOLBAR,
        CONNECTION_VISUALIZATION,
    };

    struct Data {
        const Id id;
        const char* const name;
        bool visible;
        void (*drawFunction)(Data&);
        void (*cleanupFunction)();
    };

    void drawAll();

    void cleanup();

    bool* isVisible(Id id);

    const std::array<Data, magic_enum::enum_count<Id>()>& getData();

    const char* getName(Id id);
}
