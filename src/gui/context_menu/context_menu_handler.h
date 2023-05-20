#pragma once
#include "../../helpers/color.h"
#include <imgui.h>
#include <optional>
#include <string>

namespace bricksim::gui::node_context_menu {
    class ContextMenuDrawHandler {
    public:
        ContextMenuDrawHandler();
        virtual ~ContextMenuDrawHandler();

        [[nodiscard]] virtual bool beginMenu() const = 0;
        [[nodiscard]] virtual bool beginSubMenu(const std::string& name) const;
        [[nodiscard]] virtual bool beginSubMenu(const std::string& name, std::optional<color::RGB> color) const = 0;
        [[nodiscard]] virtual bool drawAction(const std::string& name, std::optional<color::RGB> color) const = 0;
        [[nodiscard]] virtual bool drawAction(const std::string& name) const;
        virtual void endSubMenu() const = 0;
        virtual void endMenu() const = 0;
    };
    class ImGuiContextMenuDrawHandler : public ContextMenuDrawHandler {
    public:
        static const ImGuiID POPUP_ID_HASH = 0x283e48fd;
        ImGuiContextMenuDrawHandler();
        ~ImGuiContextMenuDrawHandler() override;

        [[nodiscard]] bool beginMenu() const override;
        [[nodiscard]] bool beginSubMenu(const std::string& name, std::optional<color::RGB> color) const override;
        [[nodiscard]] bool drawAction(const std::string& name, std::optional<color::RGB> color) const override;
        void endSubMenu() const override;
        void endMenu() const override;
    };
}
