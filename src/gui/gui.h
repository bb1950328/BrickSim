//
// Created by bb1950328 on 09.10.2020.
//

#ifndef BRICKSIM_GUI_H
#define BRICKSIM_GUI_H


#include "../ldr_files.h"
#include "../element_tree.h"
#include <imgui.h>

static const int NUM_LDR_FILTER_PATTERNS = 3;

static const int NUM_IMAGE_FILTER_PATTERNS = 4;

namespace gui {
    namespace {
        void drawPartThumbnail(const ImVec2 &actualThumbSizeSquared, LdrFile *const &part, LdrColor *color);
        ImVec4 getWhiteOrBlackBetterContrast(const glm::vec3 &col);
        void drawColorGroup(etree::MeshNode *ldrNode, const ImVec2 &buttonSize, int columnCount, const std::pair<const std::string, std::vector<const LdrColor *>> &colorGroup);
        void draw_element_tree_node(etree::Node *node);
        void draw_hyperlink_button(const std::string &url);
    }
    void setWindow(GLFWwindow* value);
    GLFWwindow* getWindow();
    void setLastScrollDeltaY(double value);
    double getLastScrollDeltaY();
    void setup();
    [[nodiscard]] bool isSetupDone();
    void loop();

    void cleanup();

    bool loopPartsLibraryInstallationScreen();//returns true when finished

    void drawWaitMessage(const std::string& message);
};

#endif //BRICKSIM_GUI_H
