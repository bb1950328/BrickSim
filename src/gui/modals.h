#pragma once

#include <imgui.h>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace bricksim::gui::modals {
    class Modal {
    public:
        enum class State {
            BEFORE_SHOW,
            SHOWING,
            AFTER_SHOW,
        };

        Modal(std::string title, std::string message);

        void open();
        void close();

        /**
         * @return true if it should be drawn again next frame, false if it got closed
         */
        bool draw();

        /**
         * @return true if it should be drawn again next frame, false if it got closed
         */
        virtual bool drawContent() = 0;

        [[nodiscard]] State getState() const;
        [[nodiscard]] const std::string& getMessage() const;

        virtual ~Modal();

    protected:
        std::string title;
        std::string message;
        [[nodiscard]] std::string getFullWindowTitle() const;
        State state = State::BEFORE_SHOW;
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_Modal;
    };

    class ErrorModal : public Modal {
        explicit ErrorModal(std::string errorMessage);
        ErrorModal(std::string title, std::string errorMessage);

    public:
        bool drawContent() override;
    };

    class WaitModal : public Modal {
        const float* const progress;
    public:
        WaitModal(std::string message, const float* progress);
        bool drawContent() override;
    };

    struct Answer {
        std::string text;
        std::optional<color::RGB> buttonColor;
    };

    class ClosedEndedQuestionModal : public Modal {
    protected:
        std::vector<Answer> answers;
        float totalButtonWidth;
        std::optional<size_t> chosenAnswer = std::nullopt;
    public:
        ClosedEndedQuestionModal(std::string question, std::vector<Answer> answers);
        ClosedEndedQuestionModal(std::string title, std::string question, std::vector<Answer> answers);
        bool drawContent() override;
    };

    void addToQueue(const std::shared_ptr<Modal>& modal);
    void handle();
}
