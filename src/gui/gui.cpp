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
#include "../resources.h"
#include "../lib/stb_image.h"
#include <atomic>
#include <imgui_internal.h>

namespace gui {
    namespace {
        char const * lFilterPatterns[NUM_LDR_FILTER_PATTERNS] = {"*.ldr", "*.dat", "*.mpd", "*.io"};
        char const * imageFilterPatterns[NUM_IMAGE_FILTER_PATTERNS] = {"*.png", "*.jpg", "*.bmp", "*.tga"};
        bool setupDone = false;
        GLFWwindow *window;
        double lastScrollDeltaY;

        std::string blockingMessageText;
        bool blockingMessageShowing = false;
        float blockingMessageProgress = 0;
    }

    void setupFont(float scaleFactor, ImGuiIO &io);

    void setup() {
        if (setupDone) {
            throw std::invalid_argument("setup called twice");
        }
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();//todo get the monitor on which the window is
        float xscale, yscale;
        glfwGetMonitorContentScale(monitor, &xscale, &yscale);
        std::cout << "xscale: " << xscale << "\tyscale: " << yscale << std::endl;
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto scaleFactor = (float) (config::getDouble(config::GUI_SCALE));
        if (xscale > 1 || yscale > 1) {
            scaleFactor *= (xscale + yscale) / 2.0f;
            ImGuiStyle &style = ImGui::GetStyle();
            style.ScaleAllSizes(scaleFactor);
            glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
        }

        //todo test which logo looks best on windows
        GLFWimage images[1];
        images[0].pixels = stbi_load_from_memory(resources::logo_square_nobg_png, resources::logo_square_nobg_png_len, &images[0].width, &images[0].height, nullptr, 4); //rgba channels
        glfwSetWindowIcon(window, 1, images);
        stbi_image_free(images[0].pixels);

        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        if (config::getBool(config::ENABLE_VIEWPORTS)) {
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        }

        setupFont(scaleFactor, io);

        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        // Setup Dear ImGui style
        setupStyle();
        setupDone = true;
    }

    void setupFont(float scaleFactor, ImGuiIO &io) {
        auto fontName = config::getString(config::FONT);
        const unsigned char* fontData;
        unsigned int fontDataLength;
        if (fontName=="Roboto") {
            fontData = resources::fonts_Roboto_Regular_ttf;
            fontDataLength = resources::fonts_Roboto_Regular_ttf_len;
        } else {
            if (fontName != "RobotoMono") {
                std::cout << "WARNING: Invalid font config: " << fontName;
            }
            fontData = resources::fonts_RobotoMono_Regular_ttf;
            fontDataLength = resources::fonts_RobotoMono_Regular_ttf_len;
        }
        ImFontConfig fontConfig;
        fontConfig.FontDataOwnedByAtlas = false;//otherwise ImGui tries to free() the data which causes a crash because the data is const
        io.Fonts->AddFontFromMemoryTTF((void *) fontData, fontDataLength, 13.0f * scaleFactor, &fontConfig, nullptr);

        // merge in icons from Font Awesome
        static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
        ImFontConfig iconsConfig;
        iconsConfig.MergeMode = true;
        iconsConfig.PixelSnapH = true;
        iconsConfig.FontDataOwnedByAtlas = false;
        io.Fonts->AddFontFromMemoryTTF((void *) resources::fonts_fa_solid_900_ttf, resources::fonts_fa_solid_900_ttf_len, 13.0f * scaleFactor, &iconsConfig, icons_ranges);
        //io.Fonts->AddFontFromFileTTF("resources/fonts/fa-solid-900.ttf", 13.0f * scaleFactor, &iconsConfig, icons_ranges);
    }

    void setupStyle() {
        auto guiStyle = config::getString(config::GUI_STYLE);
        if (guiStyle == "BrickSim") {
            ImVec4* colors = ImGui::GetStyle().Colors;
            colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
            colors[ImGuiCol_WindowBg]               = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
            colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
            colors[ImGuiCol_Border]                 = ImVec4(0.70f, 0.70f, 0.70f, 0.44f);
            colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_FrameBg]                = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
            colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
            colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
            colors[ImGuiCol_TitleBgActive]          = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
            colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
            colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
            colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
            colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
            colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
            colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
            colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
            colors[ImGuiCol_SliderGrab]             = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
            colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
            colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
            colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
            colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
            colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
            colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
            colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
            colors[ImGuiCol_Separator]              = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
            colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
            colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
            colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
            colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
            colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
            colors[ImGuiCol_Tab]                    = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
            colors[ImGuiCol_TabHovered]             = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
            colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
            colors[ImGuiCol_TabUnfocused]           = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
            colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
            colors[ImGuiCol_DockingPreview]         = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
            colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
            colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
            colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
            colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
            colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
            colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
            colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
            colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
            colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
            colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
            colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

            ImGui::GetStyle().FrameRounding = 8;
            ImGui::GetStyle().WindowRounding = 8;
        } else if (guiStyle == "ImGuiLight") {
            ImGui::StyleColorsLight();
        } else if (guiStyle == "ImGuiClassic") {
            ImGui::StyleColorsClassic();
        } else if (guiStyle == "ImGuiDark") {
            ImGui::StyleColorsDark();
        } else {
            std::cout << "WARNING: please set " << config::GUI_STYLE.name << "to BrickSim, ImGuiLight, ImGuiClassic or ImGuiDark" << std::endl;
        }
    }

    void drawMainWindows() {
        static bool show3dWindow = true;
        static bool showElementTreeWindow = true;
        static bool showElementPropertiesWindow = true;
        static bool showSettingsWindow = false;
        static bool showDemoWindow = true;
        static bool showDebugWindow = true;
        static bool showAboutWindow = false;
        static bool showSysInfoWindow = false;
        static bool showPartPaletteWindow = true;
        static bool showOrientationCube = true;

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN " Open", "CTRL+O")) {
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
                if (ImGui::MenuItem(ICON_FA_SIGN_OUT_ALT " Exit", "CTRL+W")) {
                    controller::setUserWantsToExit(true);
                }
                ImGui::MenuItem(ICON_FA_INFO_CIRCLE " About", "", &showAboutWindow);
                ImGui::MenuItem(ICON_FA_MICROCHIP " System Info", "", &showSysInfoWindow);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                //todo implement these functions
                if (ImGui::MenuItem(ICON_FA_UNDO" Undo", "CTRL+Z")) {}
                if (ImGui::MenuItem(ICON_FA_REDO" Redo", "CTRL+Y", false, false)) {}
                ImGui::Separator();
                if (ImGui::MenuItem(ICON_FA_CUT" Cut", "CTRL+X")) {}
                if (ImGui::MenuItem(ICON_FA_COPY" Copy", "CTRL+C")) {}
                if (ImGui::MenuItem(ICON_FA_PASTE" Paste", "CTRL+V")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem(ICON_FA_CUBES" 3D View", "ALT+V", &show3dWindow);
                ImGui::MenuItem(ICON_FA_CUBE" Orientation Cube", "", &showOrientationCube);
                ImGui::Separator();
                ImGui::MenuItem(ICON_FA_LIST" Element Tree", "ALT+T", &showElementTreeWindow);
                ImGui::MenuItem(ICON_FA_WRENCH" Element Properties", "ALT+P", &showElementPropertiesWindow);
                ImGui::Separator();
                ImGui::MenuItem(ICON_FA_TH" Part Palette", "ALT+N", &showPartPaletteWindow);
                ImGui::Separator();
                ImGui::MenuItem(ICON_FA_SLIDERS_H" Settings", "ALT+S", &showSettingsWindow);
                ImGui::Separator();
                ImGui::MenuItem(ICON_FA_IMAGE" ImGui Demo", "", &showDemoWindow);
                ImGui::MenuItem(ICON_FA_BUG" Debug", "ALT+D", &showDebugWindow);
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
                if (ImGui::MenuItem(ICON_FA_CAMERA" Screenshot", "CTRL+P")) {
                    char *fileNameChars = tinyfd_saveFileDialog(
                            ICON_FA_CAMERA" Save Screenshot",
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
                {ICON_FA_CUBES" 3D View", &show3dWindow, windows::draw3dWindow},
                {ICON_FA_LIST" Element Tree", &showElementTreeWindow, windows::drawElementTreeWindow},
                {ICON_FA_WRENCH" Element Properties", &showElementPropertiesWindow, windows::drawElementPropertiesWindow},
                {ICON_FA_TH" Part Palette", &showPartPaletteWindow, windows::drawPartPaletteWindow},
                {ICON_FA_SLIDERS_H" Settings", &showSettingsWindow, windows::drawSettingsWindow},
                {ICON_FA_INFO_CIRCLE " About", &showAboutWindow, windows::drawAboutWindow},
                {ICON_FA_MICROCHIP " System Info", &showSysInfoWindow, windows::drawSysInfoWindow},
                {ICON_FA_BUG" Debug", &showDebugWindow, windows::drawDebugWindow},
                {ICON_FA_IMAGE" ImGui Demo", &showDemoWindow, ImGui::ShowDemoWindow},
                {ICON_FA_CUBE" Orientation Cube", &showOrientationCube, windows::drawOrientationCube},
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
        lastScrollDeltaY = 0.0f;

        if (ImGui::BeginPopupModal("Please wait##Modal", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Please wait until this operation has finished.");
            ImGui::Separator();
            ImGui::Text("%s", blockingMessageText.c_str());
            ImGui::ProgressBar(blockingMessageProgress);
            if (!blockingMessageShowing) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void endFrame() {
        ImGui::Render();
        {
            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }
            std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
    }

    void beginFrame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }
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

    void drawWaitMessage(const std::string &message, float progress) {
        if (setupDone) {
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            const float fontSize = ImGui::GetFontSize();
            ImGui::SetNextWindowSize(ImVec2(fontSize * 18, fontSize * 6));
            ImGui::Begin("Please wait", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("%c %s", gui_internal::getLoFiSpinner(), message.c_str());
            ImGui::ProgressBar(progress);
            ImGui::End();
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

    void updateBlockingMessage(const std::string &message, float progress) {
        blockingMessageText = message;
        blockingMessageProgress = progress;
        if (!blockingMessageShowing) {
            ImGui::OpenPopup("Please wait##Modal");
            blockingMessageShowing = true;
        }
    }

    void closeBlockingMessage() {
        if (blockingMessageShowing) {
            blockingMessageShowing = false;
        }
    }
}