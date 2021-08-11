#include "../../controller.h"
#include "../gui.h"
#include <memory>

#include "window_element_tree.h"

namespace bricksim::gui::windows::element_tree {
    namespace {
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
                    //todo add context menu
                } else {
                    auto flags = node->selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None;
                    if (ImGui::TreeNodeEx(node->displayName.c_str(), flags)) {
                        itemClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
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
                drawElementTreeNode(editor->getDocumentNode(), editor);
            }
        }
        ImGui::End();
    }
}
