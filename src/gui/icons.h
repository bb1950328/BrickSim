#pragma once
#include "imgui.h"
#include "magic_enum.hpp"
namespace bricksim::gui::icons {
    enum IconType {
        Select,
        SelectConnected,
        SelectStronglyConnected,
    };
    [[nodiscard]] ImVec2 getU(IconType icon);
    [[nodiscard]] ImVec2 getV(IconType icon);
    [[nodiscard]] ImTextureID getTexture();
    bool drawButton(IconType icon, float size);

    void initialize();
    void cleanup();
}
