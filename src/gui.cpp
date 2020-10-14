//
// Created by Bader on 09.10.2020.
//

#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include <iostream>
#include "gui.h"
#include "config.h"
#include "controller.h"
#include "ldr_colors.h"
#include "lib/tinyfiledialogs.h"

void Gui::setup() {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();//todo get the monitor on which the window is
    float xscale, yscale;
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);
    std::cout << "xscale: " << xscale << "\tyscale: " << yscale << std::endl;
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto scaleFactor = (float) (config::get_double(config::KEY_GUI_SCALE));
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
    auto guiStyle = config::get_string(config::KEY_GUI_STYLE);
    if (guiStyle == "light") {
        ImGui::StyleColorsLight();
    } else if (guiStyle == "classic") {
        ImGui::StyleColorsClassic();
    } else if (guiStyle == "dark") {
        ImGui::StyleColorsDark();
    } else {
        std::cout << "WARNING: please set " << config::KEY_GUI_STYLE << "to light, classic or dark" << std::endl;
    }
}

void Gui::loop() {
    static bool show3dWindow = true;
    static bool showElementTreeWindow = true;
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
                char* fileNameChars = tinyfd_openFileDialog(
                        "Open File",
                        "",
                        3,
                        lFilterPatterns,
                        nullptr,
                        0);
                if (fileNameChars!= nullptr) {
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
            ImGui::MenuItem("Settings", "ALT+S", &showSettingsWindow);
            ImGui::Separator();
            ImGui::MenuItem("Demo", "", &showDemoWindow);
            ImGui::MenuItem("Debug", "ALT+D", &showDebugWindow);
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
            bool isInWindow = (windowPos.x+regionMin.x <= mousePos.x
                               && mousePos.x <= windowPos.x+regionMax.x
                               && windowPos.y+regionMin.y <= mousePos.y
                               && mousePos.y <= windowPos.y+regionMax.y);
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
                if (std::abs(lastScrollDeltaY)>0.01) {
                    controller->renderer.camera.moveForwardBackward((float)lastScrollDeltaY);
                    Controller::getInstance()->renderer.unrenderedChanges = true;
                }
            }
            auto texture3dView = (ImTextureID) controller->renderer.textureColorbuffer;
            ImGui::ImageButton(texture3dView, wsize, ImVec2(0, 1), ImVec2(1, 0), 0);
            ImGui::EndChild();
        }
        ImGui::End();
    }

    if (showElementTreeWindow) {
        ImGui::Begin("Element Tree", &showElementTreeWindow);
        ImGui::Text("TODO");//todo implement
        ImGui::End();
    }

    if (showSettingsWindow) {
        ImGui::Begin("Settings", &showSettingsWindow);
        static auto guiScale = (float) (config::get_double(config::KEY_GUI_SCALE));
        static int initialWindowSize[2]{
                static_cast<int>(config::get_long(config::KEY_SCREEN_WIDTH)),
                static_cast<int>(config::get_long(config::KEY_SCREEN_HEIGHT))
        };
        static auto ldrawDirString = config::get_string(config::KEY_LDRAW_PARTS_LIBRARY);
        static auto ldrawDir = ldrawDirString.c_str();
        static auto guiStyleString = config::get_string(config::KEY_GUI_STYLE);
        static auto guiStyle = guiStyleString == "light" ? 0 : (guiStyleString == "classic" ? 1 : 2);
        static int msaaSamples = (int)(config::get_long(config::KEY_MSAA_SAMPLES));
        static glm::vec3 backgroundColor = RGB(config::get_string(config::KEY_BACKGROUND_COLOR)).asGlmVector();
        ImGui::SliderFloat("UI Scale", &guiScale, 0.25, 8, "%.2f");
        ImGui::InputInt2("Initial Window Size", initialWindowSize);
        ImGui::InputText("Ldraw path", const_cast<char *>(ldrawDir), 256);
        ImGui::Combo("GUI Theme", &guiStyle, "Light\0Classic\0Dark\0");
        static int msaaElem = std::log2(msaaSamples);
        ImGui::SliderInt("MSAA Samples", &msaaElem, 0, 4, std::to_string((int)std::pow(2, msaaElem)).c_str());
        ImGui::ColorEdit3("Background Color", &backgroundColor.x);
        static bool saveFailed = false;
        if (ImGui::Button("Save")) {
            config::set_double(config::KEY_GUI_SCALE, guiScale);
            config::set_long(config::KEY_SCREEN_WIDTH, initialWindowSize[0]);
            config::set_long(config::KEY_SCREEN_HEIGHT, initialWindowSize[1]);
            config::set_string(config::KEY_LDRAW_PARTS_LIBRARY, ldrawDir);
            switch (guiStyle) {
                case 0:
                    config::set_string(config::KEY_GUI_STYLE, "light");
                    break;
                case 1:
                    config::set_string(config::KEY_GUI_STYLE, "classic");
                    break;
                default:
                    config::set_string(config::KEY_GUI_STYLE, "dark");
                    break;
            }
            config::set_long(config::KEY_MSAA_SAMPLES, (int)std::pow(2, msaaElem));
            config::set_string(config::KEY_BACKGROUND_COLOR, RGB(backgroundColor).asHtmlCode());
            saveFailed = !config::save();
        }
        if (saveFailed) {
            ImGui::OpenPopup("Error##saveFailed");
        }
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Error##saveFailed", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
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
