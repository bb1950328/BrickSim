#include "modals.h"
#include "../lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include "gui.h"
#include "gui_internal.h"

#include <imgui.h>
#include <palanteer.h>
#include <queue>
#include <spdlog/spdlog.h>
#include <utility>
#include <numeric>

namespace bricksim::gui::modals {

    namespace {
        std::queue<std::shared_ptr<Modal>> waiting;
        std::shared_ptr<Modal> current;

        void openNextModal() {
            current = waiting.front();
            waiting.pop();
            current->open();
        }
    }

    Modal::Modal(std::string title, std::string message) :
        title(std::move(title)), message(std::move(message)) {}

    void Modal::open() {
        ImGui::OpenPopup(getFullWindowTitle().c_str());
        state = State::SHOWING;
    }

    std::string Modal::getFullWindowTitle() const {
        return title;
    }

    bool Modal::draw() {
        bool showAgain = state == State::SHOWING;
        if (showAgain) {
            while (!ImGui::BeginPopupModal(getFullWindowTitle().c_str(), nullptr, windowFlags)) {
                ImGui::OpenPopup(getFullWindowTitle().c_str());
            }
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
    const std::string& Modal::getMessage() const {
        return message;
    }
    Modal::~Modal() = default;

    ErrorModal::ErrorModal(const std::string& errorMessage) :
        Modal(ICON_FA_CIRCLE_EXCLAMATION " Error", errorMessage) {
    }

    ErrorModal::ErrorModal(std::string title, std::string errorMessage) :
        Modal(std::move(title), std::move(errorMessage)) {
    }

    bool ErrorModal::drawContent() {
        //todo draw a big error icon on the left side #50
        ImGui::TextColored(color::RED, "%s", message.c_str());
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
        plFunction();
        if (current == nullptr && !waiting.empty()) {
            openNextModal();
            spdlog::debug("no modal open, open first in queue");
        }

        while (current != nullptr && !current->draw()) {
            if (waiting.empty()) {
                current = nullptr;
                spdlog::debug("last modal closed");
            } else {
                openNextModal();
                spdlog::debug("one modal closed, open next");
            }
        }
    }

    WaitModal::WaitModal(std::string message, const float* const progress) :
        Modal(ICON_FA_HOURGLASS " Please wait", std::move(message)), progress(progress) {
    }

    bool WaitModal::drawContent() {
        const auto& logoTexture = getLogoTexture();
        ImGui::Image(gui_internal::convertTextureId(logoTexture->getID()),
                     ImVec2(static_cast<float>(logoTexture->getSize().x), static_cast<float>(logoTexture->getSize().y)),
                     ImVec2(0, 1), ImVec2(1, 0));
        ImGui::Text("Please wait until this operation has finished.");
        ImGui::Separator();
        ImGui::Text("%s", message.c_str());
        ImGui::ProgressBar(*progress);
        return *progress < 1.0f;
    }

    bool ClosedEndedQuestionModal::drawContent() {
        //todo draw big question mark #50
        float questionWidth = ImGui::CalcTextSize(message.c_str()).x;
        ImGui::Text("%s", message.c_str());
        if (questionWidth < totalButtonWidth) {
            ImGui::SameLine();
            ImGui::Dummy({totalButtonWidth - questionWidth, 1.0f});
        }
        ImGui::Separator();
        ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - totalButtonWidth);
        for (int i = 0; i < answers.size(); ++i) {
            const auto& answ = answers[i];
            if (answ.buttonColor.has_value()) {
                ImGui::PushStyleColor(ImGuiCol_Button, answ.buttonColor.value());
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, answ.buttonColor.value() * 0.6 + color::WHITE * 0.4);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, answ.buttonColor.value() * 0.4 + color::WHITE * 0.6);
            }
            if (ImGui::Button(answ.text.c_str())) {
                chosenAnswer = i;
            }
            if (answ.buttonColor.has_value()) {
                ImGui::PopStyleColor(3);
            }
            if (answers.size() - i > 1) {
                ImGui::SameLine();
            }
        }
        return !chosenAnswer.has_value();
    }

    ClosedEndedQuestionModal::ClosedEndedQuestionModal(std::string question, std::vector<Answer> answers) :
        ClosedEndedQuestionModal(ICON_FA_CIRCLE_QUESTION " Question", std::move(question), std::move(answers)) {}

    ClosedEndedQuestionModal::ClosedEndedQuestionModal(std::string title, std::string question, std::vector<Answer> answers) :
        Modal(std::move(title), std::move(question)), answers(std::move(answers)) {
        totalButtonWidth = std::accumulate(this->answers.begin(), this->answers.end(),
                                           static_cast<float>(this->answers.size() + 1) * ImGui::GetStyle().ItemSpacing.x,
                                           [](float total, const Answer& answer) {
                                               return total + ImGui::CalcTextSize(answer.text.c_str()).x;
                                           });

        windowFlags |= ImGuiWindowFlags_AlwaysAutoResize;
    }
}
