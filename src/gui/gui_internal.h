

#ifndef BRICKSIM_GUI_INTERNAL_H
#define BRICKSIM_GUI_INTERNAL_H

#include <imgui.h>
#include <utility>
#include <string>
#include <vector>
#include "../ldr_files/ldr_colors.h"
#include "../ldr_files/ldr_files.h"
#include "../element_tree.h"

namespace gui_internal {
    void drawPartThumbnail(const ImVec2 &actualThumbSizeSquared, const std::shared_ptr<LdrFile> &part, const LdrColorReference color);
    ImVec4 getWhiteOrBlackBetterContrast(const glm::vec3 &col);
    void drawColorGroup(const std::shared_ptr<etree::MeshNode>& ldrNode, const ImVec2 &buttonSize, const int columnCount, const std::pair<const std::string, std::vector<LdrColorReference>> &colorGroup);
    void draw_hyperlink_button(const std::string &url);
    char getLoFiSpinner();
    const char* getAnimatedHourglassIcon();
    ImTextureID convertTextureId(unsigned int textureId);
}

#endif //BRICKSIM_GUI_INTERNAL_H
