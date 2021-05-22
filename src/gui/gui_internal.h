#ifndef BRICKSIM_GUI_INTERNAL_H
#define BRICKSIM_GUI_INTERNAL_H

#include <imgui.h>
#include <memory>
#include <glm/glm.hpp>
#include <vector>
#include "../ldr_files/ldr_files.h"
#include "../ldr_files/ldr_colors.h"
#include "../element_tree.h"
#include "../user_actions.h"

namespace gui_internal {
    /**
     * @return true if the thumbnail is visible, false if it's clipped
     */
    bool drawPartThumbnail(const ImVec2 &actualThumbSizeSquared, const std::shared_ptr<LdrFile> &part, const LdrColorReference color);

    ImVec4 getWhiteOrBlackBetterContrast(const glm::vec3 &col);

    void drawColorGroup(const std::shared_ptr<etree::MeshNode> &ldrNode, const ImVec2 &buttonSize, int columnCount,
                        const std::pair<const std::string, std::vector<LdrColorReference>> &colorGroup);

    void drawHyperlinkButton(const std::string &url);

    char getLoFiSpinner();
    const char *getAnimatedHourglassIcon();

    ImTextureID convertTextureId(unsigned int textureId);

    const char *getShortcutText(const user_actions::Action &action);

    void actionMenuItem(const user_actions::Action &action);
    void actionMenuItem(const user_actions::Action &action, const char *alternativeDescription);
}

#endif //BRICKSIM_GUI_INTERNAL_H
