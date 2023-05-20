#include "context_menu_handler.h"

#include <imgui_internal.h>

namespace bricksim::gui::node_context_menu {
    bool ImGuiContextMenuDrawHandler::beginMenu() const {
        return ImGui::BeginPopupEx(POPUP_ID_HASH, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize);
    }
    bool ImGuiContextMenuDrawHandler::drawAction(const std::string& name, std::optional<color::RGB> color) const {
        if (color.has_value()) {
            ImGui::PushStyleColor(ImGuiCol_Text, *color);
        }
        bool clicked = ImGui::MenuItem(name.c_str());
        if (color.has_value()) {
            ImGui::PopStyleColor();
        }
        return clicked;
    }
    void ImGuiContextMenuDrawHandler::endMenu() const {
        ImGui::EndPopup();
    }
    bool ImGuiContextMenuDrawHandler::beginSubMenu(const std::string& name, std::optional<color::RGB> color) const {
        if (color.has_value()) {
            ImGui::PushStyleColor(ImGuiCol_Text, *color);
        }
        bool clicked = ImGui::BeginMenu(name.c_str());
        if (color.has_value()) {
            ImGui::PopStyleColor();
        }
        return clicked;
    }
    void ImGuiContextMenuDrawHandler::endSubMenu() const {
        ImGui::EndMenu();
    }

    ImGuiContextMenuDrawHandler::ImGuiContextMenuDrawHandler() = default;
    ImGuiContextMenuDrawHandler::~ImGuiContextMenuDrawHandler() = default;

    bool ContextMenuDrawHandler::beginSubMenu(const std::string& name) const {
        return beginSubMenu(name, {});
    }
    bool ContextMenuDrawHandler::drawAction(const std::string& name) const {
        return drawAction(name, {});
    }
    ContextMenuDrawHandler::ContextMenuDrawHandler() = default;
    ContextMenuDrawHandler::~ContextMenuDrawHandler() = default;
}
