//
// Created by bb1950328 on 09.10.2020.
//

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>
#include "gui.h"
#include "config.h"
#include "controller.h"
#include "ldr_colors.h"
#include "lib/tinyfiledialogs.h"
#include "util.h"
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>

void Gui::setup() {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();//todo get the monitor on which the window is
    float xscale, yscale;
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);
    std::cout << "xscale: " << xscale << "\tyscale: " << yscale << std::endl;
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto scaleFactor = (float) (config::get_double(config::GUI_SCALE));
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
    auto guiStyle = config::get_string(config::GUI_STYLE);
    if (guiStyle == "light") {
        ImGui::StyleColorsLight();
    } else if (guiStyle == "classic") {
        ImGui::StyleColorsClassic();
    } else if (guiStyle == "dark") {
        ImGui::StyleColorsDark();
    } else {
        std::cout << "WARNING: please set " << config::GUI_STYLE.name << "to light, classic or dark" << std::endl;
    }
}

void drawColorGroup(Controller *controller,
                    ElementTreeLdrNode *ldrNode,
                    const ImVec2 &buttonSize,
                    const int columnCount,
                    const std::pair<const std::string, std::vector<const LdrColor *>> &colorGroup) {
    //todo make palette look prettier
    //todo show only colors which are available for this part (get the data somewhere)
    if (ImGui::TreeNodeEx(colorGroup.first.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
        int i = 0;
        for (const auto *color : colorGroup.second) {
            if (i%columnCount>0){
                ImGui::SameLine();
            }
            ImGui::PushID(color->code);
            const ImColor imColor = ImColor(color->value.red, color->value.green, color->value.blue);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)imColor);
            if (ImGui::Button(ldrNode->color->code == color->code ? "#" : "", buttonSize)) {
                ldrNode->color = LdrColorRepository::getInstance()->get_color(color->code);
                controller->elementTreeChanged = true;
            }
            ImGui::PopStyleColor(/*3*/1);
            ImGui::PopID();
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("%s", color->name.c_str());
                ImGui::EndTooltip();
            }
            ++i;
        }
        ImGui::TreePop();
    }
}

void draw_element_tree_node(ElementTreeNode *node) {
    bool itemClicked = false;
    if (node->children.empty()) {
        auto flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (node->selected) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        ImGui::TreeNodeEx(node->displayName.c_str(), flags);
        itemClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
        //todo add context menu
    } else {
        auto flags = node->selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None;
        if (ImGui::TreeNodeEx(node->displayName.c_str(), flags)) {
            itemClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
            for (const auto &child: node->children) {
                draw_element_tree_node(child);
            }
            ImGui::TreePop();
        }
    }
    if (itemClicked) {
        auto controller = Controller::getInstance();
        if (ImGui::GetIO().KeyCtrl) {
            controller->nodeSelectAddRemove(node);
        } else if (ImGui::GetIO().KeyShift) {
            controller->nodeSelectUntil(node);
        } else {
            controller->nodeSelectSet(node);
        }
    }
}

void Gui::loop() {
    static bool show3dWindow = true;
    static bool showElementTreeWindow = true;
    static bool showElementPropertiesWindow = true;
    static bool showSettingsWindow = true;
    static bool showDemoWindow = true;
    static bool showDebugWindow = true;
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.6f, 1.0f);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    auto *controller = Controller::getInstance();
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "CTRL+O")) {
                char *fileNameChars = tinyfd_openFileDialog(
                        "Open File",
                        "",
                        3,
                        lFilterPatterns,
                        nullptr,
                        0);
                if (fileNameChars != nullptr) {
                    std::string fileName(fileNameChars);
                    controller->openFile(fileName);
                }
            }
            if (ImGui::MenuItem("Exit", "CTRL+W")) {
                controller->userWantsToExit = true;
            }
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
            ImGui::MenuItem("Settings", "ALT+S", &showSettingsWindow);
            ImGui::Separator();
            ImGui::MenuItem("Demo", "", &showDemoWindow);
            ImGui::MenuItem("Debug", "ALT+D", &showDebugWindow);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Selection")) {
            if (ImGui::MenuItem("Select All", "CTRL+A")) {
                controller->nodeSelectAll();
            }
            if (ImGui::MenuItem("Select Nothing", "CTRL+U")) {
                controller->nodeSelectNone();
            }
            ImGui::TextDisabled("%lu Elements currently selected", controller->selectedNodes.size());
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("3D")) {
            if (ImGui::MenuItem("Front", "ALT+1")) controller->setStandard3dView(1);
            if (ImGui::MenuItem("Top", "ALT+2")) controller->setStandard3dView(2);
            if (ImGui::MenuItem("Right", "ALT+3")) controller->setStandard3dView(3);
            if (ImGui::MenuItem("Rear", "ALT+4")) controller->setStandard3dView(4);
            if (ImGui::MenuItem("Bottom", "ALT+5")) controller->setStandard3dView(5);
            if (ImGui::MenuItem("Left", "ALT+6")) controller->setStandard3dView(6);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (show3dWindow) {
        ImGui::Begin("3D View", &show3dWindow, ImGuiWindowFlags_NoScrollWithMouse);
        {
            ImGui::BeginChild("3DRender");
            ImVec2 wsize = ImGui::GetContentRegionAvail();
            controller->set3dViewSize((unsigned int) wsize.x, (unsigned int) wsize.y);
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
            //std::cout << ImGui::GetScrollX() << "\t" << ImGui::GetScrollY() << std::endl;
            if (isInWindow && ImGui::IsWindowFocused()) {
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                    const ImVec2 &leftBtnDrag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                    controller->renderer.camera.mouseRotate(leftBtnDrag.x - lastDeltaXleft,
                                                            (leftBtnDrag.y - lastDeltaYleft) * -1);
                    controller->renderer.unrenderedChanges = true;
                    lastDeltaXleft = leftBtnDrag.x;
                    lastDeltaYleft = leftBtnDrag.y;
                } else {
                    lastDeltaXleft = 0;
                    lastDeltaYleft = 0;
                }
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
                    const ImVec2 &rightBtnDrag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
                    controller->renderer.camera.mousePan(rightBtnDrag.x - lastDeltaXright,
                                                         (rightBtnDrag.y - lastDeltaYright) * -1);
                    controller->renderer.unrenderedChanges = true;
                    lastDeltaXright = rightBtnDrag.x;
                    lastDeltaYright = rightBtnDrag.y;
                } else {
                    lastDeltaXright = 0;
                    lastDeltaYright = 0;
                }
                if (std::abs(lastScrollDeltaY) > 0.01) {
                    controller->renderer.camera.moveForwardBackward((float) lastScrollDeltaY);
                    Controller::getInstance()->renderer.unrenderedChanges = true;
                }
            }
            auto texture3dView = (ImTextureID) controller->renderer.imageTextureColorbuffer;
            ImGui::ImageButton(texture3dView, wsize, ImVec2(0, 1), ImVec2(1, 0), 0);
            ImGui::EndChild();
        }
        ImGui::End();
    }

    if (showElementTreeWindow) {
        ImGui::Begin("Element Tree", &showElementTreeWindow);
        for (auto *rootChild : controller->elementTree.rootNode.children) {
            draw_element_tree_node(rootChild);
        }
        ImGui::End();
    }

    if (showElementPropertiesWindow) {
        ImGui::Begin("Element Properties", &showElementPropertiesWindow);
        if (controller->selectedNodes.empty()) {
            ImGui::Text("Select an element to view its properties here");
        } else if (controller->selectedNodes.size() == 1) {
            auto node = *controller->selectedNodes.begin();

            static char displayNameBuf[255];
            static ElementTreeNode *lastNode = nullptr;
            if (nullptr != lastNode) {
                lastNode->displayName = std::string(displayNameBuf);
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
            if (lastNode != node) {
                inputEulerAnglesDeg = treeEulerAnglesRad * (float) (180.0f / M_PI);
                inputPosition = treePosition;
                inputScalePercent = treeScale * 100.0f;
            }
            glm::vec3 inputEulerAnglesRad = inputEulerAnglesDeg * (float) (M_PI / 180.0f);

            if ((util::biggest_value(glm::abs(treePosition - inputPosition)) > 0.01)
                || (util::biggest_value(glm::abs(inputEulerAnglesRad - treeEulerAnglesRad)) > 0.0001)
                || (util::biggest_value(glm::abs(inputScalePercent / 100.0f - treeScale)) > 0.001)) {
                auto newRotation = glm::eulerAngleXYZ(inputEulerAnglesRad.x, inputEulerAnglesRad.y,
                                                      inputEulerAnglesRad.z);
                auto newTranslation = glm::translate(glm::mat4(1.0f), inputPosition);
                auto newScale = glm::scale(glm::mat4(1.0f), inputScalePercent / 100.0f);
                auto newTransformation = newTranslation * newRotation * newScale;
                if (treeRelTransf != newTransformation) {
                    node->setRelativeTransformation(glm::transpose(newTransformation));
                    controller->elementTreeChanged = true;
                }
            }

            ImGui::DragFloat3("Rotation", &inputEulerAnglesDeg[0], 1.0f, -180, 180, "%.1f°");
            ImGui::DragFloat3("Position", &inputPosition[0], 1.0f, -1e9, 1e9, "%.0fLDU");
            ImGui::DragFloat3("Scale", &inputScalePercent[0], 1.0f, -1e9, 1e9, "%.2f%%");

            if ((node->getType()&ElementTreeNodeType::ET_TYPE_LDRFILE)>0) {
                auto* ldrNode = dynamic_cast<ElementTreeLdrNode *>(node);
                if (ImGui::TreeNodeEx("Color", ImGuiTreeNodeFlags_DefaultOpen)) {
                    const auto buttonWidth = ImGui::GetFontSize() * 1.5f;
                    const ImVec2 &buttonSize = ImVec2(buttonWidth, buttonWidth);
                    const int columnCount = std::floor(ImGui::GetContentRegionAvailWidth() / (buttonWidth+ImGui::GetStyle().ItemSpacing.x));
                    const auto &groupedAndSortedByHue = LdrColorRepository::getInstance()->getAllColorsGroupedAndSortedByHue();
                    const static std::vector<std::string> fixed_pos = {"Solid", "Transparent", "Rubber"};
                    for (const auto &colorName : fixed_pos) {
                        const auto &colorGroup = std::make_pair(colorName, groupedAndSortedByHue.find(colorName)->second);
                        drawColorGroup(controller, ldrNode, buttonSize, columnCount, colorGroup);
                    }
                    for (const auto &colorGroup : groupedAndSortedByHue) {
                        bool alreadyDrawn = false;//todo google how to vector.contains()
                        for (const auto &groupName : fixed_pos) {
                            if (groupName==colorGroup.first) {
                                alreadyDrawn = true;
                                break;
                            }
                        }

                        if (!alreadyDrawn) {
                            drawColorGroup(controller, ldrNode, buttonSize, columnCount, colorGroup);
                        }
                    }
                    ImGui::TreePop();
                }
            }

            lastNode = node;
        } else {
            ImGui::Text("Multi-select currently not supported here");
        }
        ImGui::End();
    }

    if (showSettingsWindow) {
        ImGui::Begin("Settings", &showSettingsWindow);
        static auto guiScale = (float) (config::get_double(config::GUI_SCALE));
        static int initialWindowSize[2]{
                static_cast<int>(config::get_long(config::SCREEN_WIDTH)),
                static_cast<int>(config::get_long(config::SCREEN_HEIGHT))
        };
        static auto ldrawDirString = config::get_string(config::LDRAW_PARTS_LIBRARY);
        static auto ldrawDir = ldrawDirString.c_str();
        static auto guiStyleString = config::get_string(config::GUI_STYLE);
        static auto guiStyle = guiStyleString == "light" ? 0 : (guiStyleString == "classic" ? 1 : 2);
        static int msaaSamples = (int) (config::get_long(config::MSAA_SAMPLES));
        static glm::vec3 backgroundColor = util::RGB(config::get_string(config::BACKGROUND_COLOR)).asGlmVector();
        ImGui::SliderFloat("UI Scale", &guiScale, 0.25, 8, "%.2f");
        ImGui::InputInt2("Initial Window Size", initialWindowSize);
        ImGui::InputText("Ldraw path", const_cast<char *>(ldrawDir), 256);
        ImGui::Combo("GUI Theme", &guiStyle, "Light\0Classic\0Dark\0");
        static int msaaElem = std::log2(msaaSamples);
        ImGui::SliderInt("MSAA Samples", &msaaElem, 0, 4, std::to_string((int) std::pow(2, msaaElem)).c_str());
        ImGui::ColorEdit3("Background Color", &backgroundColor.x);
        static bool saveFailed = false;
        if (ImGui::Button("Save")) {
            config::set_double(config::GUI_SCALE, guiScale);
            config::set_long(config::SCREEN_WIDTH, initialWindowSize[0]);
            config::set_long(config::SCREEN_HEIGHT, initialWindowSize[1]);
            config::set_string(config::LDRAW_PARTS_LIBRARY, ldrawDir);
            switch (guiStyle) {
                case 0:
                    config::set_string(config::GUI_STYLE, "light");
                    break;
                case 1:
                    config::set_string(config::GUI_STYLE, "classic");
                    break;
                default:
                    config::set_string(config::GUI_STYLE, "dark");
                    break;
            }
            config::set_long(config::MSAA_SAMPLES, (int) std::pow(2, msaaElem));
            config::set_string(config::BACKGROUND_COLOR, util::RGB(backgroundColor).asHtmlCode());
            saveFailed = !config::save();
        }
        if (saveFailed) {
            ImGui::OpenPopup("Error##saveFailed");
        }
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Error##saveFailed", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Failed to save settings. ");
            ImGui::Text("Please check if you have write access to the config file location");
            ImGui::Separator();

            if (ImGui::Button("OK")) {
                saveFailed = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::EndPopup();
        }
        ImGui::End();
    }

    if (showDebugWindow) {
        ImGui::Begin("Debug Information", &showDebugWindow);
        long lastFrameTime = controller->lastFrameTime;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", lastFrameTime / 1000.0, 1000000.0 / lastFrameTime);
        ImGui::End();
    }

    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    lastScrollDeltaY = 0.0f;
}

void Gui::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
