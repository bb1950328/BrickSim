//
// Created by bb1950328 on 09.10.2020.
//
#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>
#include "gui.h"
#include "../config.h"
#include "../controller.h"
#include "../lib/tinyfiledialogs.h"
#include "../info_providers/part_color_availability_provider.h"
#include "gui_internal.h"
#include <atomic>
#include <imgui_internal.h>

namespace gui {
    namespace {
        char const * lFilterPatterns[NUM_LDR_FILTER_PATTERNS] = {"*.ldr", "*.dat", "*.mpd"};
        char const * imageFilterPatterns[NUM_IMAGE_FILTER_PATTERNS] = {"*.png", "*.jpg", "*.bmp", "*.tga"};
        bool setupDone = false;
        GLFWwindow *window;
        double lastScrollDeltaY;


    }

    void setup() {
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

    void loop() {
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
                ImGui::TextDisabled("%llu Elements currently selected", controller::getSelectedNodes().size());
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

        static std::tuple<std::string, bool*, std::function<void(bool*)>> windowFuncsAndState[]{
                {"3D View", &show3dWindow, windows::draw3dWindow},
                {"Element Tree", &showElementTreeWindow, windows::drawElementTreeWindow},
                {"Element Properties", &showElementPropertiesWindow, windows::drawElementPropertiesWindow},
                {"Part Palette", &showPartPaletteWindow, windows::drawPartPaletteWindow},
                {"Settings", &showSettingsWindow, windows::drawSettingsWindow},
                {"About", &showAboutWindow, windows::drawAboutWindow},
                {"Sysinfo", &showSysInfoWindow, windows::drawSysInfoWindow},
                {"Debug", &showDebugWindow, windows::drawDebugWindow},
                {"Dear ImGui Demo", &showDemoWindow, ImGui::ShowDemoWindow},
        };

        std::vector<std::pair<std::string, float>> drawingTimes;
        for (const auto &winFuncState : windowFuncsAndState) {
            const auto& windowName = std::get<0>(winFuncState);
            const auto& windowState = std::get<1>(winFuncState);
            const auto& windowFunc = std::get<2>(winFuncState);

            if (*windowState) {
                auto before = std::chrono::high_resolution_clock::now();
                windowFunc(windowState);
                auto after = std::chrono::high_resolution_clock::now();
                drawingTimes.emplace_back(windowName, std::chrono::duration_cast<std::chrono::microseconds>(after - before).count() / 1000.0);
            }
        }
        statistic::lastWindowDrawingTimesMs = drawingTimes;

        // Rendering
        ImGui::Render();
        {
            std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
        lastScrollDeltaY = 0.0f;
    }

    void cleanup() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    bool loopPartsLibraryInstallationScreen() {
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
            if (ImGui::BeginPopupModal("ldraw library not found.", nullptr,
                                       ImGuiWindowFlags_AlwaysAutoResize)) {//todo this gives a segmentation fault because of some imgui id stack thing
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
                gui_internal::draw_hyperlink_button("http://www.ldraw.org/library/updates/complete.zip");
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

    void drawWaitMessage(const std::string &message) {
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
            ImGui::Text("%c %s", gui_internal::getLoFiSpinner(), message.c_str());
            ImGui::End();

            ImGui::Render();
            {
                std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }
        }
    }

    bool isSetupDone() {
        return setupDone;
    }

    void setWindow(GLFWwindow *value) {
        window = value;
    }

    GLFWwindow *getWindow() {
        return window;
    }

    void setLastScrollDeltaY(double value) {
        lastScrollDeltaY = value;
    }

    double getLastScrollDeltaY() {
        return lastScrollDeltaY;
    }
}