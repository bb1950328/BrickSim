//
// Created by Bader on 15.11.2020.
//

#include "gui_internal.h"
#include "../controller.h"
#include "../info_providers/part_color_availability_provider.h"

namespace gui_internal {
    void drawPartThumbnail(const ImVec2 &actualThumbSizeSquared, LdrFile *const &part, LdrColor *color) {
        bool realThumbnailAvailable = false;
        if (ImGui::IsRectVisible(actualThumbSizeSquared)) {
            auto optTexId = controller::getThumbnailGenerator().getThumbnailNonBlocking(part, color);
            if (optTexId.has_value()) {
                auto texId = (ImTextureID) (optTexId.value());
                ImGui::ImageButton(texId, actualThumbSizeSquared, ImVec2(0, 1), ImVec2(1, 0), 0);
                realThumbnailAvailable = true;
            }
        }
        if (!realThumbnailAvailable) {
            ImGui::Button(part->metaInfo.name.c_str(), actualThumbSizeSquared);
        }
        if (ImGui::IsItemHovered()) {
            auto availableColors = part_color_availability_provider::getAvailableColorsForPart(part);
            std::string availText;
            if (availableColors.has_value() && !availableColors.value().empty()) {
                if (availableColors.value().size() == 1) {
                    availText = std::string("\nOnly available in ") + (*availableColors.value().begin())->name;
                } else {
                    availText = std::string("\nAvailable in ") + std::to_string(availableColors.value().size()) + " Colors";
                }
            } else {
                availText = std::string("");
            }
            ImGui::SetTooltip("%s\n%s%s", part->metaInfo.title.c_str(), part->metaInfo.name.c_str(), availText.c_str());
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            controller::insertLdrElement(part);
        }
    }

    ImVec4 getWhiteOrBlackBetterContrast(const glm::vec3 &col) {
        return util::vectorSum(col) > 1.5 ? ImVec4(0, 0, 0, 1) : ImVec4(1, 1, 1, 1);
    }

    void drawColorGroup(etree::MeshNode *ldrNode, const ImVec2 &buttonSize, const int columnCount, const std::pair<const std::string, std::vector<const LdrColor *>> &colorGroup) {
        if (ImGui::TreeNodeEx(colorGroup.first.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            int i = 0;
            for (const auto *color : colorGroup.second) {
                if (i % columnCount > 0) {
                    ImGui::SameLine();
                }
                ImGui::PushID(color->code);
                const ImColor imColor = ImColor(color->value.red, color->value.green, color->value.blue);
                ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4) imColor);
                if (ImGui::Button(ldrNode->getDisplayColor()->code == color->code ? "#" : "", buttonSize)) {
                    ldrNode->setColor(ldr_color_repo::get_color(color->code));
                    controller::setElementTreeChanged(true);
                }
                ImGui::PopStyleColor(/*3*/1);
                ImGui::PopID();
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("%s", color->name.c_str());
                    ImGui::EndTooltip();
                }
                ++i;
            }
            ImGui::TreePop();
        }
    }

    void draw_hyperlink_button(const std::string &url) {
        if (ImGui::Button(url.c_str())) {
            util::openDefaultBrowser(url);
        }
    }
}