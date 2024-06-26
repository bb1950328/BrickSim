#include "../../config/read.h"
#include "../../controller.h"
#include "../../info_providers/part_color_availability_provider.h"
#include "../../info_providers/price_guide_provider.h"
#include "../../lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include "../gui_internal.h"
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include "../gui.h"
#include "window_element_properties.h"
#include "window_ldraw_file_inspector.h"

namespace bricksim::gui::windows::element_properties {
    void drawTransformationEdit(const std::shared_ptr<etree::Node>& lastSelectedNode, const std::shared_ptr<etree::Node>& node) {
        static glm::mat4 lastTreeTransf;
        const glm::mat4 treeRelTransf = glm::transpose(node->getRelativeTransformation());
        const util::DecomposedTransformation decomposedTreeTransf = util::decomposeTransformationToStruct(treeRelTransf);

        bool nodeHasChanged = lastSelectedNode != node;
        bool transformationHasChanged = lastTreeTransf != treeRelTransf;
        lastTreeTransf = treeRelTransf;

        bool translationChanged;
        bool scaleChanged;
        bool rotationChanged;

        static glm::vec3 inputPosition;
        {
            static glm::vec3 lastTreePosition;
            if (nodeHasChanged || transformationHasChanged) {
                lastTreePosition = inputPosition = decomposedTreeTransf.translation;
            }
            ImGui::DragFloat3(ICON_FA_LOCATION_DOT " Position", &inputPosition[0], 10.f, -1e9, 1e9, "%.0fLDU");
            translationChanged = glm::any(glm::epsilonNotEqual(decomposedTreeTransf.translation, inputPosition, 0.01f));
        }

        const static auto useEulerAngles = config::get().elementProperties.angleMode;
        static glm::vec3 inputEulerAnglesDeg;
        static glm::vec3 inputQuatAxis;
        static float inputQuatAngleDeg;
        switch (useEulerAngles) {
            case config::AngleMode::Euler:
            {
                if (nodeHasChanged) {
                    inputEulerAnglesDeg = glm::degrees(glm::eulerAngles(decomposedTreeTransf.orientation));
                }
                const auto lastValue = inputEulerAnglesDeg;
                ImGui::DragFloat3(ICON_FA_ARROWS_ROTATE " Rotation", &inputEulerAnglesDeg[0], 1.f, -180, 180, "%.1f°");

                rotationChanged = glm::any(glm::epsilonNotEqual(inputEulerAnglesDeg, lastValue, 0.0001f));
                break;
            }
            case config::AngleMode::Quaternion:
            {
                if (nodeHasChanged) {
                    inputQuatAngleDeg = glm::angle(decomposedTreeTransf.orientation);
                    inputQuatAxis = glm::axis(decomposedTreeTransf.orientation);
                }
                const auto lastValueAxis = inputQuatAxis;
                const auto lastValueAngleDeg = inputQuatAngleDeg;
                ImGui::DragFloat3(ICON_FA_LOCATION_ARROW " Rotation Axis", &inputQuatAxis[0], 0.01f, -1e9, +1e9, "%.2f");
                ImGui::DragFloat(ICON_FA_ARROWS_ROTATE " Rotation Angle", &inputQuatAngleDeg, 1.0f, 0, 360, "%.0f °");
                rotationChanged = std::abs(lastValueAngleDeg - inputQuatAngleDeg) > 0.01 || glm::any(glm::epsilonNotEqual(inputQuatAxis, lastValueAxis, 0.001f));
                break;
            }
        }

        static glm::vec3 inputScalePercent;
        {
            static glm::vec3 lastTreeScale;
            if (nodeHasChanged || lastTreeScale != decomposedTreeTransf.scale) {
                lastTreeScale = decomposedTreeTransf.scale;
                inputScalePercent = decomposedTreeTransf.scale * 100.0f;
            }
            ImGui::DragFloat3(ICON_FA_MAXIMIZE " Scale", &inputScalePercent[0], 1.0f, -1e9, 1e9, "%.2f%%");
            scaleChanged = util::biggestValue(glm::abs(inputScalePercent / 100.0f - decomposedTreeTransf.scale)) > 0.001;
        }

        if (translationChanged || rotationChanged || scaleChanged) {
            glm::mat4 newRotation;
            switch (useEulerAngles) {
                case config::AngleMode::Euler:
                    newRotation = glm::eulerAngleXYZ(glm::radians(inputEulerAnglesDeg.x), glm::radians(inputEulerAnglesDeg.y), glm::radians(inputEulerAnglesDeg.z));
                    break;
                case config::AngleMode::Quaternion:
                    newRotation = glm::toMat4(glm::angleAxis(glm::radians(inputQuatAngleDeg), inputQuatAxis));
                    break;
            }
            auto newTranslation = glm::translate(glm::mat4(1.0f), inputPosition);
            auto newScale = glm::scale(glm::mat4(1.0f), inputScalePercent / 100.0f);
            auto newTransformation = newTranslation * newRotation * newScale;
            if (treeRelTransf != newTransformation) {
                node->setRelativeTransformation(glm::transpose(newTransformation));
                node->incrementVersion();
                spdlog::debug("user edited transformation in element properties");
            }
        }
    }

    void drawDisplayNameEdit(std::shared_ptr<etree::Node>& lastSelectedNode, const std::shared_ptr<etree::Node>& node) {
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
            ImGui::TextUnformatted(ICON_FA_CIRCLE_XMARK " Changing the name of an element of this type is not possible.");
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    void drawType(const std::shared_ptr<etree::Node>& node) {
        ImGui::PushStyleColor(ImGuiCol_Text, getColorOfType(node->getType()));
        static char typeBuffer[255];
        strcpy(typeBuffer, getDisplayNameOfType(node->getType()));
        ImGui::InputText("##Type", typeBuffer, 255, ImGuiInputTextFlags_ReadOnly);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::Text("Type");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted("Type is not mutable");
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    void drawPriceGuide(const std::shared_ptr<etree::Node>& node) {
        if (ImGui::TreeNodeEx(ICON_FA_MONEY_BILL_WAVE " Price Guide")) {
            auto partNode = std::dynamic_pointer_cast<etree::PartNode>(node);
            auto partCode = partNode->ldrFile->metaInfo.name;
            stringutil::replaceAll(partCode, ".dat", "");
            const auto color = partNode->getDisplayColor().get();
            const auto currencyCode = config::get().bricklinkIntegration.currencyCode;
            const auto colorBricklinkName = util::translateLDrawColorNameToBricklink(color->name);
            auto availableColors = info_providers::part_color_availability::getAvailableColorsForPart(partNode->ldrFile);
            if (availableColors.has_value()) {
                if (ImGui::Button(ICON_FA_CLOUD_ARROW_DOWN " (Re)load all available colors")) {
                    for (const auto& item: availableColors.value()) {
                        const auto itemValue = item.get();
                        controller::addBackgroundTask("Reload price guide for " + partCode + " in " + itemValue->name, [partCode, itemValue, currencyCode]() {
                            info_providers::price_guide::getPriceGuide(partCode, currencyCode, util::translateLDrawColorNameToBricklink(itemValue->name), true);
                        });
                    }
                }
            }
            uomap_t<ldr::ColorReference, const info_providers::price_guide::PriceGuide> pGuides;
            if (availableColors.has_value()) {
                for (const auto& item: availableColors.value()) {
                    auto pg = info_providers::price_guide::getPriceGuideIfCached(partCode, currencyCode,
                                                                                 util::translateLDrawColorNameToBricklink(item.get()->name));
                    if (pg.has_value()) {
                        pGuides.emplace(item, pg.value());
                    }
                }
            } else {
                auto pg = info_providers::price_guide::getPriceGuideIfCached(partCode, currencyCode, colorBricklinkName);
                if (pg.has_value()) {
                    pGuides.emplace(color->asReference(), pg.value());
                }
            }
            if (!pGuides.empty()) {
                if (pGuides.find(color->asReference()) != pGuides.end()) {
                    if (ImGui::Button((ICON_FA_CLOUD_ARROW_DOWN " Reload for " + color->name).c_str())) {
                        controller::addBackgroundTask("Reload price guide for " + partCode, [partCode, colorBricklinkName, currencyCode]() {
                            info_providers::price_guide::getPriceGuide(partCode, currencyCode, colorBricklinkName, true);
                        });
                    }
                }
                ImGui::Text("Currency: %s", currencyCode.c_str());
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Can be changed in settings");
                }

                const auto windowBgImVec = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
                const auto windowBg = glm::vec3(windowBgImVec.x, windowBgImVec.y, windowBgImVec.z);
                auto drawColoredValueText = [&windowBg](const char* text, ldr::ColorReference color) {
                    //ImGui::SameLine();
                    auto colorValue = color.get();
                    auto col = colorValue->value.asGlmVector();
                    if (util::vectorSum(glm::abs(windowBg - col)) < 0.3) {
                        ImGui::PushStyleColor(ImGuiCol_Text, colorValue->value);
                        auto bgColor = gui_internal::getWhiteOrBlackBetterContrast(col);
                        ImGui::PushStyleColor(ImGuiCol_Button, bgColor);
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, bgColor);
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgColor);
                        ImGui::Button(text);
                        ImGui::PopStyleColor(4);
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Text, colorValue->value);
                        ImGui::Text("%s", text);
                        ImGui::PopStyleColor();
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("%s", colorValue->name.c_str());
                    }
                };

                auto tableFlags =
                        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ScrollX;
                ImVec2 outer_size = ImVec2(-FLT_MIN, ImGui::GetTextLineHeightWithSpacing() * 9);
                auto tableId = std::string("##priceGuideTable") + partCode;
                for (const auto& pGuide: pGuides) {
                    tableId += std::string(";") + std::to_string(pGuide.first.code);
                }
                if (ImGui::BeginTable(tableId.c_str(), pGuides.size() + 1, tableFlags, outer_size)) {
                    ImGui::TableSetupScrollFreeze(1, 1);
                    ImGui::TableSetupColumn(ICON_FA_MONEY_BILL, ImGuiTableColumnFlags_NoHide);
                    for (const auto& pGuide: pGuides) {
                        ImGui::TableSetupColumn(pGuide.first.get()->name.c_str());
                    }
                    ImGui::TableHeadersRow();

                    static char valueBuffer[10];
                    int column = 0;

                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(column++);
                    ImGui::Text("Total Lots: ");
                    for (const auto& pGuide: pGuides) {
                        ImGui::TableSetColumnIndex(column++);
                        snprintf(valueBuffer, 10, "%d", pGuide.second.totalLots);
                        drawColoredValueText(valueBuffer, pGuide.first);
                    }

                    column = 0;
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(column++);
                    ImGui::Text("Total Qty: ");
                    for (const auto& pGuide: pGuides) {
                        ImGui::TableSetColumnIndex(column++);
                        snprintf(valueBuffer, 10, "%d", pGuide.second.totalQty);
                        drawColoredValueText(valueBuffer, pGuide.first);
                    }

                    column = 0;
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(column++);
                    ImGui::Text("Min Price: ");
                    for (const auto& pGuide: pGuides) {
                        ImGui::TableSetColumnIndex(column++);
                        snprintf(valueBuffer, 10, pGuide.second.minPrice < 0.05 ? "%.4f" : "%.2f", pGuide.second.minPrice);
                        drawColoredValueText(valueBuffer, pGuide.first);
                    }

                    column = 0;
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(column++);
                    ImGui::Text("Avg Price: ");
                    for (const auto& pGuide: pGuides) {
                        ImGui::TableSetColumnIndex(column++);
                        snprintf(valueBuffer, 10, pGuide.second.minPrice < 0.05 ? "%.4f" : "%.2f", pGuide.second.avgPrice);
                        drawColoredValueText(valueBuffer, pGuide.first);
                    }

                    column = 0;
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(column++);
                    ImGui::Text("Qty avg Price: ");
                    for (const auto& pGuide: pGuides) {
                        ImGui::TableSetColumnIndex(column++);
                        snprintf(valueBuffer, 10, pGuide.second.minPrice < 0.05 ? "%.4f" : "%.2f", pGuide.second.qtyAvgPrice);
                        drawColoredValueText(valueBuffer, pGuide.first);
                    }

                    column = 0;
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(column++);
                    ImGui::Text("Max Price: ");
                    for (const auto& pGuide: pGuides) {
                        ImGui::TableSetColumnIndex(column++);
                        snprintf(valueBuffer, 10, pGuide.second.minPrice < 0.05 ? "%.4f" : "%.2f", pGuide.second.maxPrice);
                        drawColoredValueText(valueBuffer, pGuide.first);
                    }
                    ImGui::EndTable();
                }
                //todo a small histogram would be nice (parse data from price guide html table)
            } else {
                if (ImGui::Button((ICON_FA_DOWNLOAD " Get for " + color->name).c_str())) {
                    controller::addBackgroundTask("Get Price Guide for " + partCode, [partCode, colorBricklinkName, currencyCode]() {
                        info_providers::price_guide::getPriceGuide(partCode, currencyCode, colorBricklinkName, false);
                    });
                }
                if (availableColors.has_value() && ImGui::Button((ICON_FA_DOWNLOAD " Get for all " + std::to_string(availableColors.value().size()) + " available colors").c_str())) {
                    for (const auto& avCol: availableColors.value()) {
                        const auto avColValue = avCol.get();
                        controller::addBackgroundTask("Get Price Guide for " + partCode + " in " + avColValue->name, [partCode, avColValue, currencyCode]() {
                            info_providers::price_guide::getPriceGuide(partCode, currencyCode, util::translateLDrawColorNameToBricklink(avColValue->name), false);
                        });
                    }
                }
            }
            ImGui::TreePop();
        }
    }

    void drawColorEdit(const std::shared_ptr<etree::Node>& lastSelectedNode, const std::shared_ptr<etree::MeshNode>& node) {
        if (ImGui::TreeNodeEx(ICON_FA_PALETTE " Color", ImGuiTreeNodeFlags_DefaultOpen)) {
            static bool isColor16, savedIsColor16;
            if (lastSelectedNode == node) {
                if (isColor16 != savedIsColor16) {
                    if (isColor16) {
                        node->setColor(ldr::Color::MAIN_COLOR_CODE);
                    } else {
                        node->setColor(node->getDisplayColor());
                    }
                    savedIsColor16 = isColor16;
                }
            } else {
                savedIsColor16 = isColor16 = node->getElementColor().code == ldr::Color::MAIN_COLOR_CODE;
            }
            ImGui::Checkbox(ICON_FA_ARROW_DOWN " Take color from parent element", &isColor16);
            if (!isColor16) {
                const auto buttonWidth = ImGui::GetFontSize() * 1.5f;
                const ImVec2& buttonSize = ImVec2(buttonWidth, buttonWidth);
                const int columnCount = std::floor(ImGui::GetContentRegionAvail().x / (buttonWidth + ImGui::GetStyle().ItemSpacing.x));

                std::optional<uoset_t<ldr::ColorReference>> availableColors = std::nullopt;
                if (node->getType() == etree::NodeType::TYPE_PART) {
                    const auto ldrNode = std::dynamic_pointer_cast<etree::LdrNode>(node);
                    availableColors = info_providers::part_color_availability::getAvailableColorsForPart(ldrNode->ldrFile);
                }
                bool showAllColors;
                if (availableColors.has_value() && !availableColors.value().empty()) {
                    static bool onlyAvailableChecked = false;
                    ImGui::Checkbox("Only show available Colors", &onlyAvailableChecked);
                    showAllColors = !onlyAvailableChecked;
                    if (onlyAvailableChecked) {
                        std::pair<std::string, std::vector<ldr::ColorReference>> group = std::make_pair("Available", std::vector<ldr::ColorReference>());
                        for (const auto& color: availableColors.value()) {
                            group.second.push_back(color);
                        }
                        gui_internal::drawColorGroup(node, buttonSize, columnCount, group);
                    }
                } else {
                    showAllColors = true;
                }
                if (showAllColors) {
                    const auto& groupedAndSortedByHue = ldr::color_repo::getAllColorsGroupedAndSortedByHue();
                    const static std::vector<std::string> fixed_pos = {"Solid", "Transparent", "Rubber"};
                    for (const auto& colorName: fixed_pos) {
                        const auto& colorGroup = std::make_pair(colorName, groupedAndSortedByHue.find(colorName)->second);
                        gui_internal::drawColorGroup(node, buttonSize, columnCount, colorGroup);
                    }
                    for (const auto& colorGroup: groupedAndSortedByHue) {
                        if (std::find(fixed_pos.begin(), fixed_pos.end(), colorGroup.first) == fixed_pos.end()) {
                            gui_internal::drawColorGroup(node, buttonSize, columnCount, colorGroup);
                        }
                    }
                }
            }
            ImGui::TreePop();
        }
    }

    void drawLayerEdit(const std::shared_ptr<etree::Node>& lastSelectedNode, const std::shared_ptr<etree::Node>& node) {
        ImGui::DragScalar(ICON_FA_LAYER_GROUP " Layer", ImGuiDataType_U8, &node->layer, 0.2f, nullptr, nullptr);
        static layer_t lastLayer = node->layer;
        if (lastSelectedNode != node) {
            lastLayer = node->layer;
        } else if (lastLayer != node->layer) {
            node->incrementVersion();
            lastLayer = node->layer;
        }
    }

    void drawDeleteButton(const std::shared_ptr<Editor>& activeEditor) {
        const auto selectedNodes = activeEditor->getSelectedNodes();
        uoset_t<etree::NodeType> selectedTypes;
        std::transform(selectedNodes.begin(),
                       selectedNodes.end(),
                       std::inserter(selectedTypes, selectedTypes.begin()),
                       [](auto node) {
                           return node.first->getType();
                       });
        if (!selectedTypes.contains(etree::NodeType::TYPE_ROOT)) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8, 0, 0, 1));
            std::string deleteButtonLabel = selectedNodes.size() > 1
                                                ? fmt::format(ICON_FA_TRASH_CAN " Delete {} elements", selectedNodes.size())
                                                : ICON_FA_TRASH_CAN " Delete element";
            if (ImGui::Button(deleteButtonLabel.c_str())) {
                activeEditor->deleteSelectedElements();
            }
            if (ImGui::IsItemHovered() && selectedTypes.contains(etree::NodeType::TYPE_MODEL)) {
                ImGui::SetTooltip(ICON_FA_TRIANGLE_EXCLAMATION " When you delete a model, all its instances will be deleted too");
            }
            ImGui::PopStyleColor();
        }
    }

    void draw(Data& data) {
        static std::shared_ptr<etree::Node> lastSelectedNode = nullptr;
        if (ImGui::Begin(data.name, &data.visible)) {
            collectWindowInfo(data.id);
            const auto& activeEditor = controller::getActiveEditor();
            if (activeEditor == nullptr || activeEditor->getSelectedNodes().empty()) {
                ImGui::Text("Select an element to view its properties here");
            } else {
                auto& selectedNodes = activeEditor->getSelectedNodes();
                if (selectedNodes.size() == 1) {
                    auto& node = selectedNodes.begin()->first;
                    auto meshNode = std::dynamic_pointer_cast<etree::MeshNode>(node);
                    const auto ldrNode = std::dynamic_pointer_cast<etree::LdrNode>(node);

                    drawDisplayNameEdit(lastSelectedNode, node);

                    if (ldrNode != nullptr) {
                        if (ldrNode->ldrFile->metaInfo.type == ldr::FileType::PART) {
                            const auto fileName = ldrNode->ldrFile->metaInfo.name;
                            const auto lastDot = fileName.rfind('.');
                            auto partCode = lastDot != std::string::npos
                                                ? fileName.substr(0, lastDot)
                                                : fileName;
                            ImGui::InputText("Part Code", partCode.data(), partCode.size(), ImGuiInputTextFlags_ReadOnly);
                        }
                    }

                    drawType(node);

                    if (node->isTransformationUserEditable()) {
                        drawTransformationEdit(lastSelectedNode, node);
                    }
                    if (meshNode != nullptr && meshNode->isColorUserEditable()) {
                        drawColorEdit(lastSelectedNode, meshNode);
                    }
                    if (node->getType() == etree::NodeType::TYPE_PART) {
                        drawPriceGuide(node);
                    }
                    if (ldrNode != nullptr && node->getType() != etree::NodeType::TYPE_MODEL) {
                        drawLayerEdit(lastSelectedNode, node);
                    }

                    if (node->getType() == etree::NodeType::TYPE_ROOT) {
                        auto filePathOpt = activeEditor->getFilePath();
                        if (filePathOpt.has_value()) {
                            std::string filePath = filePathOpt->string();
                            ImGui::InputText("File Path", filePath.data(), filePath.capacity(), ImGuiInputTextFlags_ReadOnly);
                        }
                    } else if (node->getType() == etree::NodeType::TYPE_MODEL) {
                        std::string source = ldrNode->ldrFile->source.path.string();
                        ImGui::InputText("Source File", source.data(), source.capacity(), ImGuiInputTextFlags_ReadOnly);
                        std::string sourceType = ldrNode->ldrFile->source.isMainFile ? "Main File" : "Subfile";
                        ImGui::InputText("Source Type", sourceType.data(), sourceType.capacity(), ImGuiInputTextFlags_ReadOnly);
                    }
                    if (ldrNode != nullptr) {
                        if (ImGui::Button(ICON_FA_EYE " Inspect LDraw File")) {
                            ldraw_file_inspector::setCurrentFile(ldrNode->ldrFile);
                            *gui::windows::isVisible(Id::LDRAW_FILE_INSPECTOR) = true;
                        }
                    }
                    lastSelectedNode = node;
                }
                drawDeleteButton(activeEditor);
            }
        }
        ImGui::End();
    }
}
