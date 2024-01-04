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

        struct NodeContextMenuItem {
            const char* const name;
            etree::NodeType requiredType;
            bool allowInMultiSelect;
            std::function<void(const std::shared_ptr<Editor>&, const std::shared_ptr<etree::Node>&)> action;
            std::optional<color::RGB> textColor;
        };

        struct NoNodeContextMenuItem {
            const char* const name;
            std::function<bool(const std::shared_ptr<Editor>&)> enabler;
            std::function<void(const std::shared_ptr<Editor>&)> action;
            std::optional<color::RGB> textColor;
        };

        static const std::vector<NodeContextMenuItem> NODE_ITEMS = {
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

        static const std::vector<NoNodeContextMenuItem> NO_NODE_ITEMS = {
                {ICON_FA_FILE_PEN " Edit Last Model",
                 [](const std::shared_ptr<Editor>& editor) {
                     const auto editingModel = editor->getEditingModel();
                     return true;
                 },
                 [](const std::shared_ptr<Editor>& editor) {
                     editor->editLastModelInHistory();
                 }},
        };
    }

    void drawContextMenu() {
        const auto editor = context.editor.lock();
        if (editor == nullptr) {
            return;
        }
        if (drawHandler->beginMenu()) {
            if (context.nodes.empty()) {
                for (const auto& item: NO_NODE_ITEMS) {
                    if (item.enabler(editor)) {
                        if (drawHandler->drawAction(item.name, item.textColor)) {
                            item.action(editor);
                        }
                    }
                }
            } else {
                bool allSame = true;
                auto sameType = context.nodes[0].lock()->type;
                for (const auto& item: context.nodes) {
                    if (sameType != item.lock()->type) {
                        allSame = false;
                        break;
                    }
                }
                bool multiSelect = context.nodes.size() > 1;

                for (const auto& item: NODE_ITEMS) {
                    if (multiSelect && (!item.allowInMultiSelect || !allSame)) {
                        continue;
                    }
                    if (item.requiredType != etree::NodeType::TYPE_OTHER && item.requiredType != sameType) {
                        continue;
                    }
                    if (drawHandler->drawAction(item.name, item.textColor)) {
                        for (const auto& n: context.nodes) {
                            item.action(editor, n.lock());
                        }
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
                      context.nodes.empty() ? "nothing" : context.nodes[0].lock()->displayName,
                      context.nodes.size() > 1
                          ? fmt::format(" and {} other nodes", context.nodes.size())
                          : "");
    }
}
