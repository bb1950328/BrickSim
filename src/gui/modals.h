#pragma once

#include <string>

namespace bricksim::gui::modals {
    class Modal {
    protected:
        std::string title;
        std::string message;

    public:
        Modal(std::string  title, std::string  message);

        void open();
        /**
         * @return true if it should be drawn again next frame, false if it got closed
         */
        bool draw();

        /**
         * @return true if it should be drawn again next frame, false if it got closed
         */
        virtual bool drawContent() = 0;

    private:
        std::string getFullWindowTitle();
    };

    class ErrorModal : public Modal {
        ErrorModal(std::string errorMessage);
        ErrorModal(std::string title, std::string errorMessage);

    public:
        bool drawContent() override;
    };
}