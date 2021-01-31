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
#include "../constant_data/resources.h"
#include "../lib/stb_image.h"
#include <atomic>
#include <imgui_internal.h>
#include <spdlog/spdlog.h>
#include "../helpers/platform_detection.h"
#include "../helpers/parts_library_downloader.h"

namespace gui {
    const char* WINDOW_NAME_3D_VIEW = ICON_FA_CUBES" 3D View";
    const char* WINDOW_NAME_ELEMENT_TREE = ICON_FA_LIST" Element Tree";
    const char* WINDOW_NAME_ELEMENT_PROPERTIES = ICON_FA_WRENCH" Element Properties";
    const char* WINDOW_NAME_PART_PALETTE = ICON_FA_TH" Part Palette";
    const char* WINDOW_NAME_SETTINGS = ICON_FA_SLIDERS_H" Settings";
    const char* WINDOW_NAME_ABOUT = ICON_FA_INFO_CIRCLE " About";
    const char* WINDOW_NAME_SYSTEM_INFO = ICON_FA_MICROCHIP " System Info";
    const char* WINDOW_NAME_DEBUG = ICON_FA_BUG" Debug";
    const char* WINDOW_NAME_IMGUI_DEMO = ICON_FA_IMAGE" ImGui Demo";
    const char* WINDOW_NAME_ORIENTATION_CUBE = ICON_FA_CUBE" Orientation Cube";
    const char* WINDOW_NAME_LOG = ICON_FA_LIST" Log";

    namespace {
        bool show3dWindow = true;
        bool showElementTreeWindow = true;
        bool showElementPropertiesWindow = true;
        bool showSettingsWindow = false;
        bool showDemoWindow = false;
        bool showDebugWindow = true;
        bool showAboutWindow = false;
        bool showSysInfoWindow = false;
        bool showPartPaletteWindow = true;
        bool showOrientationCube = true;
        bool showLogWindow = false;

        char const * lFilterPatterns[NUM_LDR_FILTER_PATTERNS] = {"*.ldr", "*.dat", "*.mpd", "*.io"};
        char const * imageFilterPatterns[NUM_IMAGE_FILTER_PATTERNS] = {"*.png", "*.jpg", "*.bmp", "*.tga"};
        char const * zipFilterPatterns[NUM_ZIP_FILTER_PATTERNS] = {"*.zip"};
        bool setupDone = false;
        GLFWwindow *window;
        double lastScrollDeltaY;

        std::string blockingMessageText;
        bool blockingMessageShowing = false;
        float blockingMessageProgress = 0;

        util::TextureLoadResult logoTexture;

        ImGuiID dockspaceId = 0;

        void applyDefaultWindowLayout() {
            spdlog::debug("applying default window layout");
            ImGui::DockBuilderRemoveNodeChildNodes(dockspaceId);

            show3dWindow = true;
            showElementTreeWindow = true;
            showElementPropertiesWindow = true;
            showSettingsWindow = false;
            showDemoWindow = false;
            showDebugWindow = true;
            showAboutWindow = false;
            showSysInfoWindow = false;
            showPartPaletteWindow = true;
            showOrientationCube = true;
            showLogWindow = false;

            ImGuiID level0left, level0right;
            ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Right, 0.4, &level0right, &level0left);

            ImGuiID level1rightTop, level1rightBottom;
            ImGui::DockBuilderSplitNode(level0right, ImGuiDir_Down, 0.6, &level1rightBottom, &level1rightTop);

            ImGuiID level2rightTopLeft, level2rightTopRight;
            ImGui::DockBuilderSplitNode(level1rightTop, ImGuiDir_Left, 0.4, &level2rightTopLeft, &level2rightTopRight);

            ImGui::DockBuilderDockWindow(WINDOW_NAME_ORIENTATION_CUBE, level2rightTopLeft);
            ImGui::DockBuilderDockWindow(WINDOW_NAME_DEBUG, level2rightTopRight);
            ImGui::DockBuilderDockWindow(WINDOW_NAME_ELEMENT_PROPERTIES, level1rightBottom);


            ImGuiID level1leftBottom, level1leftTop;
            ImGui::DockBuilderSplitNode(level0left, ImGuiDir_Down, 0.4, &level1leftBottom, &level1leftTop);

            ImGuiID level2leftTopLeft, level2leftTopRight;
            ImGui::DockBuilderSplitNode(level1leftTop, ImGuiDir_Left, 0.3, &level2leftTopLeft, &level2leftTopRight);

            ImGui::DockBuilderDockWindow(WINDOW_NAME_PART_PALETTE, level1leftBottom);
            ImGui::DockBuilderDockWindow(WINDOW_NAME_ELEMENT_TREE, level2leftTopLeft);
            ImGui::DockBuilderDockWindow(WINDOW_NAME_3D_VIEW, level2leftTopRight);
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
                    spdlog::warn("invalid font config: \"{}\"", fontName);
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
                spdlog::warn("please set {} to BrickSim, ImGuiLight, ImGuiClassic or ImGuiDark (currently set to \"{}\"", config::GUI_STYLE.name, guiStyle);
            }
        }
    }

    void initialize() {
        if (setupDone) {
            throw std::invalid_argument("setup called twice");
        }
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();//todo get the monitor on which the window is
        float xscale, yscale;
        glfwGetMonitorContentScale(monitor, &xscale, &yscale);
        spdlog::info("xscale={}, yscale={}", xscale, yscale);
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

        auto flipFlagBefore = util::isStbiFlipVertically();
        util::setStbiFlipVertically(false);
        GLFWimage images[1];
        images[0].pixels = stbi_load_from_memory(resources::logo_icon_png, resources::logo_icon_png_len, &images[0].width, &images[0].height, nullptr, 4); //rgba channels
        glfwSetWindowIcon(window, 1, images);
        stbi_image_free(images[0].pixels);

        util::setStbiFlipVertically(flipFlagBefore);

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

        logoTexture = util::loadTextureFromMemory(resources::logo_fit_nobg_png, resources::logo_fit_nobg_png_len);

        setupDone = true;
    }

    void drawMainWindows() {
        static std::tuple<std::string, bool *, std::function<void(bool *)>> windowFuncsAndState[]{
                {WINDOW_NAME_3D_VIEW,             &show3dWindow,                windows::draw3dWindow},
                {WINDOW_NAME_ELEMENT_TREE,        &showElementTreeWindow,       windows::drawElementTreeWindow},
                {WINDOW_NAME_ELEMENT_PROPERTIES,  &showElementPropertiesWindow, windows::drawElementPropertiesWindow},
                {WINDOW_NAME_PART_PALETTE,        &showPartPaletteWindow,       windows::drawPartPaletteWindow},
                {WINDOW_NAME_SETTINGS,            &showSettingsWindow,          windows::drawSettingsWindow},
                {WINDOW_NAME_ABOUT,               &showAboutWindow,             windows::drawAboutWindow},
                {WINDOW_NAME_SYSTEM_INFO,         &showSysInfoWindow,           windows::drawSysInfoWindow},
                {WINDOW_NAME_DEBUG,               &showDebugWindow,             windows::drawDebugWindow},
                {WINDOW_NAME_IMGUI_DEMO,          &showDemoWindow,              ImGui::ShowDemoWindow},
                {WINDOW_NAME_ORIENTATION_CUBE,    &showOrientationCube,         windows::drawOrientationCube},
                {WINDOW_NAME_LOG,                 &showLogWindow,               windows::drawLogWindow},
        };

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
                ImGui::MenuItem(WINDOW_NAME_3D_VIEW, "ALT+V", &show3dWindow);
                ImGui::MenuItem(WINDOW_NAME_ORIENTATION_CUBE, "", &showOrientationCube);
                ImGui::Separator();
                ImGui::MenuItem(WINDOW_NAME_ELEMENT_TREE, "ALT+T", &showElementTreeWindow);
                ImGui::MenuItem(WINDOW_NAME_ELEMENT_PROPERTIES, "ALT+P", &showElementPropertiesWindow);
                ImGui::Separator();
                ImGui::MenuItem(WINDOW_NAME_PART_PALETTE, "ALT+N", &showPartPaletteWindow);
                ImGui::Separator();
                ImGui::MenuItem(WINDOW_NAME_SETTINGS, "ALT+S", &showSettingsWindow);
                ImGui::Separator();
                ImGui::MenuItem(WINDOW_NAME_IMGUI_DEMO, "", &showDemoWindow);
                ImGui::MenuItem(WINDOW_NAME_DEBUG, "ALT+D", &showDebugWindow);
                ImGui::MenuItem(WINDOW_NAME_LOG, nullptr, &showLogWindow);
                ImGui::Separator();
                if (ImGui::MenuItem("Apply default window layout")) {
                    applyDefaultWindowLayout();
                }
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

        dockspaceId = ImGui::DockSpaceOverViewport();

        std::vector<std::pair<std::string, float>> drawingTimes;
        for (const auto &winFuncState : windowFuncsAndState) {
            const auto &windowName = std::get<0>(winFuncState);
            const auto &windowState = std::get<1>(winFuncState);
            const auto &windowFunc = std::get<2>(winFuncState);

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
            ImGui::Image(gui_internal::convertTextureId(logoTexture.textureId), ImVec2(logoTexture.width, logoTexture.height), ImVec2(0, 1), ImVec2(1, 0));
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
        static bool firstTime = true;
        if(firstTime) {
            std::lock_guard<std::recursive_mutex> lg(controller::getOpenGlMutex());
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            firstTime = false;
        } else {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }
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

    PartsLibrarySetupResponse drawPartsLibrarySetupScreen() {
        static char state = 'A';
        /** States:
         * A show info
         * B Change path
         * D Download in progress
         * Z Finished
         */
        static long bytesDownloaded = 0, bytesTotal = 0;
        static std::thread downloadThread;//todo make this work
        static char pathBuffer[1023];
        static auto windowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        if (state == 'A') {
            if (ImGui::Begin(ICON_FA_EXCLAMATION_TRIANGLE" LDraw library not found.", nullptr, windowFlags)) {
                auto parts_lib_raw = config::getString(config::LDRAW_PARTS_LIBRARY);
                auto parts_lib_extended = util::extendHomeDir(parts_lib_raw);

                ImGui::Text("Currently, the path for the ldraw parts library is set to \"%s\"", parts_lib_raw.c_str());
                if (parts_lib_extended != parts_lib_raw) {
                    ImGui::TextDisabled("'~' is the users home directory, which currently is : '%s'", util::extendHomeDir("~").c_str());
                }
                ImGui::Text(" ");
                ImGui::Text("But this directory isn't recognized as a valid ldraw parts library.");
                ImGui::Text("Your options are:");
                //ImGui::BulletText(" ");
                ImGui::Bullet();
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_EDIT" Change the path manually to point to your ldraw directory")) {
                    state = 'B';
                    strcpy(pathBuffer, parts_lib_raw.c_str());
                }
                ImGui::BulletText("Move the ldraw parts directory to the path above");
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_CHECK" Done##1")) {
                    state = 'Z';
                }
                ImGui::BulletText("Download");
                ImGui::SameLine();
                gui_internal::draw_hyperlink_button("http://www.ldraw.org/library/updates/complete.zip");
                ImGui::SameLine();
                ImGui::Text("and unzip it to the path above");
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_CHECK" Done##2")) {
                    state = 'Z';
                }
                ImGui::BulletText("Automatically download the parts library");
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_DOWNLOAD" Start")) {
                    state = 'D';
                }
            }
            ImGui::End();
        } else if (state == 'B') {
            if (ImGui::Begin("Set LDraw parts library path", nullptr, windowFlags)) {
                ImGui::InputText("LDraw parts directory or zip path", pathBuffer, sizeof(pathBuffer));
                ImGui::SameLine();
                if(ImGui::Button(ICON_FA_FOLDER_OPEN)) {
                    char *folderNameChars = tinyfd_selectFolderDialog("Select LDraw parts library folder", pathBuffer);
                    std::strcpy(pathBuffer, folderNameChars);
                }
                ImGui::SameLine();
                if(ImGui::Button(ICON_FA_FILE_ARCHIVE)) {
                    char *fileNameChars = tinyfd_openFileDialog(
                            "Select LDraw parts library .zip",
                            pathBuffer,
                            NUM_ZIP_FILTER_PATTERNS,
                            zipFilterPatterns,
                            "LDraw parts library",
                            0);
                    std::strcpy(pathBuffer, fileNameChars);
                }
                //todo make button for file dialog
                ImGui::TextDisabled("'~' will be replaced with '%s' (the current home directory)", util::extendHomeDir("~").c_str());
                auto enteredPath = std::filesystem::path(util::extendHomeDirPath(pathBuffer));
                static std::filesystem::path lastCheckedPath;
                static ldr_file_repo::LibraryType libraryType;
                if (lastCheckedPath!=enteredPath) {
                    lastCheckedPath = enteredPath;
                    libraryType = ldr_file_repo::checkLibraryValid(enteredPath);
                }
                if (libraryType == ldr_file_repo::LibraryType::NOT_FOUND) {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), ICON_FA_TIMES_CIRCLE" This path doesn't exist or isn't a valid LDraw parts library");
                } else if (libraryType==ldr_file_repo::LibraryType::DIRECTORY){
                    ImGui::TextColored(ImVec4(0, 1, 0, 1), ICON_FA_CHECK"This is a valid path to an LDraw parts library directory.");
                } else {
                    ImGui::TextColored(ImVec4(0, 1, 0, 1), ICON_FA_CHECK" This is a valid path to an LDraw parts library zip.");
                }
                if (ImGui::Button(ICON_FA_BAN" Cancel")) {
                    state = 'A';
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_CHECK_CIRCLE" OK")) {
                    state = 'Z';
                    config::setString(config::LDRAW_PARTS_LIBRARY, util::replaceHomeDir(pathBuffer));
                }
            }
            ImGui::End();
        } else if (state == 'D') {
            if (ImGui::Begin(ICON_FA_DOWNLOAD" Downloading LDraw parts library", nullptr, windowFlags)) {
                switch (parts_library_downloader::getStatus()) {
                    case parts_library_downloader::DOING_NOTHING:downloadThread = std::thread(parts_library_downloader::downloadPartsLibrary);
                        break;
                    case parts_library_downloader::IN_PROGRESS: {
                        auto progress = parts_library_downloader::getProgress();
                        ImGui::Text(ICON_FA_DOWNLOAD" Downloading ldraw parts library...");

                        float progressFraction = 1.0f * progress.first / progress.second;
                        std::string speedTxt = std::to_string(progressFraction) + "%" + util::formatBytesValue(parts_library_downloader::getSpeedBytesPerSecond())+"/s";
                        ImGui::ProgressBar(progressFraction, ImVec2(-FLT_MIN, 0), speedTxt.c_str());
                        if (ImGui::Button(ICON_FA_STOP_CIRCLE" Cancel and exit program")) {
                            parts_library_downloader::stopDownload();
                            downloadThread.join();
                            ImGui::End();
                            return REQUEST_EXIT;
                        }
                        break;
                    }
                    case parts_library_downloader::FAILED:
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), ICON_FA_TIMES_CIRCLE" Download failed with error code %d", parts_library_downloader::getErrorCode());
                        if (ImGui::Button(ICON_FA_CHEVRON_LEFT" Back")) {
                            parts_library_downloader::reset();
                            state = 'Z';
                        }
                        break;
                    case parts_library_downloader::FINISHED: state = 'Z';
                        parts_library_downloader::reset();
                        break;
                }
            }
            ImGui::End();
        }
        const auto finished = state == 'Z';
        if (finished) {
            state = 'A';
        }
        return finished?FINISHED:RUNNING;
    }

    void drawWaitMessage(const std::string &message, float progress) {
        if (setupDone) {
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            //const float fontSize = ImGui::GetFontSize();
            //ImGui::SetNextWindowSize(ImVec2(fontSize * 18, fontSize * 6));
            ImGui::Begin("Please wait", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Image(gui_internal::convertTextureId(logoTexture.textureId), ImVec2(logoTexture.width, logoTexture.height), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::Text("%s %s", gui_internal::getAnimatedHourglassIcon(), message.c_str());
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