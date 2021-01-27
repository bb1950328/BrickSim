//
// Created by bb1950328 on 15.11.2020.
//

#include "gui.h"
#include "../controller.h"

void draw_element_tree_node(etree::Node *node) {
    if (node->visibleInElementTree) {
        auto colorVec = glm::vec4(getColorOfType(node->getType()).asGlmVector(), 1.0);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(colorVec.x, colorVec.y, colorVec.z, colorVec.w));

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
                for (const auto &child: node->getChildren()) {
                    draw_element_tree_node(child);
                }
                ImGui::TreePop();
            }
        }
        ImGui::PopStyleColor();
        if (itemClicked) {
            if (ImGui::GetIO().KeyCtrl) {
                controller::nodeSelectAddRemove(node);
            } else if (ImGui::GetIO().KeyShift) {
                controller::nodeSelectUntil(node);
            } else {
                controller::nodeSelectSet(node);
            }
        }
    }
}

namespace gui {
    void windows::drawElementTreeWindow(bool *show) {
        ImGui::Begin(WINDOW_NAME_ELEMENT_TREE, show);
        for (auto *rootChild : controller::getElementTree().rootNode.getChildren()) {
            draw_element_tree_node(rootChild);
        }
        ImGui::End();
    }
}
