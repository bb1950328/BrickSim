#include "dialogs.h"
#include "../controller.h"
#include "../lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include "../user_actions.h"
#include "imgui.h"
#include "spdlog/spdlog.h"
#include "tinyfiledialogs.h"
namespace bricksim::gui::dialogs {
    constexpr std::array<char const*, 4> LDRAW_FILE_FILTER_PATTERNS = {"*.ldr", "*.dat", "*.mpd", "*.io"};
    constexpr std::array<char const*, 4> IMAGE_FILE_FILTER_PATTERNS = {"*.png", "*.jpg", "*.bmp", "*.tga"};

    bool openFindActionPopup = false;

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

    void drawDialogs(GLFWwindow* window) {
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
                    ImGui::Selectable(user_actions::getName(action).data(), selected);
                }
                ImGui::EndListBox();
            }
            bool close = false;
            if (ImGui::Button(ICON_FA_CHECK " OK") || glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
                user_actions::execute(selectedAction.value(), controller::getActiveEditor());
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
}
