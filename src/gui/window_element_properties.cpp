//
// Created by Bader on 15.11.2020.
//

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "gui.h"
#include "../controller.h"
#include "../info_providers/part_color_availability_provider.h"
#include "../info_providers/price_guide_provider.h"
#include "gui_internal.h"

namespace gui {
    void windows::drawElementPropertiesWindow(bool *show) {
        static etree::Node *lastSelectedNode = nullptr;
        ImGui::Begin("Element Properties", show);
        if (controller::getSelectedNodes().empty()) {
            ImGui::Text("Select an element to view its properties here");
        } else if (controller::getSelectedNodes().size() == 1) {
            auto node = *controller::getSelectedNodes().begin();

            static char displayNameBuf[255];
            if (nullptr != lastSelectedNode) {
                lastSelectedNode->displayName = std::string(displayNameBuf);
            }
            strcpy(displayNameBuf, node->displayName.data());
            const auto displayNameEditable = node->isDisplayNameUserEditable();
            auto flags = displayNameEditable ? ImGuiInputTextFlags_None : ImGuiInputTextFlags_ReadOnly;
            ImGui::InputText("Name", displayNameBuf, 255, flags);
            if (!displayNameEditable && ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted("Changing the name of an element of this type is not possible.");
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
            auto typeColor = getColorOfType(node->getType()).asGlmVector();
            //ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(typeColor.x, typeColor.y, typeColor.z, 1.0));//todo make this affect only text but not label
            static char typeBuffer[255];
            strcpy(typeBuffer, getDisplayNameOfType(node->getType()));
            ImGui::InputText("Type", typeBuffer, 255, ImGuiInputTextFlags_ReadOnly);
            //ImGui::PopStyleColor();
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted("Type is not mutable");
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }

            if (node->isTransformationUserEditable()) {
                auto treeRelTransf = glm::transpose(node->getRelativeTransformation());
                glm::quat treeOrientation;
                glm::vec3 treeSkew;
                glm::vec4 treePerspective;
                glm::vec3 treePosition;
                glm::vec3 treeScale;
                glm::vec3 treeEulerAnglesRad;
                glm::decompose(treeRelTransf, treeScale, treeOrientation, treePosition, treeSkew, treePerspective);
                treeEulerAnglesRad = glm::eulerAngles(treeOrientation);

                static glm::vec3 inputEulerAnglesDeg;
                static glm::vec3 inputPosition;
                static glm::vec3 inputScalePercent;
                if (lastSelectedNode != node) {
                    inputEulerAnglesDeg = treeEulerAnglesRad * (float) (180.0f / M_PI);
                    inputPosition = treePosition;
                    inputScalePercent = treeScale * 100.0f;
                }
                glm::vec3 inputEulerAnglesRad = inputEulerAnglesDeg * (float) (M_PI / 180.0f);

                if ((util::biggestValue(glm::abs(treePosition - inputPosition)) > 0.01)
                    || (util::biggestValue(glm::abs(inputEulerAnglesRad - treeEulerAnglesRad)) > 0.0001)
                    || (util::biggestValue(glm::abs(inputScalePercent / 100.0f - treeScale)) > 0.001)) {
                    auto newRotation = glm::eulerAngleXYZ(inputEulerAnglesRad.x, inputEulerAnglesRad.y,
                                                          inputEulerAnglesRad.z);
                    auto newTranslation = glm::translate(glm::mat4(1.0f), inputPosition);
                    auto newScale = glm::scale(glm::mat4(1.0f), inputScalePercent / 100.0f);
                    auto newTransformation = newTranslation * newRotation * newScale;
                    if (treeRelTransf != newTransformation) {
                        node->setRelativeTransformation(glm::transpose(newTransformation));
                        controller::setElementTreeChanged(true);
                    }
                }

                ImGui::DragFloat3("Rotation", &inputEulerAnglesDeg[0], 1.0f, -180, 180, "%.1f°");
                ImGui::DragFloat3("Position", &inputPosition[0], 1.0f, -1e9, 1e9, "%.0fLDU");
                ImGui::DragFloat3("Scale", &inputScalePercent[0], 1.0f, -1e9, 1e9, "%.2f%%");
            }
            if ((node->getType() & etree::NodeType::TYPE_MESH) > 0) {
                auto *meshNode = dynamic_cast<etree::MeshNode *>(node);
                if (meshNode->isColorUserEditable() && ImGui::TreeNodeEx("Color", ImGuiTreeNodeFlags_DefaultOpen)) {
                    static bool isColor16, savedIsColor16;
                    if (lastSelectedNode == node) {
                        if (isColor16 != savedIsColor16) {
                            if (isColor16) {
                                meshNode->setColor(ldr_color_repo::get_color(LdrColor::MAIN_COLOR_CODE));
                            } else {
                                meshNode->setColor(meshNode->getDisplayColor());
                            }
                            savedIsColor16 = isColor16;
                        }
                    } else {
                        savedIsColor16 = isColor16 = meshNode->getElementColor()->code == LdrColor::MAIN_COLOR_CODE;
                    }
                    ImGui::Checkbox("Take color from parent element", &isColor16);
                    if (!isColor16) {
                        const auto buttonWidth = ImGui::GetFontSize() * 1.5f;
                        const ImVec2 &buttonSize = ImVec2(buttonWidth, buttonWidth);
                        const int columnCount = std::floor(ImGui::GetContentRegionAvailWidth() / (buttonWidth + ImGui::GetStyle().ItemSpacing.x));

                        std::optional<std::set<const LdrColor *>> availableColors = std::nullopt;
                        if (meshNode->getType() == etree::TYPE_PART) {
                            availableColors = part_color_availability_provider::getAvailableColorsForPart(
                                    dynamic_cast<etree::LdrNode *>(meshNode)->ldrFile);
                        }
                        bool showAllColors;
                        if (availableColors.has_value() && !availableColors.value().empty()) {
                            static bool onlyAvailableChecked = false;
                            ImGui::Checkbox("Only show available Colors", &onlyAvailableChecked);
                            showAllColors = !onlyAvailableChecked;
                            if (onlyAvailableChecked) {
                                std::pair<std::string, std::vector<const LdrColor *>> group = std::make_pair("Available", std::vector<const LdrColor *>());
                                for (const auto &color : availableColors.value()) {
                                    group.second.push_back(color);
                                }
                                gui_internal::drawColorGroup(meshNode, buttonSize, columnCount, group);
                            }
                        } else {
                            showAllColors = true;
                        }
                        if (showAllColors) {
                            const auto &groupedAndSortedByHue = ldr_color_repo::getAllColorsGroupedAndSortedByHue();
                            const static std::vector<std::string> fixed_pos = {"Solid", "Transparent", "Rubber"};
                            for (const auto &colorName : fixed_pos) {
                                const auto &colorGroup = std::make_pair(colorName, groupedAndSortedByHue.find(colorName)->second);
                                gui_internal::drawColorGroup(meshNode, buttonSize, columnCount, colorGroup);
                            }
                            for (const auto &colorGroup : groupedAndSortedByHue) {
                                if (std::find(fixed_pos.begin(), fixed_pos.end(), colorGroup.first) == fixed_pos.end()) {
                                    gui_internal::drawColorGroup(meshNode, buttonSize, columnCount, colorGroup);
                                }
                            }
                        }
                    }
                    ImGui::TreePop();
                }
            }
            if (node->getType() == etree::NodeType::TYPE_PART) {
                if (ImGui::TreeNodeEx("Price Guide")) {
                    auto *partNode = dynamic_cast<etree::PartNode *>(node);
                    auto partCode = partNode->ldrFile->metaInfo.name;
                    util::replaceAll(partCode, ".dat", "");
                    const auto color = partNode->getDisplayColor();
                    const auto currencyCode = config::getString(config::BRICKLINK_CURRENCY_CODE);
                    const auto colorBricklinkName = util::translateLDrawColorNameToBricklink(color->name);
                    auto availableColors = part_color_availability_provider::getAvailableColorsForPart(partNode->ldrFile);
                    if (availableColors.has_value()) {
                        if (ImGui::Button("(Re)load all available colors")) {
                            for (const auto &item : availableColors.value()) {
                                controller::addBackgroundTask("Reload price guide for " + partCode + " in " + item->name, [partCode, item, currencyCode]() {
                                    price_guide_provider::getPriceGuide(partCode, currencyCode, util::translateLDrawColorNameToBricklink(item->name), true);
                                });
                            }
                        }
                    }
                    std::map<const LdrColor *, const price_guide_provider::PriceGuide> pGuides;
                    if (availableColors.has_value()) {
                        for (const auto &item : availableColors.value()) {
                            auto pg = price_guide_provider::getPriceGuideIfCached(partCode, currencyCode,
                                                                                  util::translateLDrawColorNameToBricklink(item->name));
                            if (pg.has_value()) {
                                pGuides.emplace(item, pg.value());
                            }
                        }
                    } else {
                        auto pg = price_guide_provider::getPriceGuideIfCached(partCode, currencyCode, colorBricklinkName);
                        if (pg.has_value()) {
                            pGuides.emplace(color, pg.value());
                        }
                    }
                    if (!pGuides.empty()) {
                        if (pGuides.find(color) != pGuides.end()) {
                            if (ImGui::Button(("Reload for " + color->name).c_str())) {
                                controller::addBackgroundTask("Reload price guide for " + partCode, [partCode, colorBricklinkName, currencyCode]() {
                                    price_guide_provider::getPriceGuide(partCode, currencyCode, colorBricklinkName, true);
                                });
                            }
                        }
                        ImGui::Text("Currency: %s", currencyCode.c_str());
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Can be changed in settings");
                        }

                        const auto windowBgImVec = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
                        const auto windowBg = glm::vec3(windowBgImVec.x, windowBgImVec.y, windowBgImVec.z);
                        auto drawColoredValueText = [&windowBg](const char *text, const LdrColor *color) {
                            ImGui::SameLine();
                            auto col = color->value.asGlmVector();
                            if (util::vectorSum(glm::abs(windowBg - col)) < 0.3) {
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(col.x, col.y, col.z, 1.0f));
                                auto bgColor = gui_internal::getWhiteOrBlackBetterContrast(col);
                                ImGui::PushStyleColor(ImGuiCol_Button, bgColor);
                                ImGui::PushStyleColor(ImGuiCol_ButtonActive, bgColor);
                                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgColor);
                                ImGui::Button(text);
                                ImGui::PopStyleColor(4);
                            } else {
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(col.x, col.y, col.z, 1.0f));
                                ImGui::Text("%s", text);
                                ImGui::PopStyleColor();
                            }
                            if (ImGui::IsItemHovered()) {
                                ImGui::SetTooltip("%s", color->name.c_str());
                            }
                        };

                        static char valueBuffer[10];

                        ImGui::Text("Total Lots: ");
                        for (const auto &pGuide : pGuides) {
                            snprintf(valueBuffer, 10, "%d", pGuide.second.totalLots);
                            drawColoredValueText(valueBuffer, pGuide.first);
                        }

                        ImGui::Text("Total Qty: ");
                        for (const auto &pGuide : pGuides) {
                            snprintf(valueBuffer, 10, "%d", pGuide.second.totalQty);
                            drawColoredValueText(valueBuffer, pGuide.first);
                        }

                        ImGui::Text("Min Price: ");
                        for (const auto &pGuide : pGuides) {
                            snprintf(valueBuffer, 10, pGuide.second.minPrice < 0.05 ? "%.4f" : "%.2f", pGuide.second.minPrice);
                            drawColoredValueText(valueBuffer, pGuide.first);
                        }

                        ImGui::Text("Avg Price: ");
                        for (const auto &pGuide : pGuides) {
                            snprintf(valueBuffer, 10, pGuide.second.minPrice < 0.05 ? "%.4f" : "%.2f", pGuide.second.avgPrice);
                            drawColoredValueText(valueBuffer, pGuide.first);
                        }

                        ImGui::Text("Qty avg Price: ");
                        for (const auto &pGuide : pGuides) {
                            snprintf(valueBuffer, 10, pGuide.second.minPrice < 0.05 ? "%.4f" : "%.2f", pGuide.second.qtyAvgPrice);
                            drawColoredValueText(valueBuffer, pGuide.first);
                        }

                        ImGui::Text("Max Price: ");
                        for (const auto &pGuide : pGuides) {
                            snprintf(valueBuffer, 10, pGuide.second.minPrice < 0.05 ? "%.4f" : "%.2f", pGuide.second.maxPrice);
                            drawColoredValueText(valueBuffer, pGuide.first);
                        }
                        //todo a small histogram would be nice (parse data from price guide html table)
                    } else {
                        if (ImGui::Button(("Get for " + color->name).c_str())) {
                            controller::addBackgroundTask("Get Price Guide for " + partCode, [partCode, colorBricklinkName, currencyCode]() {
                                price_guide_provider::getPriceGuide(partCode, currencyCode, colorBricklinkName, false);
                            });
                        }
                        if (availableColors.has_value() &&
                            ImGui::Button(("Get for all " + std::to_string(availableColors.value().size()) + " available colors").c_str())) {
                            for (const auto &avCol : availableColors.value()) {
                                controller::addBackgroundTask("Get Price Guide for " + partCode + " in " + avCol->name, [partCode, avCol, currencyCode]() {
                                    price_guide_provider::getPriceGuide(partCode, currencyCode, util::translateLDrawColorNameToBricklink(avCol->name),
                                                                        false);
                                });
                            }
                        }
                    }
                    ImGui::TreePop();
                }
            }

            lastSelectedNode = node;
        } else {
        }

        if (!controller::getSelectedNodes().empty()) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8, 0, 0, 1));
            std::string deleteButtonLabel = controller::getSelectedNodes().size() > 1
                                            ? (std::string("Delete ") + std::to_string(controller::getSelectedNodes().size()) + " elements")
                                            : "Delete Element";
            const auto deleteClicked = ImGui::Button(deleteButtonLabel.c_str());
            ImGui::PopStyleColor();
            if (deleteClicked) {
                controller::deleteSelectedElements();
            }
        }

        ImGui::End();
    }
}