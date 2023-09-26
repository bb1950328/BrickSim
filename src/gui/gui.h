#pragma once

#include "../graphics/texture.h"
#include "windows/windows.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <magic_enum.hpp>
#include <string>

namespace bricksim {
    class Editor;
}

namespace bricksim::gui {
    namespace {
        void setupStyle();
        void setupFont(float scaleFactor, ImGuiIO& io);
    }

    struct UserQuestion {
        std::string title;
        std::string question;
        std::vector<std::string> options;
    };

    constexpr color::RGB COLOR_ACTIVE_EDITOR = {0, 255, 0};
    constexpr color::RGB COLOR_EDITING_MODEL = {0, 255, 255};

    void drawMenuBar(bool* show);

    void setWindow(GLFWwindow* value);
    GLFWwindow* getWindow();

    void setLastScrollDeltaY(double value);
    double getLastScrollDeltaY();

    bool areKeysCaptured();

    void initialize();
    [[nodiscard]] bool isSetupDone();

    void beginFrame();
    void drawMainWindows();
    void endFrame();

    void applyDefaultWindowLayout();

    void cleanup();

    enum class PartsLibrarySetupResponse {
        RUNNING,    //the function should be called again next time
        FINISHED,   //installation is finsi
        REQUEST_EXIT//the user wants to exit the application
    };

    PartsLibrarySetupResponse drawPartsLibrarySetupScreen();

    const std::shared_ptr<graphics::Texture>& getLogoTexture();

    void drawWaitMessage(const std::string& message, float progress);

    /**
     * This function must be called in each window somewhere between {@code ImGui::Begin()} and {@code ImGui::End()}
     * @param id the id of the current window
     */
    void collectWindowInfo(windows::Id id);

    [[nodiscard]] std::optional<windows::Id> getCurrentlyFocusedWindow();
}
