#include "modals.h"
#include "../lib/IconFontCppHeaders/IconsFontAwesome5.h"

#include <imgui.h>
#include <utility>

namespace bricksim::gui::modals {

    Modal::Modal(std::string title, std::string message) :
        title(std::move(title)), message(std::move(message)) {}
    void Modal::open() {
        ImGui::OpenPopup(getFullWindowTitle().c_str());
    }
    std::string Modal::getFullWindowTitle() {
        return title;
    }
    bool Modal::draw() {
        bool showAgain = false;
        if (ImGui::BeginPopupModal(getFullWindowTitle().c_str(), nullptr, ImGuiWindowFlags_Modal)) {
            showAgain = drawContent();
            ImGui::EndPopup();
        }
        return showAgain;
    }

    ErrorModal::ErrorModal(std::string errorMessage) :
        Modal(ICON_FA_EXCLAMATION_CIRCLE " Error", std::move(errorMessage)) {
    }
    ErrorModal::ErrorModal(std::string title, std::string errorMessage) :
        Modal(std::move(title), std::move(errorMessage)) {
    }
    bool ErrorModal::drawContent() {
        //todo draw a big error icon on the left side
        ImGui::TextColored(color::RGB::RED, "%s", message.c_str());
        ImGui::PushItemWidth(-1);
        if (ImGui::Button("OK")) {
            return false;
        }
        return true;
    }
}