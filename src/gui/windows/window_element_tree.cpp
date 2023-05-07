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
                ImGui::PushStyleColor(ImGuiCol_Text, getColorOfType(node->getType()));

                bool itemClicked = false;
                if (node->getChildren().empty()) {
                    auto flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                    if (node->selected) {
                        flags |= ImGuiTreeNodeFlags_Selected;
                    }
                    ImGui::TreeNodeEx(node->getDescription().c_str(), flags);
                    itemClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
                    node_context_menu::drawContextMenu(node, contextMenuDrawHandler);
                } else {
                    auto flags = node->selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None;
                    if (ImGui::TreeNodeEx(node->displayName.c_str(), flags)) {
                        itemClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
                        node_context_menu::drawContextMenu(node, contextMenuDrawHandler);
                        for (const auto& child: node->getChildren()) {
                            drawElementTreeNode(child, editor);
                        }
                        ImGui::TreePop();
                    }
                }
                ImGui::PopStyleColor();
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
