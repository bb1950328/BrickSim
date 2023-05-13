#include "node_context_menu.h"
#include "../lib/IconFontCppHeaders/IconsFontAwesome5.h"
#include "imgui.h"
#include "spdlog/spdlog.h"

namespace bricksim::gui::node_context_menu {

    bool ImGuiContextMenuDrawHandler::beginMenu() const {
        return ImGui::BeginPopupContextItem();
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
    void drawContextMenu(const std::shared_ptr<Editor>& editor,
                         const std::shared_ptr<etree::Node>& context,
                         const ContextMenuDrawHandler& drawHandler,
                         std::vector<std::function<void()>>& runAfterTasks) {
        if (drawHandler.beginMenu()) {
            if (context->type == etree::NodeType::TYPE_MODEL) {
                if (drawHandler.drawAction(ICON_FA_EDIT " Make Editing Model")) {
                    editor->setEditingModel(std::dynamic_pointer_cast<etree::ModelNode>(context));
                }
            }

            if (context->getType() != etree::NodeType::TYPE_ROOT) {
                if (drawHandler.drawAction(ICON_FA_TRASH " Delete", color::RED)) {
                    runAfterTasks.emplace_back([&context, &editor]() {
                        editor->deleteElement(context);
                    });
                }
            }
            drawHandler.endMenu();
        }
    }
    bool ContextMenuDrawHandler::beginSubMenu(const std::string& name) const {
        return beginSubMenu(name, {});
    }
    bool ContextMenuDrawHandler::drawAction(const std::string& name) const {
        return drawAction(name, {});
    }
}
