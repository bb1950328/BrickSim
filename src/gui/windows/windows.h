#ifndef BRICKSIM_WINDOWS_H
#define BRICKSIM_WINDOWS_H

#include "../../lib/magic_enum/include/magic_enum.hpp"

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
    };
    struct Data {
        const Id id;
        const char* const name;
        bool visible;
        void (*drawFunction)(Data&);
    };

    void drawAll();

    bool* isVisible(Id id);

    const std::array<Data, magic_enum::enum_count<Id>()>& getData();

    const char* getName(Id id);
}

#endif //BRICKSIM_WINDOWS_H
