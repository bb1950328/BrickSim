#include "../../controller.h"
#include "../../ldr/colors.h"
#include "../../ldr/file_repo.h"
#include "../../lib/IconFontCppHeaders/IconsFontAwesome6.h"
#include "../../part_finder.h"
#include "../gui.h"
#include "../gui_internal.h"

#include "window_part_palette.h"

namespace bricksim::gui::windows::part_palette {
    void draw(Data& data) {
        if (ImGui::Begin(data.name, &data.visible)) {
            collectWindowInfo(data.id);
            static char searchTextBuffer[128] = {'\0'};
            ImGui::InputText(ICON_FA_MAGNIFYING_GLASS "##search", searchTextBuffer, 128);
            ImGui::SameLine();
            static int thumbnailZoomPercent = 100;//todo get from config
            ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8);
            ImGui::DragInt("##Zoom", &thumbnailZoomPercent, 5, 10, 500, " Zoom: %d%%");
            static auto color = ldr::color_repo::getColor(1);//todo save in config
            const glm::vec3& col = color->value.asGlmVector();
            ImGui::PushStyleColor(ImGuiCol_Text, gui_internal::getWhiteOrBlackBetterContrast(col));
            ImGui::PushStyleColor(ImGuiCol_Button, color->value);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, glm::vec4(col, .8));

            ldr::ColorReference static colorChosenInPopup;
            ImGui::SameLine();
            if (ImGui::Button(color->name.c_str())) {
                colorChosenInPopup = color->asReference();
                ImGui::OpenPopup(ICON_FA_SWATCHBOOK " Part Palette Color");
            }
            ImGui::PopStyleColor(3);
            ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal(ICON_FA_SWATCHBOOK " Part Palette Color", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                const auto buttonWidth = ImGui::GetFontSize() * 1.5f;
                const ImVec2& buttonSize = ImVec2(buttonWidth, buttonWidth);
                const int columnCount = 20;

                for (const auto& colorGroup: ldr::color_repo::getAllColorsGroupedAndSortedByHue()) {
                    if (ImGui::TreeNodeEx(colorGroup.first.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                        int i = 0;
                        for (const auto& currentColor: colorGroup.second) {
                            const auto currentColorValue = currentColor.get();
                            if (i % columnCount > 0) {
                                ImGui::SameLine();
                            }
                            ImGui::PushID(currentColorValue->code);
                            ImGui::PushStyleColor(ImGuiCol_Button, currentColorValue->value);
                            if (ImGui::Button(colorChosenInPopup.code == currentColorValue->code ? ICON_FA_CHECK : "", buttonSize)) {
                                colorChosenInPopup = currentColorValue->code;
                            }
                            ImGui::PopStyleColor(/*3*/ 1);
                            ImGui::PopID();
                            if (ImGui::IsItemHovered()) {
                                ImGui::BeginTooltip();
                                ImGui::Text("%s", currentColorValue->name.c_str());
                                ImGui::EndTooltip();
                            }
                            ++i;
                        }
                        ImGui::TreePop();
                    }
                }
                if (ImGui::Button(ICON_FA_RECTANGLE_XMARK " Cancel")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_FA_SQUARE_CHECK " Apply")) {
                    color = colorChosenInPopup.get();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            static float categorySelectWidth = 250;//todo save
            const auto totalWidth = ImGui::GetContentRegionAvail().x;
            const auto itemSpacingX = ImGui::GetStyle().ItemSpacing.x;
            float thumbnailContainerWidth = totalWidth - categorySelectWidth - itemSpacingX;
            //static const auto partsGrouped = ldr::file_repo::getAllPartsGroupedByCategory();
            static const auto partCategories = ldr::file_repo::get().getAllCategories();
            static uoset_t<std::string> selectedCategories = {*partCategories.begin()};//first category preselected
            std::set<std::string> editorNames;
            std::transform(controller::getEditors().cbegin(),
                           controller::getEditors().cend(),
                           std::inserter(editorNames, editorNames.begin()),
                           [](const std::shared_ptr<Editor>& editor) {
                               return editor->getFilename();
                           });

            ImGui::BeginChild("##categorySelectTree", ImVec2(categorySelectWidth, 0));
            for (const auto& set: {std::cref(editorNames), std::cref(partCategories)}) {
                for (const auto& category: set.get()) {
                    int flags = selectedCategories.find(category) != selectedCategories.end()
                                        ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Selected
                                        : ImGuiTreeNodeFlags_Leaf;
                    if (ImGui::TreeNodeEx(category.c_str(), flags)) {
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                            if (ImGui::GetIO().KeyCtrl) {
                                if (selectedCategories.find(category) == selectedCategories.end()) {
                                    selectedCategories.insert(category);
                                } else {
                                    selectedCategories.erase(category);
                                }
                            } else if (ImGui::GetIO().KeyShift) {
                                auto groupIt = partCategories.find(category);
                                while (groupIt != partCategories.begin() && selectedCategories.find(*groupIt) == selectedCategories.end()) {
                                    selectedCategories.insert(*groupIt);
                                    groupIt--;
                                }
                                selectedCategories.insert(*groupIt);
                            } else {
                                bool wasOnlySelectionBefore = selectedCategories.size() == 1 && *selectedCategories.begin() == category;
                                selectedCategories.clear();
                                if (!wasOnlySelectionBefore) {
                                    selectedCategories.insert(category);
                                }
                            }
                        }
                        ImGui::TreePop();
                    }
                }
            }
            ImGui::EndChild();
            ImGui::SameLine();
            ImGui::BeginChild("##thumbnailsContainer", ImVec2(thumbnailContainerWidth, 0), ImGuiWindowFlags_AlwaysVerticalScrollbar);
            const static auto thumbnailSpacing = 4;
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(thumbnailSpacing, thumbnailSpacing));
            auto actualThumbSize = std::floor(controller::getThumbnailGenerator()->size / 100.0 * thumbnailZoomPercent);
            auto actualThumbSizeSquared = ImVec2(actualThumbSize, actualThumbSize);
            int columns = std::max(1.0, std::floor((ImGui::GetContentRegionAvail().x + thumbnailSpacing) / (actualThumbSize + thumbnailSpacing)));
            int currentCol = 0;

            const bool searchEmpty = searchTextBuffer[0] == '\0';
            const auto& searchPredicate = part_finder::getPredicate(searchTextBuffer);

            const auto drawPart = [&actualThumbSizeSquared, &columns, &currentCol, &searchEmpty, &searchPredicate](const std::shared_ptr<ldr::File>& file) {
                if (searchEmpty || searchPredicate.matches(*file)) {
                    gui_internal::drawPartThumbnail(actualThumbSizeSquared, file, color->asReference());
                    currentCol++;
                    if (currentCol == columns) {
                        currentCol = 0;
                    } else {
                        ImGui::SameLine();
                    }
                }
            };

            const auto drawCategory = [&editorNames, &drawPart](const std::string& category) {
                if (editorNames.find(category) != editorNames.end()) {
                    const auto editor = std::find_if(controller::getEditors().begin(),
                                                     controller::getEditors().end(),
                                                     [&category](const std::shared_ptr<Editor>& editor) {
                                                         return editor->getFilename() == category;
                                                     })
                                                ->get();
                    for (const auto& item: editor->getRootNode()->getChildren()) {
                        const auto modelNode = std::dynamic_pointer_cast<etree::ModelNode>(item);
                        if (modelNode != nullptr) {
                            drawPart(modelNode->ldrFile);
                            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                                editor->openContextMenuNodeSelectedOrClicked(modelNode);
                            }
                        }
                    }
                } else {
                    for (const auto& part: ldr::file_repo::get().getAllFilesOfCategory(category)) {
                        drawPart(part);
                    }
                }
            };

            if (selectedCategories.size() > 1) {
                for (const auto& category: selectedCategories) {
                    ImGui::Text("%s", category.c_str());
                    drawCategory(category);
                    if (currentCol != 0) {
                        ImGui::NewLine();
                    }
                    currentCol = 0;
                }
            } else if (selectedCategories.size() == 1) {
                drawCategory(*selectedCategories.begin());
            } else {
                if (ldr::file_repo::get().areAllPartsLoaded()) {
                    for (const auto& category: editorNames) {
                        ImGui::Text("%s", category.c_str());
                        drawCategory(category);
                    }
                    for (const auto& category: ldr::file_repo::get().getAllPartsGroupedByCategory()) {
                        bool textWritten = false;
                        for (const auto& part: category.second) {
                            if (!textWritten && (searchEmpty || searchPredicate.matches(*part))) {
                                ImGui::Text("%s", category.first.c_str());
                                textWritten = true;
                            }
                            drawPart(part);
                        }
                        if (currentCol != 0) {
                            ImGui::NewLine();
                        }
                        currentCol = 0;
                    }
                } else {
                    static bool taskAdded = false;
                    if (!taskAdded) {
                        controller::addBackgroundTask("Load remaining Parts", []() {
                            ldr::file_repo::get().getAllPartsGroupedByCategory();
                        });
                        taskAdded = true;
                    }
                    const auto loaded = ldr::file_repo::get().getLoadedPartsGroupedByCategory().size();
                    const auto all = ldr::file_repo::get().getAllCategories().size();
                    ImGui::ProgressBar(loaded * 1.0f / all);
                    ImGui::Text("%c %lu of %lu categories loaded, please wait", gui_internal::getLoFiSpinner(), loaded, all);
                }
            }
            ImGui::PopStyleVar();
            ImGui::EndChild();
        }
        ImGui::End();
    }
}
