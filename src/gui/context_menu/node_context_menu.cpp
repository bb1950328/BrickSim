#include "node_context_menu.h"

#include "../../lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include "context_menu_handler.h"
#include "imgui_internal.h"
#include "spdlog/spdlog.h"
#include <utility>

namespace bricksim::gui::node_context_menu {
    namespace {
        Context context;
        uint64_t contextUnusedFrames = 0;
        std::unique_ptr<ContextMenuDrawHandler> drawHandler = std::make_unique<ImGuiContextMenuDrawHandler>();
        struct ContextMenuItem {
            const char* const name;
            etree::NodeType requiredType;
            bool allowInMultiSelect;
            std::function<void(const std::shared_ptr<Editor>&, const std::shared_ptr<etree::Node>&)> action;
            std::optional<color::RGB> textColor;
        };

        static const std::vector<ContextMenuItem> ITEMS = {
                {ICON_FA_FILE_PEN " Make Editing Model",
                 etree::NodeType::TYPE_MODEL,
                 false,
                 [](const auto& editor, const auto& node) {
                     editor->setEditingModel(std::dynamic_pointer_cast<etree::ModelNode>(node));
                 }},
                {ICON_FA_BARS " Inline All Instances",
                 etree::NodeType::TYPE_MODEL,
                 true,
                 [](const auto& editor, const auto& node) {
                     editor->inlineElement(std::dynamic_pointer_cast<etree::ModelNode>(node));
                 }},
                {ICON_FA_BARS " Inline This Instance",
                 etree::NodeType::TYPE_MODEL_INSTANCE,
                 true,
                 [](const auto& editor, const auto& node) {
                     editor->inlineElement(std::dynamic_pointer_cast<etree::ModelInstanceNode>(node));
                 }},
                {ICON_FA_FILE_PEN " Edit Referenced Model",
                 etree::NodeType::TYPE_MODEL_INSTANCE,
                 false,
                 [](const auto& editor, const auto& node) {
                     editor->setEditingModel(std::dynamic_pointer_cast<etree::ModelInstanceNode>(node)->modelNode);
                 }},
                {ICON_FA_TRASH " Delete",
                 etree::NodeType::TYPE_OTHER,
                 true,
                 [](const auto& editor, const auto& node) {
                     editor->deleteElement(node);
                 },
                 color::RED},
                {ICON_FA_ARROWS_TO_EYE " Center in 3D View",
                 etree::NodeType::TYPE_OTHER,
                 false,
                 [](const auto& editor, const auto& node) {
                     editor->centerElementIn3dView(node);
                 }},
        };
    }

    void drawContextMenu() {
        if (drawHandler->beginMenu()) {
            bool allSame = true;
            auto sameType = context.nodes[0]->type;
            for (const auto& item: context.nodes) {
                if (sameType != item->type) {
                    allSame = false;
                    break;
                }
            }
            bool multiSelect = context.nodes.size() > 1;

            for (const auto& item: ITEMS) {
                if (multiSelect && (!item.allowInMultiSelect || !allSame)) {
                    continue;
                }
                if (item.requiredType != etree::NodeType::TYPE_OTHER && item.requiredType != sameType) {
                    continue;
                }
                if (drawHandler->drawAction(item.name, item.textColor)) {
                    for (const auto& n: context.nodes) {
                        item.action(context.editor, n);
                    }
                }
            }
            drawHandler->endMenu();
        } else if (!context.nodes.empty()) {
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
        spdlog::debug("Open context menu on {}{}",
                      context.nodes[0]->displayName,
                      context.nodes.size() > 1
                              ? fmt::format(" and {} other nodes", context.nodes.size())
                              : "");
    }
}
