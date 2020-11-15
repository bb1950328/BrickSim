//
// Created by Bader on 15.11.2020.
//

#ifndef BRICKSIM_GUI_INTERNAL_H
#define BRICKSIM_GUI_INTERNAL_H

#include <imgui.h>
#include <utility>
#include <string>
#include <vector>
#include "../ldr_colors.h"
#include "../ldr_files.h"
#include "../element_tree.h"

namespace gui_internal {
    void drawPartThumbnail(const ImVec2 &actualThumbSizeSquared, LdrFile *const &part, LdrColor *color);
    ImVec4 getWhiteOrBlackBetterContrast(const glm::vec3 &col);
    void drawColorGroup(etree::MeshNode *ldrNode, const ImVec2 &buttonSize, int columnCount, const std::pair<const std::string, std::vector<const LdrColor *>> &colorGroup);
    void draw_hyperlink_button(const std::string &url);
}

#endif //BRICKSIM_GUI_INTERNAL_H
