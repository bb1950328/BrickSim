#include "gui.h"
#include "../config.h"
#include "../constant_data/resources.h"
#include "../controller.h"
#include "../helpers/parts_library_downloader.h"
#include "../lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include "../metrics.h"
#include "dialogs.h"
#include "gui_internal.h"
#include "main_menu_bar.h"
#include "stb_image_write.h"
#include "style.h"
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
        constexpr std::array<char const*, 1> ZIP_FILE_FILTER_PATTERNS = {"*.zip"};
        bool setupDone = false;
        GLFWwindow* window;
        double lastScrollDeltaY;

        std::unique_ptr<MouseCursorHandler> cursorHandler;

        std::shared_ptr<graphics::Texture> logoTexture;

        ImGuiID dockspaceId = 0;

        std::optional<windows::Id> currentlyFocusedWindow;
        std::optional<windows::Id> lastFocusedWindow;

        void addCustomIcons(ImGuiIO& io, ImFont* font) {
            plFunction();
            std::array<util::RawImage, icons::GLYPH_COUNT> rawIconImages;
            std::array<int, icons::GLYPH_COUNT> rectIds;

            for (const auto size: magic_enum::enum_values<icons::IconSize>()) {
                for (const auto type: magic_enum::enum_values<icons::IconType>()) {
                    const auto i = icons::getGlyphIndex(type, size);
                    rawIconImages[i] = icons::getRawImage(type, size);
                    rectIds[i] = io.Fonts->AddCustomRectFontGlyph(font, icons::getGlyphCodepoint(type, size), rawIconImages[i].width, rawIconImages[i].height, rawIconImages[i].width + 1);
                }
            }

            unsigned char* tex_pixels = nullptr;
            int tex_width, tex_height;
            {
                plScope("ImFontAtlas.GetTexDataAsRGBA32");
                io.Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_width, &tex_height);
            }

            for (std::size_t i = 0; i < icons::GLYPH_COUNT; ++i) {
                const auto* rect = io.Fonts->GetCustomRectByIndex(rectIds[i]);
                const auto& src = rawIconImages[i];
                assert(rect->Width == src.width && rect->Height == src.height && "icon resize not implemented");
                const auto* srcData = src.data.data();
                const auto srcLineByteSize = src.width * src.channels;
                for (int y = 0; y < rect->Height; y++) {
                    ImU32* const destLineStart = (ImU32*)tex_pixels + (rect->Y + y) * tex_width + (rect->X);
                    const auto* const srcLineStart = srcData + y * srcLineByteSize;
                    if (src.channels == 4) {
                        memcpy(destLineStart, srcLineStart, srcLineByteSize);
                    } else {
                        auto* d = destLineStart;
                        auto* s = srcLineStart;
                        if (src.channels == 1) {
                            for (int x = 0; x < rect->Width; x++) {
                                *d++ = IM_COL32(s[0], s[0], s[0], 255);
                                s += src.channels;
                            }
                        } else if (src.channels >= 3) {
                            for (int x = 0; x < rect->Width; x++) {
                                *d++ = IM_COL32(s[0], s[1], s[2], 255);
                                s += src.channels;
                            }
                        } else {
                            throw std::invalid_argument(fmt::format("cannot copy image for IconType::{} with {} channels", magic_enum::enum_name(magic_enum::enum_value<icons::IconType>(i)), src.channels));
                        }
                    }
                }
            }
        }
        void addFontawesomeFont(float scaleFactor, ImGuiIO& io) {
            plFunction();
            static const std::array<ImWchar, 3> icons_ranges = {ICON_MIN_FA, ICON_MAX_FA, 0};
            ImFontConfig iconsConfig;
            iconsConfig.MergeMode = true;
            iconsConfig.PixelSnapH = true;
            iconsConfig.FontDataOwnedByAtlas = false;
            io.Fonts->AddFontFromMemoryTTF((void*)resources::fonts::fa_solid_900_ttf.data(),
                                           resources::fonts::fa_solid_900_ttf.size(),
                                           13.f * scaleFactor,
                                           &iconsConfig,
                                           icons_ranges.data());
        }
        ImFont* createBaseFont(float scaleFactor, ImGuiIO& io) {
            plFunction();
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
            const auto font = io.Fonts->AddFontFromMemoryTTF((void*)fontData, static_cast<int>(fontDataLength), 13.f * scaleFactor, &fontConfig, nullptr);
            return font;
        }

        void setupFont(float scaleFactor, ImGuiIO& io) {
            plFunction();
            ImFont* font = createBaseFont(scaleFactor, io);

            addFontawesomeFont(scaleFactor, io);

            addCustomIcons(io, font);
        }

        void setupStyle() {
            auto guiStyle = config::get(config::GUI_STYLE);
            if (guiStyle == "BrickSim") {
                applyBrickSimImGuiStyle();
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
        plScope("gui::initialize");
        if (setupDone) {
            throw std::invalid_argument("setup called twice");
        }
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();//todo get the monitor on which the window is https://github.com/glfw/glfw/issues/1699
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

        cursorHandler = std::make_unique<MouseCursorHandler>();

        setupDone = true;
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
        drawMainMenuBar();

        dockspaceId = ImGui::DockSpaceOverViewport();

        windows::drawAll();

        lastScrollDeltaY = 0.0f;

        dialogs::drawDialogs(window);
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
        windows::cleanup();
        logoTexture = nullptr;
        cursorHandler = nullptr;
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
                auto enteredPath = std::filesystem::path(util::replaceSpecialPaths(pathBuffer));
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
                    config::set(config::LDRAW_PARTS_LIBRARY, util::replaceSpecialPaths(pathBuffer));
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
    MouseCursorHandler& getCursorHandler() {
        return *cursorHandler;
    }
}
