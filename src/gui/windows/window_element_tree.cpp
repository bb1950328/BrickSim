#include "../../controller.h"
#include "../gui.h"
#include <memory>

#include "../node_context_menu.h"
#include "window_element_tree.h"

namespace bricksim::gui::windows::element_tree {
    namespace {
        const node_context_menu::ImGuiContextMenuDrawHandler contextMenuDrawHandler{};
        void drawElementTreeNode(const std::shared_ptr<etree::Node>& node, const std::shared_ptr<Editor>& editor) {
            if (node->visibleInElementTree) {
                color::RGB textColor = color::WHITE;
                if (node == editor->getRootNode()) {
                    if (controller::getActiveEditor() == editor) {
                        textColor = COLOR_ACTIVE_EDITOR;
                    }
                } else if (node == editor->getEditingModel()) {
                    textColor = COLOR_EDITING_MODEL;
                } else {
                    textColor = etree::getColorOfType(node->getType());
                }

                ImGuiTreeNodeFlags flags = node->selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None;
                ImGui::PushStyleColor(ImGuiCol_Text, textColor);
                if (node->getChildren().empty()) {
                    flags |= ImGuiTreeNodeFlags_Leaf;
                    flags |= ImGuiTreeNodeFlags_NoTreePushOnOpen;
                }
                const auto drawChildren = ImGui::TreeNodeEx(node->getDescription().c_str(), flags);
                const auto itemClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
                ImGui::PopStyleColor();
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                    node_context_menu::openContextMenu({editor, node});
                }
                if (drawChildren && !node->getChildren().empty()) {
                    for (const auto& child: node->getChildren()) {
                        drawElementTreeNode(child, editor);
                    }
                    ImGui::TreePop();
                }
                if (itemClicked) {
                    editor->nodeClicked(node, ImGui::GetIO().KeyCtrl, ImGui::GetIO().KeyShift);
                }
            }
        }
    }

    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible)) {
            for (auto& editor: controller::getEditors()) {
                drawElementTreeNode(editor->getRootNode(), editor);
            }
        }
        ImGui::End();
    }
}
