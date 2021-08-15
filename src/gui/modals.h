#pragma once

#include <memory>
#include <string>

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

    protected:
        std::string title;
        std::string message;
        std::string getFullWindowTitle();
        State state;
    };

    class ErrorModal : public Modal {
        ErrorModal(std::string errorMessage);
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

    void addToQueue(const std::shared_ptr<Modal>& modal);
    void handle();
}