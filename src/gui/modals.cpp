#include "modals.h"
#include "../lib/IconFontCppHeaders/IconsFontAwesome5.h"
#include "gui.h"
#include "gui_internal.h"

#include <imgui.h>
#include <queue>
#include <spdlog/spdlog.h>
#include <utility>

namespace bricksim::gui::modals {

    namespace {
        std::queue<std::shared_ptr<Modal>> waiting;
        std::shared_ptr<Modal> current;

        void openNextModal() {
            current = waiting.front();
            waiting.pop();
            current->open();
            spdlog::debug("one modal closed, open next");
        }
    }

    Modal::Modal(std::string title, std::string message) :
        title(std::move(title)), message(std::move(message)), state(State::BEFORE_SHOW) {}

    void Modal::open() {
        ImGui::OpenPopup(getFullWindowTitle().c_str());
        state = State::SHOWING;
    }

    std::string Modal::getFullWindowTitle() {
        return title;
    }

    bool Modal::draw() {
        bool showAgain = false;
        if (state == State::SHOWING && ImGui::BeginPopupModal(getFullWindowTitle().c_str(), nullptr, ImGuiWindowFlags_Modal)) {
            showAgain = drawContent();
            ImGui::EndPopup();
        }
        if (!showAgain) {
            state = State::AFTER_SHOW;
        }
        return showAgain;
    }

    Modal::State Modal::getState() const {
        return state;
    }

    void Modal::close() {
        state = State::AFTER_SHOW;
    }

    ErrorModal::ErrorModal(std::string errorMessage) :
        Modal(ICON_FA_EXCLAMATION_CIRCLE " Error", std::move(errorMessage)) {
    }

    ErrorModal::ErrorModal(std::string title, std::string errorMessage) :
        Modal(std::move(title), std::move(errorMessage)) {
    }

    bool ErrorModal::drawContent() {
        //todo draw a big error icon on the left side https://github.com/bb1950328/BrickSim/issues/50
        ImGui::TextColored(color::RGB::RED, "%s", message.c_str());
        ImGui::PushItemWidth(-1);
        if (ImGui::Button("OK")) {
            return false;
        }
        return true;
    }

    void addToQueue(const std::shared_ptr<Modal>& modal) {
        waiting.push(modal);
    }

    void handle() {
        if (current == nullptr && !waiting.empty()) {
            openNextModal();
        }

        while (current != nullptr && !current->draw()) {
            if (waiting.empty()) {
                current = nullptr;
                spdlog::debug("last modal closed");
            } else {
                openNextModal();
            }
        }
    }

    WaitModal::WaitModal(std::string message, const float* const progress) :
        Modal(ICON_FA_HOURGLASS " Please wait", std::move(message)), progress(progress) {
    }

    bool WaitModal::drawContent() {
        const auto& logoTexture = getLogoTexture();
        ImGui::Image(gui_internal::convertTextureId(logoTexture->getID()),
                     ImVec2(logoTexture->getSize().x, logoTexture->getSize().y),
                     ImVec2(0, 1), ImVec2(1, 0));
        ImGui::Text("Please wait until this operation has finished.");
        ImGui::Separator();
        ImGui::Text("%s", message.c_str());
        ImGui::ProgressBar(*progress);
        return *progress < 1.0f;
    }
}