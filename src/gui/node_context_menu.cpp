#include "node_context_menu.h"
#include "imgui.h"
#include "spdlog/spdlog.h"

namespace bricksim::gui::node_context_menu {

    bool ImGuiContextMenuDrawHandler::beginMenu() const {
        return ImGui::BeginPopupContextItem();
    }
    bool ImGuiContextMenuDrawHandler::drawAction(const std::string& name) const {
        return ImGui::Button(name.c_str());
    }
    void ImGuiContextMenuDrawHandler::endMenu() const {
        ImGui::EndPopup();
    }
    void drawContextMenu(const std::shared_ptr<etree::Node>& context, const ContextMenuDrawHandler& drawHandler) {
        if (drawHandler.beginMenu()) {
            if (drawHandler.drawAction("blabla")) {
                spdlog::info("clicked1");
            }
            if (drawHandler.drawAction("blublublu")) {
                spdlog::info("clicked2");
            }
            drawHandler.endMenu();
        }
    }
}
