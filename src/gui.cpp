//
// Created by bb1950328 on 09.10.2020.
//
#define GLM_ENABLE_EXPERIMENTAL

#include <imgui.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>
#include "gui.h"
#include "config.h"
#include "controller.h"
#include "git_stats.h"
#include "lib/tinyfiledialogs.h"
#include "info_providers/part_color_availability_provider.h"
#include "info_providers/price_guide_provider.h"
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>
#include <atomic>
#include <imgui_internal.h>

void drawPartThumbnail(const ImVec2 &actualThumbSizeSquared, LdrFile *const &part, LdrColor *color);

ImVec4 getWhiteOrBlackBetterContrast(const glm::vec3& col) {
    return util::vectorSum(col) > 1.5 ? ImVec4(0, 0, 0, 1) : ImVec4(1, 1, 1, 1);
}

void Gui::setup() {
    if (!setupDone) {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();//todo get the monitor on which the window is
        float xscale, yscale;
        glfwGetMonitorContentScale(monitor, &xscale, &yscale);
        std::cout << "xscale: " << xscale << "\tyscale: " << yscale << std::endl;
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto scaleFactor = (float) (config::getDouble(config::GUI_SCALE));
        if (xscale > 1 || yscale > 1) {
            scaleFactor = (xscale + yscale) / 2.0f;
            ImGuiStyle &style = ImGui::GetStyle();
            style.ScaleAllSizes(scaleFactor);
            glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
        }
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.Fonts->AddFontFromFileTTF("RobotoMono-Regular.ttf", 13.0f * scaleFactor, nullptr, nullptr);
        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        // Setup Dear ImGui style
        auto guiStyle = config::getString(config::GUI_STYLE);
        if (guiStyle == "light") {
            ImGui::StyleColorsLight();
        } else if (guiStyle == "classic") {
            ImGui::StyleColorsClassic();
        } else if (guiStyle == "dark") {
            ImGui::StyleColorsDark();
        } else {
            std::cout << "WARNING: please set " << config::GUI_STYLE.name << "to light, classic or dark" << std::endl;
        }
        setupDone = true;
    } else {
        throw std::invalid_argument("setup called twice");
    }
}

void drawColorGroup(etree::MeshNode *ldrNode,
                    const ImVec2 &buttonSize,
                    const int columnCount,
                    const std::pair<const std::string, std::vector<const LdrColor *>> &colorGroup) {
    if (ImGui::TreeNodeEx(colorGroup.first.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
        int i = 0;
        for (const auto *color : colorGroup.second) {
            if (i % columnCount > 0) {
                ImGui::SameLine();
            }
            ImGui::PushID(color->code);
            const ImColor imColor = ImColor(color->value.red, color->value.green, color->value.blue);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4) imColor);
            if (ImGui::Button(ldrNode->getDisplayColor()->code == color->code ? "#" : "", buttonSize)) {
                ldrNode->setColor(ldr_color_repo::get_color(color->code));
                controller::setElementTreeChanged(true);
            }
            ImGui::PopStyleColor(/*3*/1);
            ImGui::PopID();
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("%s", color->name.c_str());
                ImGui::EndTooltip();
            }
            ++i;
        }
        ImGui::TreePop();
    }
}

void draw_element_tree_node(etree::Node *node) {
    if (node->visibleInElementTree) {
        auto colorVec = glm::vec4(getColorOfType(node->getType()).asGlmVector(), 1.0);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(colorVec.x, colorVec.y, colorVec.z, colorVec.w));

        bool itemClicked = false;
        if (node->getChildren().empty()) {
            auto flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            if (node->selected) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }
            ImGui::TreeNodeEx(node->getDescription().c_str(), flags);
            itemClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
            //todo add context menu
        } else {
            auto flags = node->selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None;
            if (ImGui::TreeNodeEx(node->displayName.c_str(), flags)) {
                itemClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
                for (const auto &child: node->getChildren()) {
                    draw_element_tree_node(child);
                }
                ImGui::TreePop();
            }
        }
        ImGui::PopStyleColor();
        if (itemClicked) {
            if (ImGui::GetIO().KeyCtrl) {
                controller::nodeSelectAddRemove(node);
            } else if (ImGui::GetIO().KeyShift) {
                controller::nodeSelectUntil(node);
            } else {
                controller::nodeSelectSet(node);
            }
        }
    }
}

void draw_hyperlink_button(const std::string &url) {
    if (ImGui::Button(url.c_str())) {
        util::openDefaultBrowser(url);
    }
}

void Gui::loop() {
    static bool show3dWindow = true;
    static bool showElementTreeWindow = true;
    static bool showElementPropertiesWindow = true;
    static bool showSettingsWindow = false;
    static bool showDemoWindow = true;
    static bool showDebugWindow = true;
    static bool showAboutWindow = false;
    static bool showSysInfoWindow = false;
    static bool showPartPaletteWindow = true;
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.6f, 1.0f);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "CTRL+O")) {
                char *fileNameChars = tinyfd_openFileDialog(
                        "Open File",
                        "",
                        NUM_LDR_FILTER_PATTERNS,
                        lFilterPatterns,
                        nullptr,
                        0);
                if (fileNameChars != nullptr) {
                    std::string fileName(fileNameChars);
                    controller::openFile(fileName);
                }
            }
            if (ImGui::MenuItem("Exit", "CTRL+W")) {
                controller::setUserWantsToExit(true);
            }
            ImGui::MenuItem("About", "", &showAboutWindow);
            ImGui::MenuItem("System Info", "", &showSysInfoWindow);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            //todo implement these functions
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("3D View", "ALT+V", &show3dWindow);
            ImGui::MenuItem("Element Tree", "ALT+T", &showElementTreeWindow);
            ImGui::MenuItem("Element Properties", "ALT+P", &showElementPropertiesWindow);
            ImGui::MenuItem("Part Palette", "ALT+N", &showPartPaletteWindow);
            ImGui::MenuItem("Settings", "ALT+S", &showSettingsWindow);
            ImGui::Separator();
            ImGui::MenuItem("Demo", "", &showDemoWindow);
            ImGui::MenuItem("Debug", "ALT+D", &showDebugWindow);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Selection")) {
            if (ImGui::MenuItem("Select All", "CTRL+A")) {
                controller::nodeSelectAll();
            }
            if (ImGui::MenuItem("Select Nothing", "CTRL+U")) {
                controller::nodeSelectNone();
            }
            ImGui::TextDisabled("%lu Elements currently selected", controller::getSelectedNodes().size());
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("3D")) {
            if (ImGui::MenuItem("Front", "ALT+1")) { controller::setStandard3dView(1); }
            if (ImGui::MenuItem("Top", "ALT+2")) { controller::setStandard3dView(2); }
            if (ImGui::MenuItem("Right", "ALT+3")) { controller::setStandard3dView(3); }
            if (ImGui::MenuItem("Rear", "ALT+4")) { controller::setStandard3dView(4); }
            if (ImGui::MenuItem("Bottom", "ALT+5")) { controller::setStandard3dView(5); }
            if (ImGui::MenuItem("Left", "ALT+6")) { controller::setStandard3dView(6); }
            ImGui::Separator();
            if (ImGui::MenuItem("Screenshot", "CTRL+P")) {
                char *fileNameChars = tinyfd_saveFileDialog(
                        "Save Screenshot",
                        "",
                        NUM_IMAGE_FILTER_PATTERNS,
                        imageFilterPatterns,
                        nullptr);
                if (fileNameChars != nullptr) {
                    std::string fileNameString(fileNameChars);
                    controller::getRenderer()->saveImage(fileNameString);
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (show3dWindow) {
        ImGui::Begin("3D View", &show3dWindow, ImGuiWindowFlags_NoScrollWithMouse);
        {
            ImGui::BeginChild("3DRender");
            ImVec2 wsize = ImGui::GetContentRegionAvail();
            controller::set3dViewSize((unsigned int) wsize.x, (unsigned int) wsize.y);
            const ImVec2 &windowPos = ImGui::GetWindowPos();
            const ImVec2 &regionMin = ImGui::GetWindowContentRegionMin();
            const ImVec2 &mousePos = ImGui::GetMousePos();
            const ImVec2 &regionMax = ImGui::GetWindowContentRegionMax();
            bool isInWindow = (windowPos.x + regionMin.x <= mousePos.x
                               && mousePos.x <= windowPos.x + regionMax.x
                               && windowPos.y + regionMin.y <= mousePos.y
                               && mousePos.y <= windowPos.y + regionMax.y);
            static float lastDeltaXleft = 0, lastDeltaYleft = 0;
            static float lastDeltaXright = 0, lastDeltaYright = 0;
            static bool leftMouseDragging = false;
            //std::cout << ImGui::GetScrollX() << "\t" << ImGui::GetScrollY() << std::endl;
            if (isInWindow && ImGui::IsWindowFocused()) {
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                    leftMouseDragging = true;
                    const ImVec2 &leftBtnDrag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                    controller::getRenderer()->camera.mouseRotate(leftBtnDrag.x - lastDeltaXleft,
                                                                  (leftBtnDrag.y - lastDeltaYleft) * -1);
                    controller::getRenderer()->unrenderedChanges = true;
                    lastDeltaXleft = leftBtnDrag.x;
                    lastDeltaYleft = leftBtnDrag.y;
                } else {
                    lastDeltaXleft = 0;
                    lastDeltaYleft = 0;
                }
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
                    const ImVec2 &rightBtnDrag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
                    controller::getRenderer()->camera.mousePan(rightBtnDrag.x - lastDeltaXright,
                                                               (rightBtnDrag.y - lastDeltaYright) * -1);
                    controller::getRenderer()->unrenderedChanges = true;
                    lastDeltaXright = rightBtnDrag.x;
                    lastDeltaYright = rightBtnDrag.y;
                } else {
                    lastDeltaXright = 0;
                    lastDeltaYright = 0;
                }
                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && lastDeltaXleft == 0 && lastDeltaYleft == 0) {
                    if (!leftMouseDragging) {
                        auto relCursorPosX = ImGui::GetMousePos().x - ImGui::GetWindowPos().x - ImGui::GetWindowContentRegionMin().x;
                        auto relCursorPosY = ImGui::GetMousePos().y - ImGui::GetWindowPos().y - ImGui::GetWindowContentRegionMin().y;
                        //std::cout << "relCursorPos: " << relCursorPosX << ", " << relCursorPosY << std::endl;
                        //std::cout << "GetWindowPos: " << ImGui::GetWindowPos().x << ", " << ImGui::GetWindowPos().y << std::endl;
                        //std::cout << "GetMousePos: " << ImGui::GetMousePos().x << ", " << ImGui::GetMousePos().y << std::endl << std::endl;
                        const auto elementIdUnderMouse = controller::getRenderer()->getSelectionPixel(relCursorPosX, relCursorPosY);
                        if (elementIdUnderMouse == 0) {
                            controller::nodeSelectNone();
                        } else {
                            auto *clickedNode = controller::getRenderer()->meshCollection.getElementById(elementIdUnderMouse);
                            if (clickedNode != nullptr) {
                                if (ImGui::GetIO().KeyCtrl) {
                                    controller::nodeSelectAddRemove(clickedNode);
                                } else if (ImGui::GetIO().KeyShift) {
                                    controller::nodeSelectUntil(clickedNode);
                                } else {
                                    controller::nodeSelectSet(clickedNode);
                                }
                                //std::cout << clickedNode->displayName << std::endl;
                            }
                        }
                    }
                    leftMouseDragging = false;
                }
                if (std::abs(lastScrollDeltaY) > 0.01) {
                    controller::getRenderer()->camera.moveForwardBackward((float) lastScrollDeltaY);
                    controller::getRenderer()->unrenderedChanges = true;
                }
            }
            auto texture3dView = (ImTextureID) (config::getBool(config::DISPLAY_SELECTION_BUFFER)
                                                ? controller::getRenderer()->selectionTextureColorbuffer
                                                : controller::getRenderer()->imageTextureColorbuffer);
            ImGui::ImageButton(texture3dView, wsize, ImVec2(0, 1), ImVec2(1, 0), 0);
            ImGui::EndChild();
        }
        ImGui::End();
    }

    if (showElementTreeWindow) {
        ImGui::Begin("Element Tree", &showElementTreeWindow);
        for (auto *rootChild : controller::getElementTree().rootNode.getChildren()) {
            draw_element_tree_node(rootChild);
        }
        ImGui::End();
    }

    static etree::Node *lastSelectedNode = nullptr;
    if (showElementPropertiesWindow) {
        ImGui::Begin("Element Properties", &showElementPropertiesWindow);
        if (controller::getSelectedNodes().empty()) {
            ImGui::Text("Select an element to view its properties here");
        } else if (controller::getSelectedNodes().size() == 1) {
            auto node = *controller::getSelectedNodes().begin();

            static char displayNameBuf[255];
            if (nullptr != lastSelectedNode) {
                lastSelectedNode->displayName = std::string(displayNameBuf);
            }
            strcpy(displayNameBuf, node->displayName.data());
            const auto displayNameEditable = node->isDisplayNameUserEditable();
            auto flags = displayNameEditable ? ImGuiInputTextFlags_None : ImGuiInputTextFlags_ReadOnly;
            ImGui::InputText("Name", displayNameBuf, 255, flags);
            if (!displayNameEditable && ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted("Changing the name of an element of this type is not possible.");
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
            auto typeColor = getColorOfType(node->getType()).asGlmVector();
            //ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(typeColor.x, typeColor.y, typeColor.z, 1.0));//todo make this affect only text but not label
            static char typeBuffer[255];
            strcpy(typeBuffer, getDisplayNameOfType(node->getType()));
            ImGui::InputText("Type", typeBuffer, 255, ImGuiInputTextFlags_ReadOnly);
            //ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted("Type is not mutable");
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }

            if (node->isTransformationUserEditable()) {
                auto treeRelTransf = glm::transpose(node->getRelativeTransformation());
                glm::quat treeOrientation;
                glm::vec3 treeSkew;
                glm::vec4 treePerspective;
                glm::vec3 treePosition;
                glm::vec3 treeScale;
                glm::vec3 treeEulerAnglesRad;
                glm::decompose(treeRelTransf, treeScale, treeOrientation, treePosition, treeSkew, treePerspective);
                treeEulerAnglesRad = glm::eulerAngles(treeOrientation);

                static glm::vec3 inputEulerAnglesDeg;
                static glm::vec3 inputPosition;
                static glm::vec3 inputScalePercent;
                if (lastSelectedNode != node) {
                    inputEulerAnglesDeg = treeEulerAnglesRad * (float) (180.0f / M_PI);
                    inputPosition = treePosition;
                    inputScalePercent = treeScale * 100.0f;
                }
                glm::vec3 inputEulerAnglesRad = inputEulerAnglesDeg * (float) (M_PI / 180.0f);

                if ((util::biggestValue(glm::abs(treePosition - inputPosition)) > 0.01)
                    || (util::biggestValue(glm::abs(inputEulerAnglesRad - treeEulerAnglesRad)) > 0.0001)
                    || (util::biggestValue(glm::abs(inputScalePercent / 100.0f - treeScale)) > 0.001)) {
                    auto newRotation = glm::eulerAngleXYZ(inputEulerAnglesRad.x, inputEulerAnglesRad.y,
                                                          inputEulerAnglesRad.z);
                    auto newTranslation = glm::translate(glm::mat4(1.0f), inputPosition);
                    auto newScale = glm::scale(glm::mat4(1.0f), inputScalePercent / 100.0f);
                    auto newTransformation = newTranslation * newRotation * newScale;
                    if (treeRelTransf != newTransformation) {
                        node->setRelativeTransformation(glm::transpose(newTransformation));
                        controller::setElementTreeChanged(true);
                    }
                }

                ImGui::DragFloat3("Rotation", &inputEulerAnglesDeg[0], 1.0f, -180, 180, "%.1f°");
                ImGui::DragFloat3("Position", &inputPosition[0], 1.0f, -1e9, 1e9, "%.0fLDU");
                ImGui::DragFloat3("Scale", &inputScalePercent[0], 1.0f, -1e9, 1e9, "%.2f%%");
            }
            if ((node->getType() & etree::NodeType::TYPE_MESH) > 0) {
                auto *meshNode = dynamic_cast<etree::MeshNode *>(node);
                if (meshNode->isColorUserEditable() && ImGui::TreeNodeEx("Color", ImGuiTreeNodeFlags_DefaultOpen)) {
                    static bool isColor16, savedIsColor16;
                    if (lastSelectedNode == node) {
                        if (isColor16 != savedIsColor16) {
                            if (isColor16) {
                                meshNode->setColor(ldr_color_repo::get_color(LdrColor::MAIN_COLOR_CODE));
                            } else {
                                meshNode->setColor(meshNode->getDisplayColor());
                            }
                            savedIsColor16 = isColor16;
                        }
                    } else {
                        savedIsColor16 = isColor16 = meshNode->getElementColor()->code == LdrColor::MAIN_COLOR_CODE;
                    }
                    ImGui::Checkbox("Take color from parent element", &isColor16);
                    if (!isColor16) {
                        const auto buttonWidth = ImGui::GetFontSize() * 1.5f;
                        const ImVec2 &buttonSize = ImVec2(buttonWidth, buttonWidth);
                        const int columnCount = std::floor(ImGui::GetContentRegionAvailWidth() / (buttonWidth + ImGui::GetStyle().ItemSpacing.x));

                        std::optional<std::set<const LdrColor *>> availableColors = std::nullopt;
                        if (meshNode->getType() == etree::TYPE_PART) {
                            availableColors = part_color_availability_provider::getAvailableColorsForPart(dynamic_cast<etree::LdrNode *>(meshNode)->ldrFile);
                        }
                        bool showAllColors;
                        if (availableColors.has_value() && !availableColors.value().empty()) {
                            static bool onlyAvailableChecked = false;
                            ImGui::Checkbox("Only show available Colors", &onlyAvailableChecked);
                            showAllColors = !onlyAvailableChecked;
                            if (onlyAvailableChecked) {
                                std::pair<std::string, std::vector<const LdrColor *>> group = std::make_pair("Available", std::vector<const LdrColor *>());
                                for (const auto &color : availableColors.value()) {
                                    group.second.push_back(color);
                                }
                                drawColorGroup(meshNode, buttonSize, columnCount, group);
                            }
                        } else {
                            showAllColors = true;
                        }
                        if (showAllColors) {
                            const auto &groupedAndSortedByHue = ldr_color_repo::getAllColorsGroupedAndSortedByHue();
                            const static std::vector<std::string> fixed_pos = {"Solid", "Transparent", "Rubber"};
                            for (const auto &colorName : fixed_pos) {
                                const auto &colorGroup = std::make_pair(colorName, groupedAndSortedByHue.find(colorName)->second);
                                drawColorGroup(meshNode, buttonSize, columnCount, colorGroup);
                            }
                            for (const auto &colorGroup : groupedAndSortedByHue) {
                                if (std::find(fixed_pos.begin(), fixed_pos.end(), colorGroup.first) == fixed_pos.end()) {
                                    drawColorGroup(meshNode, buttonSize, columnCount, colorGroup);
                                }
                            }
                        }
                    }
                    ImGui::TreePop();
                }
            }
            if (node->getType() == etree::NodeType::TYPE_PART) {
                if (ImGui::TreeNodeEx("Price Guide")) {
                    auto *partNode = dynamic_cast<etree::PartNode *>(node);
                    auto partCode = partNode->ldrFile->metaInfo.name;
                    util::replaceAll(partCode, ".dat", "");
                    const auto color = partNode->getDisplayColor();
                    const auto currencyCode = config::getString(config::BRICKLINK_CURRENCY_CODE);
                    const auto colorBricklinkName = util::translateLDrawColorNameToBricklink(color->name);
                    auto availableColors = part_color_availability_provider::getAvailableColorsForPart(partNode->ldrFile);
                    if (availableColors.has_value()) {
                        if (ImGui::Button("(Re)load all available colors")) {
                            for (const auto &item : availableColors.value()) {
                                controller::addBackgroundTask("Reload price guide for " + partCode + " in " + item->name, [partCode, item, currencyCode]() {
                                    price_guide_provider::getPriceGuide(partCode, currencyCode, util::translateLDrawColorNameToBricklink(item->name), true);
                                });
                            }
                        }
                    }
                    std::map<const LdrColor *, const price_guide_provider::PriceGuide> pGuides;
                    if (availableColors.has_value()) {
                        for (const auto &item : availableColors.value()) {
                            auto pg = price_guide_provider::getPriceGuideIfCached(partCode, currencyCode, util::translateLDrawColorNameToBricklink(item->name));
                            if (pg.has_value()) {
                                pGuides.emplace(item, pg.value());
                            }
                        }
                    } else {
                        auto pg = price_guide_provider::getPriceGuideIfCached(partCode, currencyCode, colorBricklinkName);
                        if (pg.has_value()) {
                            pGuides.emplace(color, pg.value());
                        }
                    }
                    if (!pGuides.empty()) {
                        if (pGuides.find(color) != pGuides.end()) {
                            if (ImGui::Button(("Reload for " + color->name).c_str())) {
                                controller::addBackgroundTask("Reload price guide for " + partCode, [partCode, colorBricklinkName, currencyCode]() {
                                    price_guide_provider::getPriceGuide(partCode, currencyCode, colorBricklinkName, true);
                                });
                            }
                        }
                        ImGui::Text("Currency: %s", currencyCode.c_str());
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Can be changed in settings");
                        }

                        const auto windowBgImVec = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
                        const auto windowBg = glm::vec3(windowBgImVec.x, windowBgImVec.y, windowBgImVec.z);
                        auto drawColoredValueText = [&windowBg](const char* text, const LdrColor* color){
                            ImGui::SameLine();
                            auto col = color->value.asGlmVector();
                            if (util::vectorSum(glm::abs(windowBg - col)) < 0.3) {
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(col.x, col.y, col.z, 1.0f));
                                auto bgColor = getWhiteOrBlackBetterContrast(col);
                                ImGui::PushStyleColor(ImGuiCol_Button, bgColor);
                                ImGui::PushStyleColor(ImGuiCol_ButtonActive, bgColor);
                                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgColor);
                                ImGui::Button(text);
                                ImGui::PopStyleColor(4);
                            } else {
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(col.x, col.y, col.z, 1.0f));
                                ImGui::Text("%s", text);
                                ImGui::PopStyleColor();
                            }
                            if (ImGui::IsItemHovered()) {
                                ImGui::SetTooltip("%s", color->name.c_str());
                            }
                        };

                        static char valueBuffer[10];

                        ImGui::Text("Total Lots: ");
                        for (const auto &pGuide : pGuides) {
                            snprintf(valueBuffer, 10, "%d", pGuide.second.totalLots);
                            drawColoredValueText(valueBuffer, pGuide.first);
                        }

                        ImGui::Text("Total Qty: ");
                        for (const auto &pGuide : pGuides) {
                            snprintf(valueBuffer, 10, "%d", pGuide.second.totalQty);
                            drawColoredValueText(valueBuffer, pGuide.first);
                        }

                        ImGui::Text("Min Price: ");
                        for (const auto &pGuide : pGuides) {
                            snprintf(valueBuffer, 10, pGuide.second.minPrice<0.05?"%.4f":"%.2f", pGuide.second.minPrice);
                            drawColoredValueText(valueBuffer, pGuide.first);
                        }

                        ImGui::Text("Avg Price: ");
                        for (const auto &pGuide : pGuides) {
                            snprintf(valueBuffer, 10, pGuide.second.minPrice<0.05?"%.4f":"%.2f", pGuide.second.avgPrice);
                            drawColoredValueText(valueBuffer, pGuide.first);
                        }

                        ImGui::Text("Qty avg Price: ");
                        for (const auto &pGuide : pGuides) {
                            snprintf(valueBuffer, 10, pGuide.second.minPrice<0.05?"%.4f":"%.2f", pGuide.second.qtyAvgPrice);
                            drawColoredValueText(valueBuffer, pGuide.first);
                        }

                        ImGui::Text("Max Price: ");
                        for (const auto &pGuide : pGuides) {
                            snprintf(valueBuffer, 10, pGuide.second.minPrice<0.05?"%.4f":"%.2f", pGuide.second.maxPrice);
                            drawColoredValueText(valueBuffer, pGuide.first);
                        }
                        //todo a small histogram would be nice (parse data from price guide html table)
                    } else {
                        if (ImGui::Button(("Get for " + color->name).c_str())) {
                            controller::addBackgroundTask("Get Price Guide for " + partCode, [partCode, colorBricklinkName, currencyCode]() {
                                price_guide_provider::getPriceGuide(partCode, currencyCode, colorBricklinkName, false);
                            });
                        }
                        if (availableColors.has_value() && ImGui::Button(("Get for all " + std::to_string(availableColors.value().size()) + " available colors").c_str())) {
                            for (const auto &avCol : availableColors.value()) {
                                controller::addBackgroundTask("Get Price Guide for " + partCode + " in " + avCol->name, [partCode, avCol, currencyCode]() {
                                    price_guide_provider::getPriceGuide(partCode, currencyCode, util::translateLDrawColorNameToBricklink(avCol->name), false);
                                });
                            }
                        }
                    }
                    ImGui::TreePop();
                }
            }

            lastSelectedNode = node;
        } else {
        }

        if (!controller::getSelectedNodes().empty()) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8, 0, 0, 1));
            std::string deleteButtonLabel = controller::getSelectedNodes().size() > 1
                                            ? (std::string("Delete ") + std::to_string(controller::getSelectedNodes().size()) + " elements")
                                            : "Delete Element";
            const auto deleteClicked = ImGui::Button(deleteButtonLabel.c_str());
            ImGui::PopStyleColor();
            if (deleteClicked) {
                controller::deleteSelectedElements();
            }
        }

        ImGui::End();
    }

    if (showPartPaletteWindow) {
        ImGui::Begin("Part palette", &showPartPaletteWindow);

        static char searchTextBuffer[128] = {'\0'};
        ImGui::InputText("##search", searchTextBuffer, 128);
        ImGui::SameLine();
        static int thumbnailZoomPercent = 100;//todo get from config
        ImGui::SetNextItemWidth(ImGui::GetFontSize()*8);
        ImGui::DragInt("##Zoom", &thumbnailZoomPercent, 5, 10, 500, " Zoom: %d%%");
        static LdrColor* color = ldr_color_repo::get_color(1);//todo save in config
        const glm::vec3 &col = color->value.asGlmVector();
        const ImVec4 &txtColor = getWhiteOrBlackBetterContrast(col);
        ImGui::PushStyleColor(ImGuiCol_Text, txtColor);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(col.x, col.y, col.z, 1));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(col.x, col.y, col.z, .8));

        static LdrColor* colorChosenInPopup;
        ImGui::SameLine();
        if (ImGui::Button(color->name.c_str())) {
            colorChosenInPopup = color;
            ImGui::OpenPopup("Part Palette Color");
        }
        ImGui::PopStyleColor(3);
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal("Part Palette Color", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            const auto buttonWidth = ImGui::GetFontSize() * 1.5f;
            const ImVec2 &buttonSize = ImVec2(buttonWidth, buttonWidth);
            const int columnCount = 20;

            for (const auto &colorGroup: ldr_color_repo::getAllColorsGroupedAndSortedByHue()) {
                if (ImGui::TreeNodeEx(colorGroup.first.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                    int i = 0;
                    for (const auto *currentColor : colorGroup.second) {
                        if (i % columnCount > 0) {
                            ImGui::SameLine();
                        }
                        ImGui::PushID(currentColor->code);
                        const ImColor imColor = ImColor(currentColor->value.red, currentColor->value.green, currentColor->value.blue);
                        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4) imColor);
                        if (ImGui::Button(colorChosenInPopup->code == currentColor->code ? "#" : "", buttonSize)) {
                            colorChosenInPopup = ldr_color_repo::get_color(currentColor->code);
                        }
                        ImGui::PopStyleColor(/*3*/1);
                        ImGui::PopID();
                        if (ImGui::IsItemHovered()) {
                            ImGui::BeginTooltip();
                            ImGui::Text("%s", currentColor->name.c_str());
                            ImGui::EndTooltip();
                        }
                        ++i;
                    }
                    ImGui::TreePop();
                }
            }
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Apply")) {
                color = colorChosenInPopup;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        static float categorySelectWidth = 250;//todo save
        const auto totalWidth = ImGui::GetContentRegionAvailWidth();
        const auto itemSpacingX = ImGui::GetStyle().ItemSpacing.x;
        float thumbnailContainerWidth = totalWidth - categorySelectWidth - itemSpacingX;
        //static const auto partsGrouped = ldr_file_repo::getPartsGroupedByCategory();
        static const auto partCategories = ldr_file_repo::getAllCategories();
        static std::set<std::string> selectedCategories = {*partCategories.begin()};//first category preselected

        ImGui::BeginChild("##categorySelectTree", ImVec2(categorySelectWidth, 0));
        for (const auto &category : partCategories) {
            int flags = selectedCategories.find(category) != selectedCategories.end()
                        ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Selected
                        : ImGuiTreeNodeFlags_Leaf;
            if (ImGui::TreeNodeEx(category.c_str(), flags)) {
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                    if (ImGui::GetIO().KeyCtrl) {
                        if (selectedCategories.find(category) == selectedCategories.end()) {
                            selectedCategories.insert(category);
                        } else {
                            selectedCategories.erase(category);
                        }
                    } else if (ImGui::GetIO().KeyShift) {
                        auto groupIt = partCategories.find(category);
                        while (groupIt != partCategories.begin() && selectedCategories.find(*groupIt) == selectedCategories.end()) {
                            selectedCategories.insert(*groupIt);
                            groupIt--;
                        }
                        selectedCategories.insert(*groupIt);
                    } else {
                        bool wasOnlySelectionBefore = selectedCategories.size() == 1 && *selectedCategories.begin() == category;
                        selectedCategories.clear();
                        if (!wasOnlySelectionBefore) {
                            selectedCategories.insert(category);
                        }
                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##thumbnailsContainer", ImVec2(thumbnailContainerWidth, 0), ImGuiWindowFlags_AlwaysVerticalScrollbar);
        const static auto thumbnailSpacing = 4;
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(thumbnailSpacing, thumbnailSpacing));
        int actualThumbSize = std::floor(controller::getThumbnailGenerator().size / 100.0 * thumbnailZoomPercent);
        auto actualThumbSizeSquared = ImVec2(actualThumbSize, actualThumbSize);
        int columns = std::max(1.0f, std::floor((ImGui::GetContentRegionAvailWidth() + thumbnailSpacing) / (actualThumbSize + thumbnailSpacing)));
        int currentCol = 0;

        if (selectedCategories.size() > 1) {
            for (const auto &category : selectedCategories) {
                ImGui::Text("%s", category.c_str());
                for (const auto &part : ldr_file_repo::getAllFilesOfCategory(category)) {
                    drawPartThumbnail(actualThumbSizeSquared, part, color);
                    currentCol++;
                    if (currentCol == columns) {
                        currentCol = 0;
                    } else {
                        ImGui::SameLine();
                    }
                }
                if (currentCol != 0) {
                    ImGui::NewLine();
                }
                currentCol = 0;
            }
        } else if (selectedCategories.size() == 1) {
            for (const auto &part : ldr_file_repo::getAllFilesOfCategory(*selectedCategories.begin())) {
                drawPartThumbnail(actualThumbSizeSquared, part, color);
                currentCol++;
                if (currentCol == columns) {
                    currentCol = 0;
                } else {
                    ImGui::SameLine();
                }
            }
        } else {
            for (const auto &category : ldr_file_repo::getPartsGroupedByCategory()) {
                ImGui::Text("%s", category.first.c_str());
                for (const auto &part : category.second) {
                    drawPartThumbnail(actualThumbSizeSquared, part, color);
                    currentCol++;
                    if (currentCol == columns) {
                        currentCol = 0;
                    } else {
                        ImGui::SameLine();
                    }
                }
                if (currentCol != 0) {
                    ImGui::NewLine();
                }
                currentCol = 0;
            }
        }
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::End();
    }

    if (showSettingsWindow) {
        ImGui::Begin("Settings", &showSettingsWindow);
        static auto guiScale = (float) (config::getDouble(config::GUI_SCALE));
        static int initialWindowSize[2]{
                static_cast<int>(config::getInt(config::SCREEN_WIDTH)),
                static_cast<int>(config::getInt(config::SCREEN_HEIGHT))
        };
        static auto ldrawDirString = config::getString(config::LDRAW_PARTS_LIBRARY);
        static auto ldrawDir = ldrawDirString.c_str();
        static auto guiStyleString = config::getString(config::GUI_STYLE);
        static auto guiStyle = guiStyleString == "light" ? 0 : (guiStyleString == "classic" ? 1 : 2);
        static int msaaSamples = (int) (config::getInt(config::MSAA_SAMPLES));
        static int msaaElem = std::log2(msaaSamples);
        static glm::vec3 backgroundColor = config::getColor(config::BACKGROUND_COLOR).asGlmVector();
        static glm::vec3 multiPartDocumentColor = config::getColor(config::COLOR_MULTI_PART_DOCUMENT).asGlmVector();
        static glm::vec3 mpdSubfileColor = config::getColor(config::COLOR_MPD_SUBFILE).asGlmVector();
        static glm::vec3 mpdSubfileInstanceColor = config::getColor(config::COLOR_MPD_SUBFILE_INSTANCE).asGlmVector();
        static glm::vec3 officalPartColor = config::getColor(config::COLOR_OFFICAL_PART).asGlmVector();
        static glm::vec3 unofficalPartColor = config::getColor(config::COLOR_UNOFFICAL_PART).asGlmVector();
        static bool displaySelectionBuffer = config::getBool(config::DISPLAY_SELECTION_BUFFER);
        static bool showNormals = config::getBool(config::SHOW_NORMALS);
        if (ImGui::TreeNode("General Settings")) {
            ImGui::SliderFloat("UI Scale", &guiScale, 0.25, 8, "%.2f");
            ImGui::InputInt2("Initial Window Size", initialWindowSize);
            ImGui::InputText("Ldraw path", const_cast<char *>(ldrawDir), 256);
            ImGui::Combo("GUI Theme", &guiStyle, "Light\0Classic\0Dark\0");
            ImGui::SliderInt("MSAA Samples", &msaaElem, 0, 4, std::to_string((int) std::pow(2, msaaElem)).c_str());
            ImGui::ColorEdit3("Background Color", &backgroundColor.x);
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Element Tree Settings")) {
            ImGui::ColorEdit3("Multi-Part Document Color", &multiPartDocumentColor.x);
            ImGui::ColorEdit3("MPD Subfile Color", &mpdSubfileColor.x);
            ImGui::ColorEdit3("MPD Subfile Instance Color", &mpdSubfileInstanceColor.x);
            ImGui::ColorEdit3("Offical Part Color", &officalPartColor.x);
            ImGui::ColorEdit3("Unoffical Part Color", &unofficalPartColor.x);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Debug Settings")) {
            ImGui::Checkbox("Display Selection Buffer", &displaySelectionBuffer);
            ImGui::Checkbox("Show Normals", &showNormals);
            ImGui::TreePop();
        }
        static bool saveFailed = false;
        if (ImGui::Button("Save")) {
            config::setDouble(config::GUI_SCALE, guiScale);
            config::setInt(config::SCREEN_WIDTH, initialWindowSize[0]);
            config::setInt(config::SCREEN_HEIGHT, initialWindowSize[1]);
            config::setString(config::LDRAW_PARTS_LIBRARY, ldrawDir);
            switch (guiStyle) {
                case 0:
                    config::setString(config::GUI_STYLE, "light");
                    break;
                case 1:
                    config::setString(config::GUI_STYLE, "classic");
                    break;
                default:
                    config::setString(config::GUI_STYLE, "dark");
                    break;
            }
            config::setInt(config::MSAA_SAMPLES, (int) std::pow(2, msaaElem));
            config::setColor(config::BACKGROUND_COLOR, util::RGBcolor(backgroundColor));
            config::setColor(config::COLOR_MULTI_PART_DOCUMENT, util::RGBcolor(multiPartDocumentColor));
            config::setColor(config::COLOR_MPD_SUBFILE, util::RGBcolor(mpdSubfileColor));
            config::setColor(config::COLOR_MPD_SUBFILE_INSTANCE, util::RGBcolor(mpdSubfileInstanceColor));
            config::setColor(config::COLOR_OFFICAL_PART, util::RGBcolor(officalPartColor));
            config::setColor(config::COLOR_UNOFFICAL_PART, util::RGBcolor(unofficalPartColor));
            config::setBool(config::DISPLAY_SELECTION_BUFFER, displaySelectionBuffer);
            config::setBool(config::SHOW_NORMALS, showNormals);
        }
        ImGui::End();
    }

    if (showAboutWindow) {
        if (ImGui::Begin("About", &showAboutWindow, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextWrapped("BrickSim is a program which should help you build digital brick models.");
            ImGui::TextWrapped("LEGO(R), the brick configuration, and the minifigure are trademarks of the LEGO Group, which does not sponsor, authorize or endorse this program.");
            ImGui::TextWrapped(
                    "This program comes without any warranty. Neihter the developers nor any other person shall have any liability to any person or entity with respect to any loss or damage caused or alleged to be caused directly or indirectly by this program.");
            ImGui::Separator();
            ImGui::Text("This program is open source. It's source code is available on GitHub under the following link:");
            draw_hyperlink_button("https://www.github.com/bb1950328/BrickSim");
            ImGui::TextWrapped("It's direct contributors have spent %.1f hours for this program. The following users have contributed:", git_stats::totalHours);
            ImGui::TextWrapped("%s", git_stats::contributorLoc);
            ImGui::Text("The numbers are the lines of code each contributer has committed. (Bigger number doesn't neccessarily mean more effort)");
            ImGui::TextWrapped("If you got this program from a source which is not listed on GitHub, please uninstall it and report is on GitHub (Create a Issue)");
            ImGui::TextWrapped("If find a bug, create a issue on GitHub where you describe the steps to reproduce as exact as possible so the developers can fix it.");
            ImGui::TextWrapped("You can also create an issue when you miss a feature or if you have an idea for improvement.");
            ImGui::TextWrapped("If you are a developer, contributing is very appreciated. You will find more information in the README.md");
            ImGui::TextWrapped("If you have a question or if you think you can contribute in another way (like writing manuals, designing icons or something like that), don't hesistate to create an issue");
            ImGui::Separator();
            ImGui::TextWrapped("This program wouldn't be possible without the LDraw Parts library. The shapes of all the parts in this programm come from the LDraw project. You will find more information on:");
            draw_hyperlink_button("https://www.ldraw.org");
            ImGui::TextWrapped("The graphical user interface is implemented using Dear ImGUI. More info at: ");
            draw_hyperlink_button("https://github.com/ocornut/imgui");
            ImGui::Separator();

            if (ImGui::Button("Close")) {
                showAboutWindow = false;
            }
            ImGui::End();
        }
    }

    if (showSysInfoWindow) {
        if (ImGui::Begin("System info", &showSysInfoWindow, ImGuiWindowFlags_AlwaysAutoResize)) {
            static const auto infoLines = util::getSystemInfo();
            for (const auto &line: infoLines) {
                ImGui::Text("%s", line.c_str());
            }
            if (ImGui::Button("Copy to clipboard")) {
                std::stringstream result;
                for (const auto &line: infoLines) {
                    result << line << std::endl;
                }
                glfwSetClipboardString(window, result.str().data());
            }
            ImGui::SameLine();
            if (ImGui::Button("Close")) {
                showSysInfoWindow = false;
            }
        }
        ImGui::End();
    }

    if (showDebugWindow) {
        ImGui::Begin("Debug Information", &showDebugWindow);
        long lastFrameTime = controller::getLastFrameTime();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", lastFrameTime / 1000.0, 1000000.0 / lastFrameTime);
        ImGui::Text("Total graphics buffer size: %s", util::formatBytesValue(statistic::vramUsageBytes).c_str());
        ImGui::Text("Total thumbnail buffer size: %s", util::formatBytesValue(statistic::thumbnailBufferUsageBytes).c_str());
        ImGui::Text("Last element tree reread: %.2f ms", statistic::lastElementTreeRereadMs);
        ImGui::Text("Last thumbnail render time: %.2f ms", statistic::lastThumbnailRenderingTimeMs);
        const auto &bgTasks = controller::getBackgroundTasks();
        if (!bgTasks.empty()) {
            ImGui::Text("%llu background tasks:", bgTasks.size());
            for (const auto &task : bgTasks) {
                ImGui::BulletText("%s", task.second->getTaskName().c_str());
            }
        }
        if (ImGui::Button("Reread element tree now")) {
            controller::setElementTreeChanged(true);
        }
        ImGui::End();
    }

    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }

    // Rendering
    ImGui::Render();
    {
        std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    lastScrollDeltaY = 0.0f;
}

void drawPartThumbnail(const ImVec2 &actualThumbSizeSquared, LdrFile *const &part, LdrColor *color) {
    bool realThumbnailAvailable = false;
    if (ImGui::IsRectVisible(actualThumbSizeSquared)) {
        auto optTexId = controller::getThumbnailGenerator().getThumbnailNonBlocking(part, color);
        if (optTexId.has_value()) {
            auto texId = (ImTextureID) (optTexId.value());
            ImGui::ImageButton(texId, actualThumbSizeSquared, ImVec2(0, 1), ImVec2(1, 0), 0);
            realThumbnailAvailable = true;
        }
    }
    if (!realThumbnailAvailable) {
        ImGui::Button(part->metaInfo.name.c_str(), actualThumbSizeSquared);
    }
    if (ImGui::IsItemHovered()) {
        auto availableColors = part_color_availability_provider::getAvailableColorsForPart(part);
        std::string availText;
        if (availableColors.has_value() && !availableColors.value().empty()) {
            if (availableColors.value().size() == 1) {
                availText = std::string("\nOnly available in ") + (*availableColors.value().begin())->name;
            } else {
                availText = std::string("\nAvailable in ") + std::to_string(availableColors.value().size()) + " Colors";
            }
        } else {
            availText = std::string("");
        }
        ImGui::SetTooltip("%s\n%s%s", part->metaInfo.title.c_str(), part->metaInfo.name.c_str(), availText.c_str());
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        controller::insertLdrElement(part);
    }
}

void Gui::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool Gui::loopPartsLibraryInstallationScreen() {
    static char state = 'A';
    /** States:
     * A show info
     * B Change path
     * D Download in progress
     * Z Finished
     */
    static std::atomic<float> downlaodPercent;
    static std::atomic<long long int> downloadBytes;
    //static std::thread downloadThread;//todo make this work
    static char pathBuffer[255];
    if (state == 'A') {
        if (ImGui::BeginPopupModal("ldraw library not found.", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {//todo this gives a segmentation fault because of some imgui id stack thing
            ImGui::Text("Currently, the path for the ldraw parts library is set to");
            auto parts_lib_raw = config::getString(config::LDRAW_PARTS_LIBRARY);
            auto parts_lib_extended = util::extendHomeDir(parts_lib_raw);
            ImGui::Text("'%s'", parts_lib_raw.c_str());
            if (parts_lib_extended != parts_lib_raw) {
                ImGui::Text("'~' is the users home directory, which currently is : '%s'", util::extendHomeDir("~").c_str());
            }
            ImGui::Text(" ");
            ImGui::Text("But this directory isn't recognized as a valid ldraw parts library.");
            ImGui::Text("Your options are:");
            ImGui::BulletText(" ");
            ImGui::SameLine();
            if (ImGui::Button("Change the path manually to point to your ldraw directory")) {
                state = 'B';
                strcpy(pathBuffer, parts_lib_raw.c_str());
            }
            ImGui::BulletText("Move the ldraw parts directory to the path above");
            ImGui::SameLine();
            if (ImGui::Button("Done##1")) {
                state = 'Z';
            }
            ImGui::BulletText("Download");
            ImGui::SameLine();
            draw_hyperlink_button("http://www.ldraw.org/library/updates/complete.zip");
            ImGui::SameLine();
            ImGui::Text("and unzip it to the path above");
            ImGui::SameLine();
            if (ImGui::Button("Done##2")) {
                state = 'Z';
            }
            ImGui::BulletText("Automatically download the parts library");
            ImGui::SameLine();
            if (ImGui::Button("Start")) {
                state = 'D';
            }
            ImGui::EndPopup();
        }
    } else if (state == 'B') {
        ImGui::InputText("ldraw parts directory path", pathBuffer, 255);
        ImGui::Text("'~' will be replaced with '%s' (the current home directory)", util::extendHomeDir("~").c_str());
        if (std::filesystem::exists(std::filesystem::path(pathBuffer))) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Good! Path exists.");
        } else {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "No! This path doesn't exist.");
        }
        if (ImGui::Button("Cancel")) {
            state = 'A';
        }
        ImGui::SameLine();
        if (ImGui::Button("OK")) {
            state = 'Z';
            config::setString(config::LDRAW_PARTS_LIBRARY, std::string(pathBuffer));
        }
    } else if (state == 'D') {
        //todo implement (start thread somehow)
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    const auto finished = state == 'Z';
    if (finished) {
        state = 'A';
    }
    return finished;
}

void Gui::drawWaitMessage(const std::string &message) const {
    if (setupDone) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        const float fontSize = ImGui::GetFontSize();
        ImGui::SetNextWindowSize(ImVec2(fontSize * 18, fontSize * 6));
        ImGui::Begin("Please wait", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("%c %s", "|/-\\"[(int)(glfwGetTime()*8) % 4], message.c_str());
        ImGui::End();

        ImGui::Render();
        {
            std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
    }
}

bool Gui::isSetupDone() const {
    return setupDone;
}
