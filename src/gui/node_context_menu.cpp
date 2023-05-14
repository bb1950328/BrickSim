#include "node_context_menu.h"

#include "../lib/IconFontCppHeaders/IconsFontAwesome5.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "spdlog/spdlog.h"
#include <utility>

namespace bricksim::gui::node_context_menu {
    namespace {
        Context context;
        uint64_t contextUnusedFrames = 0;
        std::unique_ptr<ContextMenuDrawHandler> drawHandler = std::make_unique<ImGuiContextMenuDrawHandler>();

        const ImGuiID POPUP_ID_HASH = 0x283e48fd;
    }

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

    void drawContextMenu() {
        if (drawHandler->beginMenu()) {
            if (context.node->type == etree::NodeType::TYPE_MODEL) {
                if (drawHandler->drawAction(ICON_FA_EDIT " Make Editing Model")) {
                    context.editor->setEditingModel(std::dynamic_pointer_cast<etree::ModelNode>(context.node));
                }
            }

            if (context.node->getType() != etree::NodeType::TYPE_ROOT) {
                if (drawHandler->drawAction(ICON_FA_TRASH " Delete", color::RED)) {
                    context.editor->deleteElement(context.node);
                }
            }
            drawHandler->endMenu();
        } else if (context.node != nullptr) {
            ++contextUnusedFrames;
            if (contextUnusedFrames > 8) {
                context = {};
                contextUnusedFrames = 0;
            }
        }
    }

    void openContextMenu(Context newContext) {
        context = std::move(newContext);
        ImGui::OpenPopupEx(POPUP_ID_HASH);
        spdlog::debug("Open context menu on {}", context.node->displayName);
    }
}
