#include "gui.h"
#include "../config.h"
#include "../constant_data/resources.h"
#include "../controller.h"
#include "../graphics/texture.h"
#include "../helpers/parts_library_downloader.h"
#include "../helpers/stringutil.h"
#include "../helpers/util.h"
#include "../ldr/file_repo.h"
#include "../lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include "../metrics.h"
#include "gui_internal.h"
#include "windows/windows.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <imgui_internal.h>
#include <memory>
#include <palanteer.h>
#include <spdlog/spdlog.h>
#include <stb_image.h>
#include <tinyfiledialogs.h>

namespace bricksim::gui {
    namespace {
        constexpr std::array<char const*, 4> LDRAW_FILE_FILTER_PATTERNS = {"*.ldr", "*.dat", "*.mpd", "*.io"};
        constexpr std::array<char const*, 4> IMAGE_FILE_FILTER_PATTERNS = {"*.png", "*.jpg", "*.bmp", "*.tga"};
        constexpr std::array<char const*, 1> ZIP_FILE_FILTER_PATTERNS = {"*.zip"};
        bool setupDone = false;
        GLFWwindow* window;
        double lastScrollDeltaY;

        bool openFindActionPopup = false;

        std::shared_ptr<graphics::Texture> logoTexture;

        ImGuiID dockspaceId = 0;

        std::optional<windows::Id> currentlyFocusedWindow;
        std::optional<windows::Id> lastFocusedWindow;

        void setupFont(float scaleFactor, ImGuiIO& io) {
            auto fontName = config::get(config::FONT);
            const unsigned char* fontData;
            std::size_t fontDataLength;
            if (fontName == "Roboto") {
                fontData = resources::fonts::Roboto_Regular_ttf.data();
                fontDataLength = resources::fonts::Roboto_Regular_ttf.size();
            } else {
                if (fontName != "RobotoMono") {
                    spdlog::warn("invalid font config: \"{}\"", fontName);
                }
                fontData = resources::fonts::RobotoMono_Regular_ttf.data();
                fontDataLength = resources::fonts::RobotoMono_Regular_ttf.size();
            }
            ImFontConfig fontConfig;
            fontConfig.FontDataOwnedByAtlas = false;//otherwise ImGui tries to free() the data which causes a crash because the data is const
            io.Fonts->AddFontFromMemoryTTF((void*)fontData, static_cast<int>(fontDataLength), 13.f * scaleFactor, &fontConfig, nullptr);

            // merge in icons from Font Awesome
            static const std::array<ImWchar, 3> icons_ranges = {ICON_MIN_FA, ICON_MAX_FA, 0};
            ImFontConfig iconsConfig;
            iconsConfig.MergeMode = true;
            iconsConfig.PixelSnapH = true;
            iconsConfig.FontDataOwnedByAtlas = false;
            io.Fonts->AddFontFromMemoryTTF((void*)resources::fonts::fa_solid_900_ttf.data(), resources::fonts::fa_solid_900_ttf.size(),
                                           13.f * scaleFactor, &iconsConfig, icons_ranges.data());
        }

        void setupStyle() {
            auto guiStyle = config::get(config::GUI_STYLE);
            if (guiStyle == "BrickSim") {
                ImVec4* colors = ImGui::GetStyle().Colors;
                colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
                colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
                colors[ImGuiCol_WindowBg] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
                colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
                colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
                colors[ImGuiCol_Border] = ImVec4(0.70f, 0.70f, 0.70f, 0.44f);
                colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
                colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
                colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
                colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
                colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
                colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
                colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
                colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
                colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
                colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
                colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
                colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
                colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
                colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
                colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
                colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
                colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
                colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
                colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
                colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
                colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
                colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
                colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
                colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
                colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
                colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
                colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
                colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.35f, 0.58f, 0.86f);
                colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
                colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.41f, 0.68f, 1.00f);
                colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
                colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
                colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
                colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
                colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
                colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
                colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
                colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
                colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
                colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
                colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
                colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
                colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
                colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

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
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();//todo get the monitor on which the window is
        float xscale;
        float yscale;
        glfwGetMonitorContentScale(monitor, &xscale, &yscale);
        spdlog::info("xscale={}, yscale={}", xscale, yscale);
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto scaleFactor = config::get(config::GUI_SCALE);
        if (xscale > 1 || yscale > 1) {
            scaleFactor *= (xscale + yscale) / 2.0f;
            ImGuiStyle& style = ImGui::GetStyle();
            style.ScaleAllSizes(scaleFactor);
            glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
        }

        auto flipFlagBefore = util::isStbiFlipVertically();
        util::setStbiFlipVertically(false);
        std::array<GLFWimage, 1> images{};
        images[0].pixels = stbi_load_from_memory(resources::logos::logo_icon_png.data(), resources::logos::logo_icon_png.size(),
                                                 &images[0].width, &images[0].height, nullptr, 4);//rgba channels
        glfwSetWindowIcon(window, 1, images.data());
        stbi_image_free(images[0].pixels);

        util::setStbiFlipVertically(flipFlagBefore);

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        if (config::get(config::ENABLE_VIEWPORTS)) {
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        }

        setupFont(scaleFactor, io);

        // Setup Platform/Renderer bindings
        controller::executeOpenGL([]() {
            ImGui_ImplGlfw_InitForOpenGL(window, true);
            ImGui_ImplOpenGL3_Init("#version 330");
        });

        // Setup Dear ImGui style
        setupStyle();

        logoTexture = std::make_shared<graphics::Texture>(resources::logos::logo_fit_nobg_png.data(), resources::logos::logo_fit_nobg_png.size());

        setupDone = true;
    }

    void showOpenFileDialog() {
        char const* fileNameChars = tinyfd_openFileDialog(
                "Open File",
                "",
                LDRAW_FILE_FILTER_PATTERNS.size(),
                LDRAW_FILE_FILTER_PATTERNS.data(),
                nullptr,
                0);
        if (fileNameChars != nullptr) {
            std::string fileName(fileNameChars);
            controller::openFile(fileName);
        }
    }

    void showSaveFileAsDialog() {
        const auto& activeEditor = controller::getActiveEditor();
        if (activeEditor != nullptr) {
            showSaveFileAsDialog(activeEditor);
        } else {
            spdlog::warn("gui::showSaveFileAsDialog() called, but there's no active editor");
        }
    }
    void showSaveFileAsDialog(const std::shared_ptr<Editor>& editor) {
        std::string title = fmt::format("Save \"{}\" as", editor->getFilename());
        char const* fileNameChars = tinyfd_saveFileDialog(
                title.c_str(),
                "",
                LDRAW_FILE_FILTER_PATTERNS.size(),
                LDRAW_FILE_FILTER_PATTERNS.data(),
                nullptr);
        if (fileNameChars != nullptr) {
            std::string fileName(fileNameChars);
            editor->saveAs(fileName);
        }
    }

    void showSaveCopyAsDialog() {
        const auto& activeEditor = controller::getActiveEditor();
        if (activeEditor != nullptr) {
            showSaveCopyAsDialog(activeEditor);
        } else {
            spdlog::warn("gui::showSaveCopyAsDialog() called, but there's no active editor");
        }
    }
    void showSaveCopyAsDialog(const std::shared_ptr<Editor>& editor) {
        std::string title = fmt::format("Save copy of \"{}\"", editor->getFilename());
        char const* fileNameChars = tinyfd_saveFileDialog(
                "Save Copy As",
                "",
                LDRAW_FILE_FILTER_PATTERNS.size(),
                LDRAW_FILE_FILTER_PATTERNS.data(),
                nullptr);
        if (fileNameChars != nullptr) {
            std::string fileName(fileNameChars);
            editor->saveCopyAs(fileName);
        }
    }

    void showScreenshotDialog() {
        const auto& activeEditor = controller::getActiveEditor();
        if (activeEditor != nullptr) {
            showScreenshotDialog(activeEditor);
        } else {
            spdlog::warn("gui::showScreenshotDialog() called, but there's no active editor");
        }
    }
    void showScreenshotDialog(const std::shared_ptr<Editor>& editor) {
        std::string title = fmt::format("Save Screenshot of \"{}\"", editor->getFilename());
        char const* fileNameChars = tinyfd_saveFileDialog(
                title.c_str(),
                "",
                IMAGE_FILE_FILTER_PATTERNS.size(),
                IMAGE_FILE_FILTER_PATTERNS.data(),
                nullptr);
        if (fileNameChars != nullptr) {
            std::string fileNameString(fileNameChars);
            editor->getScene()->getImage().saveImage(fileNameString);
        }
    }

    void showExecuteActionByNameDialog() {
        openFindActionPopup = true;
    }

    void applyDefaultWindowLayout() {
        spdlog::debug("applying default window layout");
        ImGui::DockBuilderRemoveNodeChildNodes(dockspaceId);

        ImGuiID level0left;
        ImGuiID level0right;
        ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Right, .4f, &level0right, &level0left);

        ImGuiID level1rightTop;
        ImGuiID level1rightBottom;
        ImGui::DockBuilderSplitNode(level0right, ImGuiDir_Down, .6f, &level1rightBottom, &level1rightTop);

        ImGuiID level2rightTopLeft;
        ImGuiID level2rightTopRight;
        ImGui::DockBuilderSplitNode(level1rightTop, ImGuiDir_Left, .4f, &level2rightTopLeft, &level2rightTopRight);

        ImGui::DockBuilderDockWindow(windows::getName(windows::Id::ORIENTATION_CUBE), level2rightTopLeft);
        ImGui::DockBuilderDockWindow(windows::getName(windows::Id::DEBUG), level2rightTopRight);
        ImGui::DockBuilderDockWindow(windows::getName(windows::Id::ELEMENT_PROPERTIES), level1rightBottom);

        ImGuiID level1leftBottom;
        ImGuiID level1leftTop;
        ImGui::DockBuilderSplitNode(level0left, ImGuiDir_Down, .4f, &level1leftBottom, &level1leftTop);

        ImGuiID level2leftTopLeft;
        ImGuiID level2leftTopRight;
        ImGui::DockBuilderSplitNode(level1leftTop, ImGuiDir_Left, .3f, &level2leftTopLeft, &level2leftTopRight);

        ImGui::DockBuilderDockWindow(windows::getName(windows::Id::PART_PALETTE), level1leftBottom);
        ImGui::DockBuilderDockWindow(windows::getName(windows::Id::ELEMENT_TREE), level2leftTopLeft);
        ImGui::DockBuilderDockWindow(windows::getName(windows::Id::VIEW_3D), level2leftTopRight);
    }

    void drawMainWindows() {
        plFunction();
        lastFocusedWindow = currentlyFocusedWindow;
        currentlyFocusedWindow = {};
        if (ImGui::BeginMainMenuBar()) {
            //todo class MenuBarHandler, function for each menu
            if (ImGui::BeginMenu("File")) {
                gui_internal::actionMenuItem(user_actions::NEW_FILE);
                gui_internal::actionMenuItem(user_actions::OPEN_FILE);
                gui_internal::actionMenuItem(user_actions::SAVE_FILE);
                gui_internal::actionMenuItem(user_actions::SAVE_FILE_AS);
                gui_internal::actionMenuItem(user_actions::SAVE_COPY_AS);

                ImGui::Separator();

                gui_internal::actionMenuItem(user_actions::EXIT);

                ImGui::MenuItem(ICON_FA_CIRCLE_INFO " About", "", windows::isVisible(windows::Id::ABOUT));
                ImGui::MenuItem(ICON_FA_MICROCHIP " System Info", "", windows::isVisible(windows::Id::SYSTEM_INFO));
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                gui_internal::actionMenuItem(user_actions::UNDO);
                gui_internal::actionMenuItem(user_actions::REDO);
                ImGui::Separator();
                gui_internal::actionMenuItem(user_actions::CUT);
                gui_internal::actionMenuItem(user_actions::COPY);
                gui_internal::actionMenuItem(user_actions::PASTE);
                ImGui::Separator();
                gui_internal::actionMenuItem(user_actions::INLINE_SELECTED_ELEMENTS);
                ImGui::EndMenu();
            }
            auto& editors = controller::getEditors();
            if (editors.size() == 1) {
                if (ImGui::BeginMenu("Document")) {
                    drawDocumentMenu(*editors.begin());
                    ImGui::EndMenu();
                }
            } else {
                if (ImGui::BeginMenu("Documents")) {
                    for (const auto& editor: editors) {
                        if (ImGui::BeginMenu(editor->getFilename().c_str())) {
                            drawDocumentMenu(editor);
                            ImGui::EndMenu();
                        }
                    }
                }
            }

            if (ImGui::BeginMenu("View")) {
                gui_internal::windowMenuItem(windows::Id::VIEW_3D);
                gui_internal::windowMenuItem(windows::Id::ORIENTATION_CUBE);
                ImGui::Separator();
                gui_internal::windowMenuItem(windows::Id::ELEMENT_TREE);
                gui_internal::windowMenuItem(windows::Id::ELEMENT_PROPERTIES);
                ImGui::Separator();
                gui_internal::windowMenuItem(windows::Id::PART_PALETTE);
                gui_internal::windowMenuItem(windows::Id::LDRAW_FILE_INSPECTOR);
                ImGui::Separator();
                gui_internal::windowMenuItem(windows::Id::MODEL_INFO);
                gui_internal::windowMenuItem(windows::Id::EDITOR_META_INFO);
                ImGui::Separator();
                gui_internal::windowMenuItem(windows::Id::SETTINGS);
                ImGui::Separator();
                gui_internal::windowMenuItem(windows::Id::IMGUI_DEMO);
                gui_internal::windowMenuItem(windows::Id::DEBUG);
                gui_internal::windowMenuItem(windows::Id::LOG);
                ImGui::Separator();
                if (ImGui::MenuItem("Apply default window layout")) {
                    applyDefaultWindowLayout();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Snap")) {
                auto& handler = controller::getSnapHandler();
                ImGui::MenuItem("Enabled", nullptr, handler.isEnabledPtr());
                if (ImGui::BeginMenu(ICON_FA_RULER_COMBINED " Linear snap steps")) {
                    auto& linearHandler = handler.getLinear();
                    const auto& presets = linearHandler.getPresets();
                    for (int i = 0; i < presets.size(); ++i) {
                        const auto txt = fmt::format("{} XZ={} Y={}", presets[i].name, presets[i].stepXZ, presets[i].stepY);
                        if (ImGui::MenuItem(txt.c_str(), nullptr, linearHandler.getCurrentPresetIndex() == i)) {
                            linearHandler.setCurrentPresetIndex(i);
                        }
                    }
                    const auto txt = fmt::format("Custom XZ={} Y={}", linearHandler.getTemporaryPreset().stepXZ, linearHandler.getTemporaryPreset().stepY);
                    if (ImGui::MenuItem(txt.c_str(), nullptr, linearHandler.getCurrentPresetIndex() == snap::LinearHandler::TEMPORARY_PRESET_INDEX)) {
                        linearHandler.setCurrentPresetIndex(snap::LinearHandler::TEMPORARY_PRESET_INDEX);
                    }
                    if (ImGui::BeginMenu(ICON_FA_PENCIL " Edit Custom")) {
                        static int xz;
                        static int y;
                        auto& tmpPreset = linearHandler.getTemporaryPreset();
                        xz = tmpPreset.stepXZ;
                        y = tmpPreset.stepY;
                        if (ImGui::InputInt("XZ", &xz, 1, 20) | ImGui::InputInt("Y", &y, 1, 20)) {
                            linearHandler.setTemporaryPreset({"",
                                                              std::clamp(xz, 1, 20000),
                                                              std::clamp(y, 1, 20000)});
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Selection")) {
                gui_internal::actionMenuItem(user_actions::SELECT_ALL);
                gui_internal::actionMenuItem(user_actions::SELECT_NOTHING);
                gui_internal::actionMenuItem(user_actions::SELECT_CONNECTED);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("3D")) {
                gui_internal::actionMenuItem(user_actions::VIEW_3D_FRONT, "Front");
                gui_internal::actionMenuItem(user_actions::VIEW_3D_TOP, "Top");
                gui_internal::actionMenuItem(user_actions::VIEW_3D_RIGHT, "Right");
                gui_internal::actionMenuItem(user_actions::VIEW_3D_REAR, "Rear");
                gui_internal::actionMenuItem(user_actions::VIEW_3D_BOTTOM, "Bottom");
                gui_internal::actionMenuItem(user_actions::VIEW_3D_LEFT, "Left");
                ImGui::Separator();
                const auto activeEditor = controller::getActiveEditor();
                if (activeEditor == nullptr) {
                    ImGui::BeginDisabled();
                    ImGui::MenuItem("Show Surfaces", "");
                    ImGui::MenuItem("Show Lines", "");
                    ImGui::EndDisabled();
                } else {
                    const auto mainScene = activeEditor->getScene();
                    ImGui::MenuItem("Show Surfaces", "", mainScene->isDrawTriangles());
                    ImGui::MenuItem("Show Lines", "", mainScene->isDrawLines());
                }
                gui_internal::actionMenuItem(user_actions::TAKE_SCREENSHOT);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Tools")) {
                ImGui::MenuItem(windows::getName(windows::Id::GEAR_RATIO_CALCULATOR), "", windows::isVisible(windows::Id::GEAR_RATIO_CALCULATOR));
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                gui_internal::actionMenuItem(user_actions::EXECUTE_ACTION_BY_NAME);
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        dockspaceId = ImGui::DockSpaceOverViewport();

        windows::drawAll();

        lastScrollDeltaY = 0.0f;

        if (openFindActionPopup) {
            ImGui::OpenPopup("Execute Action by Name");
            openFindActionPopup = false;
        }
        if (ImGui::BeginPopupModal("Execute Action by Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            static std::string searchBuf(48, '\0');
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)) {
                ImGui::SetKeyboardFocusHere(0);
            }
            ImGui::InputText(ICON_FA_MAGNIFYING_GLASS, searchBuf.data(), searchBuf.capacity());

            static std::optional<user_actions::Action> selectedAction;
            if (ImGui::BeginListBox("##actionsByNameListBox")) {
                auto& foundActions = user_actions::findActionsByName(searchBuf);
                static uint8_t btnUpPressed = 0;//0=not pressed, 1=pressed, handle id, >=2=pressed, already handled
                static uint8_t btnDownPressed = 0;
                if (btnUpPressed == 0 && glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
                    btnUpPressed = 1;
                } else if (btnUpPressed == 1) {
                    btnUpPressed = 2;
                } else if (btnUpPressed == 2 && glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE) {
                    btnUpPressed = 0;
                }
                if (btnDownPressed == 0 && glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
                    btnDownPressed = 1;
                } else if (btnDownPressed == 1) {
                    btnDownPressed = 2;
                } else if (btnDownPressed == 2 && glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE) {
                    btnDownPressed = 0;
                }
                int moveDelta = 0;
                if (btnUpPressed == 1) {
                    if (!selectedAction.has_value() || *foundActions.cbegin() == selectedAction.value()) {
                        selectedAction = *foundActions.cend();
                    } else {
                        moveDelta = -1;
                    }
                } else if (btnDownPressed == 1) {
                    if (!selectedAction.has_value() || *foundActions.cend() == selectedAction.value()) {
                        selectedAction = *foundActions.cbegin();
                    } else {
                        moveDelta = 1;
                    }
                }
                if (moveDelta != 0) {
                    auto it = foundActions.begin();
                    while (it != foundActions.end()) {
                        if (*it == selectedAction.value()) {
                            if (moveDelta == -1) {
                                --it;
                            } else {
                                ++it;
                            }
                            selectedAction = *it;
                            break;
                        }
                        ++it;
                    }
                }
                if (!std::any_of(foundActions.cbegin(), foundActions.cend(), [](const auto& action) { return action == selectedAction.value(); })) {
                    selectedAction = *foundActions.cbegin();
                }
                for (const auto& action: foundActions) {
                    bool selected = action == selectedAction;
                    if (selected) {
                        ImGui::SetScrollHereY();
                    }
                    ImGui::Selectable(user_actions::getName(action), selected);
                }
                ImGui::EndListBox();
            }
            bool close = false;
            if (ImGui::Button(ICON_FA_CHECK " OK") || glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
                user_actions::execute(selectedAction.value());
                close = true;
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_RECTANGLE_XMARK " Cancel") || close || glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                searchBuf[0] = '\0';
                selectedAction = std::nullopt;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void drawDocumentMenu(const std::shared_ptr<Editor>& editor) {
        if (ImGui::MenuItem(ICON_FA_FLOPPY_DISK " Save")) {
            editor->save();
        }
        if (ImGui::MenuItem(ICON_FA_FLOPPY_DISK " Save as")) {
            showSaveFileAsDialog(editor);
        }
        if (ImGui::MenuItem(ICON_FA_COPY " Save copy as")) {
            showSaveCopyAsDialog(editor);
        }
        if (ImGui::MenuItem(ICON_FA_CAMERA " Save screenshot")) {
            showScreenshotDialog(editor);
        }
    }

    void beginFrame() {
        plFunction();
        controller::executeOpenGL([]() {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        });
    }

    void endFrame() {
        plFunction();
        ImGui::Render();
        {
            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }
            controller::executeOpenGL([]() {
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            });
        }
    }

    void cleanup() {
        logoTexture = nullptr;
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
        static std::thread downloadThread;//todo make this work
        static std::string pathBuffer(1023, '\0');
        static auto windowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        if (state == 'A') {
            if (ImGui::Begin(ICON_FA_TRIANGLE_EXCLAMATION " LDraw library not found.", nullptr, windowFlags)) {
                auto parts_lib_raw = config::get(config::LDRAW_PARTS_LIBRARY);
                auto parts_lib_extended = util::extendHomeDir(parts_lib_raw);

                ImGui::Text("Currently, the path for the ldraw parts library is set to \"%s\"", parts_lib_raw.c_str());
                if (parts_lib_extended != parts_lib_raw) {
                    ImGui::TextDisabled("'~' is the users home directory, which currently is : '%s'", util::extendHomeDir("~").c_str());
                }
                ImGui::Text(" ");
                ImGui::Text("But this directory isn't recognized as a valid ldraw parts library.");
                ImGui::Text("Your options are:");
                ImGui::Bullet();
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_PEN " Change the path manually to point to your ldraw directory")) {
                    state = 'B';
                    pathBuffer.assign(parts_lib_raw);
                }
                ImGui::BulletText("Move the ldraw parts directory to the path above");
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_CHECK " Done##1")) {
                    state = 'Z';
                }
                ImGui::BulletText("Download");
                ImGui::SameLine();
                gui_internal::drawHyperlinkButton(constants::LDRAW_LIBRARY_DOWNLOAD_URL);
                ImGui::SameLine();
                ImGui::Text("and unzip it to the path above");
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_CHECK " Done##2")) {
                    state = 'Z';
                }
                ImGui::BulletText("Automatically download the parts library");
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_DOWNLOAD " Start")) {
                    state = 'D';
                }
            }
            ImGui::End();
        } else if (state == 'B') {
            if (ImGui::Begin("Set LDraw parts library path", nullptr, windowFlags)) {
                ImGui::InputText("LDraw parts directory or zip path", pathBuffer.data(), pathBuffer.capacity());
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_FOLDER_OPEN)) {
                    char const* folderNameChars = tinyfd_selectFolderDialog("Select LDraw parts library folder", pathBuffer.c_str());
                    pathBuffer.assign(folderNameChars);
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_FILE_ZIPPER)) {
                    char const* fileNameChars = tinyfd_openFileDialog(
                            "Select LDraw parts library .zip",
                            pathBuffer.c_str(),
                            ZIP_FILE_FILTER_PATTERNS.size(),
                            ZIP_FILE_FILTER_PATTERNS.data(),
                            "LDraw parts library",
                            0);
                    pathBuffer.assign(fileNameChars);
                }
                //todo make button for file dialog
                ImGui::TextDisabled("'~' will be replaced with '%s' (the current home directory)", util::extendHomeDir("~").c_str());
                auto enteredPath = std::filesystem::path(util::extendHomeDirPath(pathBuffer));
                static std::filesystem::path lastCheckedPath;
                static ldr::file_repo::LibraryType libraryType;
                if (lastCheckedPath != enteredPath) {
                    lastCheckedPath = enteredPath;
                    libraryType = ldr::file_repo::getLibraryType(enteredPath);
                }
                if (libraryType == ldr::file_repo::LibraryType::INVALID) {
                    ImGui::TextColored(color::RED, ICON_FA_CIRCLE_XMARK " This path doesn't exist or isn't a valid LDraw parts library");
                } else if (libraryType == ldr::file_repo::LibraryType::DIRECTORY) {
                    ImGui::TextColored(color::GREEN, ICON_FA_CHECK "This is a valid path to an LDraw parts library directory.");
                } else {
                    ImGui::TextColored(color::GREEN, ICON_FA_CHECK " This is a valid path to an LDraw parts library zip.");
                }
                if (ImGui::Button(ICON_FA_BAN " Cancel")) {
                    state = 'A';
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_CIRCLE_CHECK " OK")) {
                    state = 'Z';
                    config::set(config::LDRAW_PARTS_LIBRARY, util::replaceHomeDir(pathBuffer));
                }
            }
            ImGui::End();
        } else if (state == 'D') {
            if (ImGui::Begin(ICON_FA_DOWNLOAD " Downloading LDraw parts library", nullptr, windowFlags)) {
                switch (parts_library_downloader::getStatus()) {
                    case parts_library_downloader::Status::DOING_NOTHING:
                        downloadThread = std::thread(parts_library_downloader::downloadPartsLibrary);
                        break;
                    case parts_library_downloader::Status::IN_PROGRESS: {
                        auto [downNow, downTotal] = parts_library_downloader::getProgress();
                        ImGui::Text(ICON_FA_DOWNLOAD " Downloading ldraw parts library...");

                        const float progressFraction = static_cast<float>(downNow) / static_cast<float>(downTotal);
                        const auto bytesPerSecondTxt = stringutil::formatBytesValue(parts_library_downloader::getSpeedBytesPerSecond());
                        const auto speedTxt = fmt::format("{:.1f}% {}/s", progressFraction * 100, bytesPerSecondTxt);
                        ImGui::ProgressBar(progressFraction, ImVec2(-FLT_MIN, 0), speedTxt.c_str());
                        if (ImGui::Button(ICON_FA_CIRCLE_STOP " Cancel and exit program")) {
                            parts_library_downloader::stopDownload();
                            downloadThread.join();
                            ImGui::End();
                            return PartsLibrarySetupResponse::REQUEST_EXIT;
                        }
                        break;
                    }
                    case parts_library_downloader::Status::FAILED:
                        ImGui::TextColored(color::RED, ICON_FA_CIRCLE_XMARK " Download failed with error code %d", parts_library_downloader::getErrorCode());
                        if (ImGui::Button(ICON_FA_CHEVRON_LEFT " Back")) {
                            parts_library_downloader::reset();
                            state = 'Z';
                        }
                        break;
                    case parts_library_downloader::Status::FINISHED:
                        state = 'Z';
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
        return finished ? PartsLibrarySetupResponse::FINISHED : PartsLibrarySetupResponse::RUNNING;
    }

    void drawWaitMessage(const std::string& message, float progress) {
        if (setupDone) {
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            //const float fontSize = ImGui::GetFontSize();
            //ImGui::SetNextWindowSize(ImVec2(fontSize * 18, fontSize * 6));
            ImGui::Begin("Please wait", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Image(gui_internal::convertTextureId(logoTexture->getID()),
                         ImVec2(static_cast<float>(logoTexture->getSize().x), static_cast<float>(logoTexture->getSize().y)),
                         ImVec2(0, 1), ImVec2(1, 0));
            ImGui::Text("%s %s", gui_internal::getAnimatedHourglassIcon(), message.c_str());
            ImGui::ProgressBar(progress);
            ImGui::End();
        }
    }

    bool isSetupDone() {
        return setupDone;
    }

    void setWindow(GLFWwindow* value) {
        window = value;
    }

    GLFWwindow* getWindow() {
        return window;
    }

    void setLastScrollDeltaY(double value) {
        lastScrollDeltaY = value;
    }

    double getLastScrollDeltaY() {
        return lastScrollDeltaY;
    }

    bool areKeysCaptured() {
        return ImGui::GetIO().WantCaptureKeyboard;
    }
    const std::shared_ptr<graphics::Texture>& getLogoTexture() {
        return logoTexture;
    }
    void collectWindowInfo(windows::Id id) {
        if (ImGui::IsWindowFocused(ImGuiHoveredFlags_ChildWindows)) {
            currentlyFocusedWindow = id;
        }
    }
    std::optional<windows::Id> getCurrentlyFocusedWindow() {
        return currentlyFocusedWindow.has_value() ? currentlyFocusedWindow : lastFocusedWindow;
    }
}
