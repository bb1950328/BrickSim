#include "node_context_menu.h"

#include "../../lib/IconFontCppHeaders/IconsFontAwesome5.h"
#include "context_menu_handler.h"
#include "imgui_internal.h"
#include "spdlog/spdlog.h"
#include <utility>

namespace bricksim::gui::node_context_menu {
    namespace {
        Context context;
        uint64_t contextUnusedFrames = 0;
        std::unique_ptr<ContextMenuDrawHandler> drawHandler = std::make_unique<ImGuiContextMenuDrawHandler>();
    }

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
                if (drawHandler->drawAction(ICON_FA_CROSSHAIRS " Center in 3D View")) {
                    context.editor->centerElementIn3dView(context.node);
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
        ImGui::OpenPopupEx(ImGuiContextMenuDrawHandler::POPUP_ID_HASH);
        spdlog::debug("Open context menu on {}", context.node->displayName);
    }
}
